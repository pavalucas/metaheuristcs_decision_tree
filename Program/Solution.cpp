#include "Solution.h"

void Node::evaluate()
{
    entropy = 0.0;
    for (int c = 0; c < params->nbClasses; c++)
    {
        if (nbSamplesClass[c] > 0)
        {
            double frac = (double)nbSamplesClass[c]/(double)nbSamplesNode;
            entropy -= frac * log2(frac);
            if (nbSamplesClass[c] > maxSameClass)
            { 
                maxSameClass = nbSamplesClass[c];
                majorityClass = c;
            }
        }
    }
}

void Node::addSample(int i)
{
    samples.push_back(i);
    nbSamplesClass[params->dataClasses[i]]++;
    nbSamplesNode++;
}

void Node::deleteInformation()
{
    nodeType = NODE_NULL;
    splitAttribute = -1;
    splitValue = -1.e30;
    nbSamplesClass = std::vector<int>(params->nbClasses, 0);
    samples.clear();
    nbSamplesNode = 0;
    majorityClass = -1;
    maxSameClass = 0;
    entropy = -1.e30;
}

Node::Node(Params * params):params(params)
{
    deleteInformation();
}

Node Node::copyNode()
{
    Node newNode = Node(params);
    newNode.nodeType = nodeType;
    newNode.splitAttribute = splitAttribute;
    newNode.splitValue = splitValue;
    newNode.nbSamplesClass = nbSamplesClass;
    newNode.samples = samples;
    newNode.nbSamplesNode = nbSamplesNode;
    newNode.majorityClass = majorityClass;
    newNode.maxSameClass = maxSameClass;
    newNode.entropy = entropy;
    newNode.testedSplitValues = testedSplitValues;
    return newNode;
}

void Solution::printAndExport(std::string file_name, int seed_rng)
{
    std::ofstream my_file;
    my_file.open(file_name.data());
    if (my_file.is_open())
    {
        std::cout << std::endl << "---------------------------------------- PRINTING SOLUTION ----------------------------------------" << std::endl;
        my_file << std::endl << "---------------------------------------- PRINTING SOLUTION ----------------------------------------" << std::endl;			
        for (int d = 0; d <= params->maxDepth; d++)
        {
            // Printing one complete level of the tree
            for (int i = pow(2, d) - 1; i < pow(2, d + 1) - 1; i++)
            {
                if (tree[i].nodeType == Node::NODE_INTERNAL)
                {
                    std::cout << "(N" << i << ",A[" << tree[i].splitAttribute << "]" << (params->attributeTypes[tree[i].splitAttribute] == TYPE_NUMERICAL ? "<=" : "=") << tree[i].splitValue << ") ";
                    my_file << "(N" << i << ",A[" << tree[i].splitAttribute << "]" << (params->attributeTypes[tree[i].splitAttribute] == TYPE_NUMERICAL ? "<=" : "=") << tree[i].splitValue << ") ";
                }
                else if (tree[i].nodeType == Node::NODE_LEAF)
                {
                    int misclass = tree[i].nbSamplesNode - tree[i].nbSamplesClass[tree[i].majorityClass];
                    std::cout << "(L" << i << ",C" << tree[i].majorityClass << "," << tree[i].nbSamplesClass[tree[i].majorityClass] << "," << misclass << ") ";
                    my_file << "(L" << i << ",C" << tree[i].majorityClass << "," << tree[i].nbSamplesClass[tree[i].majorityClass] << "," << misclass << ") ";
                }
            }
            std::cout << std::endl;
            my_file << std::endl;
        }
        std::cout << nbMisclassifiedSamples << "/" << params->nbSamples << " MISCLASSIFIED SAMPLES" << std::endl;
        my_file << nbMisclassifiedSamples << "/" << params->nbSamples << " MISCLASSIFIED SAMPLES" << std::endl;
        std::cout << "---------------------------------------------------------------------------------------------------" << std::endl << std::endl;
        my_file << "---------------------------------------------------------------------------------------------------" << std::endl << std::endl;

        
        std::cout << "SEED: " << seed_rng << std::endl;
        my_file << "SEED: " << seed_rng << std::endl;
        my_file << "TIME(s): " << (params->endTime - params->startTime) / (double)CLOCKS_PER_SEC << std::endl;
        my_file << "NB_SAMPLES: " << params->nbSamples << std::endl;
        my_file << "NB_MISCLASSIFIED: " << nbMisclassifiedSamples << std::endl;
        my_file.close();
    }
    else
        std::cout << "----- IMPOSSIBLE TO OPEN SOLUTION FILE: " << params->pathToSolution << " ----- " << std::endl;
}

