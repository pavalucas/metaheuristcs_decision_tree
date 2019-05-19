#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include "Params.h"

// Structure representing one node of the (orthogonal) decision tree or a leaf
class Node
{

public:

	enum {NODE_NULL, NODE_LEAF, NODE_INTERNAL} nodeType;	// Node type
	Params * params;										// Access to the problem and dataset parameters
	int splitAttribute;										// Attribute to which the split is applied (filled through the greedy algorithm)
	double splitValue;										// Threshold value for the split (for numerical attributes the left branch will be <= splitValue, for categorical will be == splitValue)					
	std::vector <int> samples;								// Samples from the training set at this node
	std::vector <int> nbSamplesClass;						// Number of samples of each class at this node (for each class)
	int nbSamplesNode;										// Total number of samples in this node
	int majorityClass;										// Majority class in this node
	int maxSameClass;										// Maximum number of elements of the same class in this node
	double entropy;											// Entropy in this node
	
	void evaluate();

	void addSample(int i);

	void deleteInformation();

	Node(Params * params);
};

class Solution
{

private:

	// Access to the problem and dataset parameters
	Params * params;

public:

	// Vector representing the tree
	// Parent of tree[k]: tree[(k-1)/2]
	// Left child of tree[k]: tree[2*k+1]
	// Right child of tree[k]: tree[2*k+2]
	std::vector <Node> tree;

	// Prints the final solution
	void printAndExport(std::string file_name, int seed_rng);

	Solution(Params * params);

	void eraseSubTree(int node, int level);

	void applySplit(int node);
};
#endif
