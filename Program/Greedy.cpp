#include "Greedy.h"

void Greedy::run()
{
	// Call the recursive procedure on the root node at level 0
	recursiveConstruction(0, 0);
	solution->nbMisclassifiedSamples = solution->getNumberMissclassifiedSamples();
}

void Greedy::runWithLS()
{
	runLookAhead();
	localSearch(0, 0);
}

void Greedy::runLookAhead()
{
	recursiveConstructionLookAhead(0, 0, K);
	solution->nbMisclassifiedSamples = solution->getNumberMissclassifiedSamples();
}

void Greedy::localSearch(int node, int level)
{
	if(level > params->maxDepth || solution->tree[node].nodeType == Node::NODE_LEAF || 
		solution->nbMisclassifiedSamples == 0) 
	{
		return;
	}
	for(int curRep = 0; curRep < MAX_REPETITIONS_LS; curRep++)
	{
		previousSolution = solution->copySolution();
		bool allValuesTested = chooseRandomValueForSplitAttribute(node);
		if(allValuesTested)
		{
			delete previousSolution;
			previousSolution = nullptr;
			localSearch(2 * node + 1, level + 1);
			localSearch(2 * node + 2, level + 1);
			return;
		}
		solution->eraseSubTree(2 * node + 1, level + 1);
		solution->eraseSubTree(2 * node + 2, level + 1);
		solution->applySplit(node);
		recursiveConstruction(2 * node + 1, level + 1);
		recursiveConstruction(2 * node + 2, level + 1);
		int newNbMisclassifiedSamples = solution->getNumberMissclassifiedSamples();
		if(newNbMisclassifiedSamples < solution->nbMisclassifiedSamples) {
			solution->nbMisclassifiedSamples = newNbMisclassifiedSamples;
			delete previousSolution;
			previousSolution = nullptr;
		}
		else
		{
			Solution* tmp = solution;
			solution = previousSolution;
			delete tmp;
			previousSolution = nullptr;
		}
	}
	localSearch(2 * node + 1, level + 1);
	localSearch(2 * node + 2, level + 1);
}

bool Greedy::chooseRandomValueForSplitAttribute(int node)
{
	int curAtt = solution->tree[node].splitAttribute;
	solution->tree[node].testedSplitValues.insert(solution->tree[node].splitValue);
	previousSolution->tree[node].testedSplitValues.insert(solution->tree[node].splitValue);
	std::vector<double> attributeLevelsWithoutBestValue;
	
	if (params->attributeTypes[curAtt] == TYPE_NUMERICAL)
	{
		std::set<double> attributeLevels;
		for (int s : solution->tree[node].samples)
		{
			attributeLevels.insert(params->dataAttributes[s][curAtt]);
		}
		for (double attributeValue : attributeLevels)
		{
			if(solution->tree[node].testedSplitValues.find(attributeValue) ==
				solution->tree[node].testedSplitValues.end())
			{
				attributeLevelsWithoutBestValue.push_back(attributeValue);
			}
		}
	}
	else
	{
		for (int attLevel = 0; attLevel < params->nbLevels[curAtt]; attLevel++)
		{
			if(solution->tree[node].testedSplitValues.find(attLevel) ==
				solution->tree[node].testedSplitValues.end())
			{
				attributeLevelsWithoutBestValue.push_back(attLevel);
			}
		}
	}
	if(attributeLevelsWithoutBestValue.size() == 0)
	{
		return true;
	}
	int randomIndex = rand() % attributeLevelsWithoutBestValue.size();
	solution->tree[node].splitValue = attributeLevelsWithoutBestValue[randomIndex];
	solution->tree[node].testedSplitValues.insert(solution->tree[node].splitValue);
	previousSolution->tree[node].testedSplitValues.insert(solution->tree[node].splitValue);
	return false;
}