Solution::Solution(Params * params, bool addSamplesToRoot):params(params)
{
    // Setting number of missclassified samples to zero
    nbMisclassifiedSamples = 0;

    // Initializing tree data structure and the nodes inside -- The size of the tree is 2^{maxDepth} - 1
    tree = std::vector <Node>(pow(2,params->maxDepth+1)-1,Node(params));

    // The tree is initially made of a single leaf (the root node)
    if(addSamplesToRoot)
    {
        tree[0].nodeType = Node::NODE_LEAF;
        for (int i = 0; i < params->nbSamples; i++) 
            tree[0].addSample(i);
        tree[0].evaluate();
    }
}

Solution::~Solution()
{
    nbMisclassifiedSamples = 0;
    tree.clear();
}

void Solution::eraseSubTree(int node, int level)
{
    if(level > params->maxDepth)
    {
        return;
    }
    else if(tree[node].nodeType == Node::NODE_LEAF)
    {
        tree[node].deleteInformation();
    }
    else
    {
        tree[node].deleteInformation();
        eraseSubTree(2 * node + 1, level + 1);
        eraseSubTree(2 * node + 2, level + 1);
    }
}

void Solution::applySplit(int node)
{
    int bestSplitAttribute = tree[node].splitAttribute;
    double bestSplitThrehold = tree[node].splitValue;
	tree[node].nodeType = Node::NODE_INTERNAL;
	tree[2*node+1].nodeType = Node::NODE_LEAF ;
	tree[2*node+2].nodeType = Node::NODE_LEAF ;
	for (int s : tree[node].samples)
	{ 
		if ((params->attributeTypes[bestSplitAttribute] == TYPE_NUMERICAL   && params->dataAttributes[s][bestSplitAttribute] < bestSplitThrehold + MY_EPSILON)|| 
			(params->attributeTypes[bestSplitAttribute] == TYPE_CATEGORICAL && params->dataAttributes[s][bestSplitAttribute] < bestSplitThrehold + MY_EPSILON && params->dataAttributes[s][bestSplitAttribute] > bestSplitThrehold - MY_EPSILON))
			tree[2*node+1].addSample(s);
		else
			tree[2*node+2].addSample(s);
	}
	tree[2*node+1].evaluate(); // Setting all other data structures
	tree[2*node+2].evaluate(); // Setting all other data structures
}

int Solution::getNumberMissclassifiedSamples()
{
    int result = 0;
    for (int i = 0; i < pow(2, params->maxDepth + 1) - 1; i++)
    {
        if (tree[i].nodeType == Node::NODE_LEAF)
        {
            int misclass = tree[i].nbSamplesNode - tree[i].nbSamplesClass[tree[i].majorityClass];
            result += misclass;
        }
    }
    return result;
}

Solution* Solution::copySolution()
{
    Solution* newSolution = new Solution(params, false);
    newSolution->nbMisclassifiedSamples = nbMisclassifiedSamples;
    for (int i = 0; i < pow(2, params->maxDepth + 1) - 1; i++)
    {
        newSolution->tree[i] = tree[i].copyNode();
    }
    return newSolution;
}
