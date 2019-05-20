#include "Commandline.h"
#include "Params.h"
#include "Solution.h"
#include "Greedy.h"

int main(int argc, char *argv[])
{
	Commandline c(argc, argv);
	if (c.is_valid())
	{
		std::vector<std::string> dataset_path_vec = c.get_dataset_path_vec();
		std::vector<std::string> output_path_vec = c.get_output_path_vec();
		for(int i = 0; i < dataset_path_vec.size(); i++)
		{
			// Initialization of the problem data from the commandline
			Params params(dataset_path_vec[i], output_path_vec[i], c.get_seed(), c.get_maxDepth(), c.get_cpu_time() * CLOCKS_PER_SEC);

			// Initialization of a solution structure
			Solution* solution = new Solution(&params, true);

			// Run the greedy algorithm 
			std::cout << "----- STARTING DECISION TREE OPTIMIZATION" << std::endl;
			params.startTime = clock();
			Greedy solver(&params, solution);
			solver.runWithLS();
			solution = solver.getNewSolution();
			params.endTime = clock();
			std::cout << "----- DECISION TREE OPTIMIZATION COMPLETED IN " << (params.endTime - params.startTime) / (double)CLOCKS_PER_SEC << "(s)" << std::endl;
			
			// Printing the solution and exporting statistics (also export results into a file)
			solution->printAndExport(output_path_vec[i], c.get_seed());
			std::cout << "----- END OF ALGORITHM" << std::endl;
		}
	}
	return 0;
}