void Greedy::recursiveConstruction(int node, int level)
{
	/* BASE CASES -- MAXIMUM LEVEL HAS BEEN ATTAINED OR ALL SAMPLES BELONG TO THE SAME CLASS */
	if (level >= params->maxDepth || solution->tree[node].maxSameClass == solution->tree[node].nbSamplesNode)
		return;

	/* LOOK FOR A BEST SPLIT */
	bool allIdentical = true; // To detect contradictory data
	int nbSamplesNode = solution->tree[node].nbSamplesNode;
	double originalEntropy = solution->tree[node].entropy;
	double bestInformationGain = -1.e30;
	int bestSplitAttribute = -1;
	double bestSplitThrehold = -1.e30;
	for (int att = 0; att < params->nbAttributes; att++)
	{
		if (params->attributeTypes[att] == TYPE_NUMERICAL)
		{
			/* CASE 1) -- FIND SPLIT WITH BEST INFORMATION GAIN FOR NUMERICAL ATTRIBUTE c */
			 
			// Define some data structures
			std::vector <std::pair<double, int> > orderedSamples;		// Order of the samples according to attribute c
			std::set<double> attributeLevels;							// Store the possible levels of this attribute among the samples (will allow to "skip" samples with equal attribute value)
			for (int s : solution->tree[node].samples)
			{
				orderedSamples.push_back(std::pair<double, int>(params->dataAttributes[s][att], params->dataClasses[s]));
				attributeLevels.insert(params->dataAttributes[s][att]);
			}
			if (attributeLevels.size() <= 1) continue;					// If all sample have the same level for this attribute, it's useless to look for a split
			else allIdentical = false;
			std::sort(orderedSamples.begin(), orderedSamples.end());
			
			// Initially all samples are on the right
			std::vector <int> nbSamplesClassLeft = std::vector<int>(params->nbClasses, 0);
			std::vector <int> nbSamplesClassRight = solution->tree[node].nbSamplesClass;
			int indexSample = 0;
			for (double attributeValue : attributeLevels) // Go through all possible attribute values in increasing order
			{
				// Iterate on all samples with this attributeValue and switch them to the left
				while (indexSample < nbSamplesNode && orderedSamples[indexSample].first < attributeValue + MY_EPSILON)
				{
					nbSamplesClassLeft[orderedSamples[indexSample].second]++;
					nbSamplesClassRight[orderedSamples[indexSample].second]--;
					indexSample++;
				}
				
				if (indexSample != nbSamplesNode) // No need to consider the case in which all samples have been switched to the left
				{
					// Evaluate entropy of the two resulting sample sets
					double entropyLeft = 0.0;
					double entropyRight = 0.0;
					for (int c = 0; c < params->nbClasses; c++)
					{
						// Remark that indexSample contains at this stage the number of samples in the left
						if (nbSamplesClassLeft[c] > 0)
						{
							double fracLeft = (double)nbSamplesClassLeft[c] / (double)(indexSample);
							entropyLeft -= fracLeft * log2(fracLeft);
						}
						if (nbSamplesClassRight[c] > 0)
						{
							double fracRight = (double)nbSamplesClassRight[c] / (double)(nbSamplesNode - indexSample);
							entropyRight -= fracRight * log2(fracRight);
						}
					}

					// Evaluate the information gain and store if this is the best option found until now
					double informationGain = originalEntropy - ((double)indexSample*entropyLeft + (double)(nbSamplesNode - indexSample)*entropyRight) / (double)nbSamplesNode;
					if (informationGain > bestInformationGain)
					{
						bestInformationGain = informationGain;
						bestSplitAttribute = att;
						bestSplitThrehold = attributeValue;
					}
				}
			}
		}
		else 
		{
			/* CASE 2) -- FIND BEST SPLIT FOR CATEGORICAL ATTRIBUTE c */

			// Count for each level of attribute c and each class the number of samples
			std::vector <int> nbSamplesLevel = std::vector <int>(params->nbLevels[att],0);
			std::vector <int> nbSamplesClass = std::vector <int>(params->nbClasses, 0);
			std::vector < std::vector <int> > nbSamplesLevelClass = std::vector< std::vector <int> >(params->nbLevels[att], std::vector <int>(params->nbClasses,0));
			for (int s : solution->tree[node].samples)
			{
				nbSamplesLevel[params->dataAttributes[s][att]]++;
				nbSamplesClass[params->dataClasses[s]]++;
				nbSamplesLevelClass[params->dataAttributes[s][att]][params->dataClasses[s]]++;
			}

			// Calculate information gain for a split at each possible level of attribute c
			for (int attLevel = 0; attLevel < params->nbLevels[att]; attLevel++)
			{
				if (nbSamplesLevel[attLevel] > 0 && nbSamplesLevel[attLevel] < nbSamplesNode)
				{
					// Evaluate entropy of the two resulting sample sets
					allIdentical = false;
					double entropyLevel = 0.0;
					double entropyOthers = 0.0;
					for (int c = 0; c < params->nbClasses; c++)
					{
						if (nbSamplesLevelClass[attLevel][c] > 0)
						{
							double fracLevel = (double)nbSamplesLevelClass[attLevel][c] / (double)nbSamplesLevel[attLevel] ;
							entropyLevel -= fracLevel * log2(fracLevel);
						}
						if (nbSamplesClass[c] - nbSamplesLevelClass[attLevel][c] > 0)
						{
							double fracOthers = (double)(nbSamplesClass[c] - nbSamplesLevelClass[attLevel][c]) / (double)(nbSamplesNode - nbSamplesLevel[attLevel]);
							entropyOthers -= fracOthers * log2(fracOthers);
						}
					}

					// Evaluate the information gain and store if this is the best option found until now
					double informationGain = originalEntropy - ((double)nbSamplesLevel[attLevel] *entropyLevel + (double)(nbSamplesNode - nbSamplesLevel[attLevel])*entropyOthers) / (double)nbSamplesNode;
					if (informationGain > bestInformationGain)
					{
						bestInformationGain = informationGain;
						bestSplitAttribute = att;
						bestSplitThrehold = attLevel;
					}
				}
			}
		}
	}

	/* APPLY THE SPLIT AND RECURSIVE CALL */
	solution->tree[node].splitAttribute = bestSplitAttribute;
	solution->tree[node].splitValue = bestSplitThrehold;
	solution->applySplit(node);
	recursiveConstruction(2*node+1,level+1); // Recursive call
	recursiveConstruction(2*node+2,level+1); // Recursive call
}

