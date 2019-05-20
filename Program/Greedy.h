#ifndef GREEDY_H
#define GREEDY_H

#include "Params.h"
#include "Solution.h"

class Greedy
{
private:

	Params * params;		 // Access to the problem and dataset parameters
	Solution * solution;	 // Access to the solution structure to be filled
	Solution * previousSolution;

	// Main recursive function to run the greedy algorithm on the tree
	// Recursive call on a given node and level in the tree
	void recursiveConstruction(int node, int level);

	// Calculates the best split threshold for a continuous attribute
	// Complexity proportional to the number of samples
	double calculateBestSplitContinuous(int atributeIndex, const std::vector<int> & samples);

	// Chooses a random value for the split attribute. If all values have been tested returns false,
	// otherwise returns true
	bool chooseRandomValueForSplitAttribute(int node);

	void localSearch(int node, int level);

public:

	int MAX_REPETITIONS_LS = 6;

    // Run the algorithm
    void run();

	void runWithLS();

	void runLookAhead();

	Solution* getNewSolution();

	// Constructor
	Greedy(Params * params, Solution * solution): params(params), solution(solution){};
};

#endif
