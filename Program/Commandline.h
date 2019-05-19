#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>

class Commandline
{
private:

	bool command_ok_;				// Boolean the check if the line of command is valid
	int cpu_time_;					// Allocated CPU time (defaults to 5min)
	int seed_;						// Random seed_ (defaults to 0, in this case the current time value will be used as seed_)
	int max_depth_;					// Maximum depth for the classification tree (defaults to 4)
	std::string instance_path_;		// Instance path
	std::string output_name_;		// Output path
	std::string dataset_name_;		// Dataset name
	std::vector<std::string> dataset_name_vec_;
	std::vector<std::string> dataset_path_vec_;
	std::vector<std::string> output_path_vec_;
	bool run_with_all_datasets_;
	const std::vector<std::string> kAllDatasetName = {"p01.txt", "p02.txt", "p03.txt", "p04.txt", "p05.txt",
	"p06.txt", "p07.txt", "p08.txt", "p09.txt", "p10.txt"};

	void set_run_with_all_datasets(std::string to_parse)
	{
		if(to_parse == "all") run_with_all_datasets_ = true;
		else run_with_all_datasets_ = false;
	}

	// Setting the dataset name
	void set_instance_path(std::string to_parse) { instance_path_ = to_parse; }

	// Setting the output name
	void set_output_name(std::string to_parse) 
	{ 
		for(std::string dataset_name : dataset_name_vec_)
		{
			output_path_vec_.push_back(to_parse + "_" +  dataset_name);
		}
	}

	// Setting a default output name
	void set_default_output_name(std::string to_parse)
	{
		char caractere1 = '/';
		char caractere2 = '\\';

		int position = (int)to_parse.find_last_of(caractere1);
		int position2 = (int)to_parse.find_last_of(caractere2);
		if (position2 > position) position = position2;

		if (position != -1)
			output_name_ = to_parse.substr(0, position + 1) + "sol-" + to_parse.substr(position + 1, to_parse.length() - 1);
		else
			output_name_ = "sol-" + to_parse;
	}

	void set_dataset_name(std::string to_parse)
	{
		if(run_with_all_datasets_)
		{
			for(const std::string& dataset_name : kAllDatasetName)
			{
				dataset_name_vec_.push_back(dataset_name);
				dataset_path_vec_.push_back(instance_path_ + dataset_name);
			}
		}
		else
		{
			dataset_name_vec_ = {to_parse};
			dataset_path_vec_ = {instance_path_ + to_parse};
		}
		
		// char caractere1 = '/';
		// char caractere2 = '\\';

		// int position = (int)to_parse.find_last_of(caractere1);
		// int position2 = (int)to_parse.find_last_of(caractere2);
		// if (position2 > position) position = position2;

		// dataset_name_ = to_parse.substr(position + 1, to_parse.length() - 1);
	}

public:

	// Constructor
	Commandline(int argc, char* argv[])
	{
		command_ok_ = true;
		if (argc % 2 == 0 || argc > 11 || argc < 3)
		{
			std::cout << "ISSUE WITH THE NUMBER OF COMMANDLINE ARGUMENTS: " << argc << std::endl;
			command_ok_ = false;
		}
		else
		{
			// Setting some default values
			set_instance_path(std::string(argv[1]));
			set_default_output_name(std::string(argv[1]));
			set_run_with_all_datasets(std::string(argv[2]));
			set_dataset_name(std::string(argv[2]));
			
			cpu_time_ = 300;
			seed_ = 0;
			max_depth_ = 4;

			for (int i = 3; i < argc; i += 2)
			{
				if (std::string(argv[i]) == "-t")
					cpu_time_ = atoi(argv[i+1]);
				else if (std::string(argv[i]) == "-sol")
					set_output_name(std::string(argv[i+1]));
				else if (std::string(argv[i]) == "-seed_")
					seed_ = atoi(argv[i+1]);
				else if (std::string(argv[i]) == "-depth")
					max_depth_ = atoi(argv[i+1]);
				
				else
				{
					std::cout << "----- NON RECOGNIZED ARGUMENT: " << std::string(argv[i]) << std::endl;
					command_ok_ = false;
				}
			}
		}
	}

	// Getting path to the instance file
	std::string get_path_to_instance() { return instance_path_; }

	// Getting path to the solution file
	std::string get_path_to_solution() { return output_name_; }

	// Getting allocated CPU time
	int get_cpu_time() { return cpu_time_; }

	// Getting the random seed_
	int get_seed() { return seed_; }

	// Getting the depth
	int get_maxDepth() { return max_depth_; }

	// Tests whether the commandline parameters are OK
	bool is_valid() { return command_ok_; }

	std::vector<std::string> get_dataset_path_vec() { return dataset_path_vec_; }

	std::vector<std::string> get_output_path_vec() { return output_path_vec_; }
};
#endif