void Greedy::recursiveConstructionLookAhead(int node, int level, int k)
{
	/* BASE CASES -- MAXIMUM LEVEL HAS BEEN ATTAINED OR ALL SAMPLES BELONG TO THE SAME CLASS */
	if (level >= params->maxDepth || solution->tree[node].maxSameClass == solution->tree[node].nbSamplesNode)
		return;

	/* LOOK FOR A BEST SPLIT */
	bool allIdentical = true; // To detect contradictory data
	int nbSamplesNode = solution->tree[node].nbSamplesNode;
	double originalEntropy = solution->tree[node].entropy;
	double bestInformationGain = -1.e30;
	int bestSplitAttribute = -1;
	double bestSplitThrehold = -1.e30;
	for (int att = 0; att < params->nbAttributes; att++)
	{
		if (params->attributeTypes[att] == TYPE_NUMERICAL)
		{
			/* CASE 1) -- FIND SPLIT WITH BEST INFORMATION GAIN FOR NUMERICAL ATTRIBUTE c */
			 
			// Define some data structures
			std::set<double> attributeLevels;							// Store the possible levels of this attribute among the samples (will allow to "skip" samples with equal attribute value)
			for (int s : solution->tree[node].samples)
			{
				attributeLevels.insert(params->dataAttributes[s][att]);
			}
			if (attributeLevels.size() <= 1) continue;					// If all sample have the same level for this attribute, it's useless to look for a split
			else allIdentical = false;
			for (double attributeValue : attributeLevels) // Go through all possible attribute values in increasing order
			{
				// Iterate on all samples with this attributeValue and switch them to the left
			
				double childEntropy = getEntropyK(node, level, att, attributeValue, k-1);

				// Evaluate the information gain and store if this is the best option found until now
				double informationGain = originalEntropy - childEntropy;
				if (informationGain > bestInformationGain)
				{
					bestInformationGain = informationGain;
					bestSplitAttribute = att;
					bestSplitThrehold = attributeValue;
				}
			}
		}
		else 
		{
			/* CASE 2) -- FIND BEST SPLIT FOR CATEGORICAL ATTRIBUTE c */

			// Calculate information gain for a split at each possible level of attribute c
			for (int attLevel = 0; attLevel < params->nbLevels[att]; attLevel++)
			{
				// Evaluate entropy of the two resulting sample sets
				allIdentical = false;
				double childEntropy = getEntropyK(node, level, att, attLevel, k-1);

				// Evaluate the information gain and store if this is the best option found until now
				double informationGain = originalEntropy - childEntropy;
				if (informationGain > bestInformationGain)
				{
					bestInformationGain = informationGain;
					bestSplitAttribute = att;
					bestSplitThrehold = attLevel;
				}
			}
		}
	}

	/* APPLY THE SPLIT AND RECURSIVE CALL */
	solution->tree[node].splitAttribute = bestSplitAttribute;
	solution->tree[node].splitValue = bestSplitThrehold;
	solution->applySplit(node);
	recursiveConstructionLookAhead(2*node+1, level+1, k); // Recursive call
	recursiveConstructionLookAhead(2*node+2, level+1, k); // Recursive call
}

double Greedy::getEntropyK(int node, int level, int splitAttribute, double splitValue, int k)
{
	/* BASE CASES -- MAXIMUM LEVEL HAS BEEN ATTAINED OR ALL SAMPLES BELONG TO THE SAME CLASS */
	if (level >= params->maxDepth || solution->tree[node].maxSameClass == solution->tree[node].nbSamplesNode)
		return 0;

	if(k == 0)
	{
		solution->tree[node].splitAttribute = splitAttribute;
		solution->tree[node].splitValue = splitValue;
		solution->applySplit(node);
		double entropyLeft = solution->tree[2 * node + 1].entropy;
		double entropyRight = solution->tree[2 * node + 2].entropy;
		int nbSamplesParent = solution->tree[node].nbSamplesNode;
		int nbSamplesLeftChild = solution->tree[2 * node + 1].nbSamplesNode;
		int nbSamplesRightChild = solution->tree[2 * node + 2].nbSamplesNode;
		solution->tree[2 * node + 1].deleteInformation();
		solution->tree[2 * node + 2].deleteInformation();
		solution->tree[node].nodeType = Node::NODE_NULL;
		solution->tree[node].splitAttribute = -1;
		solution->tree[node].splitValue = -1.e30;

		return ((double)nbSamplesLeftChild * entropyLeft + (double)nbSamplesRightChild * entropyRight) / 
				(double)nbSamplesParent;
	}

	if(level + 1 >= params->maxDepth)
		return getEntropyK(node, level, splitAttribute, splitValue, k-1);
	solution->tree[node].splitAttribute = splitAttribute;
	solution->tree[node].splitValue = splitValue;
	solution->applySplit(node);
	int nbSamplesParent = solution->tree[node].nbSamplesNode;
	std::vector<int> childs = {2 * node + 1, 2 * node + 2};
	double resultEntropy = 0.0;
	for(int cur_node : childs)
	{
		int nbSamplesNode = solution->tree[cur_node].nbSamplesNode;
		double bestEntropy = 1.e30;
		for (int att = 0; att < params->nbAttributes; att++)
		{
			if (params->attributeTypes[att] == TYPE_NUMERICAL)
			{
				/* CASE 1) -- FIND SPLIT WITH BEST INFORMATION GAIN FOR NUMERICAL ATTRIBUTE c */
				
				// Define some data structures
				std::set<double> attributeLevels;							// Store the possible levels of this attribute among the samples (will allow to "skip" samples with equal attribute value)
				for (int s : solution->tree[cur_node].samples)
				{
					attributeLevels.insert(params->dataAttributes[s][att]);
				}
				if (attributeLevels.size() <= 1) continue;					// If all sample have the same level for this attribute, it's useless to look for a split
				for (double attributeValue : attributeLevels) // Go through all possible attribute values in increasing order
				{					
					double curEntropy = getEntropyK(cur_node, level + 1, att, attributeValue, k-1);
					bestEntropy = std::min(curEntropy, bestEntropy);
				}
			}
			else 
			{
				/* CASE 2) -- FIND BEST SPLIT FOR CATEGORICAL ATTRIBUTE c */

				// Calculate information gain for a split at each possible level of attribute c
				for (int attLevel = 0; attLevel < params->nbLevels[att]; attLevel++)
				{
					double curEntropy = getEntropyK(cur_node, level + 1, att, attLevel, k-1);
					bestEntropy = std::min(curEntropy, bestEntropy);
				}
			}
		
			resultEntropy += ((double)nbSamplesNode * bestEntropy) / (double)nbSamplesParent;
			solution->tree[2 * cur_node + 1].deleteInformation();
			solution->tree[2 * cur_node + 2].deleteInformation();
			solution->tree[cur_node].nodeType = Node::NODE_NULL;
			solution->tree[cur_node].splitAttribute = -1;
			solution->tree[cur_node].splitValue = -1.e30;
		}
	}
	solution->tree[2 * node + 1].deleteInformation();
	solution->tree[2 * node + 2].deleteInformation();
	solution->tree[node].splitAttribute = -1;
	solution->tree[node].splitValue = -1.e30;
	return resultEntropy;
}

Solution* Greedy::getNewSolution()
{
	return solution;
}
