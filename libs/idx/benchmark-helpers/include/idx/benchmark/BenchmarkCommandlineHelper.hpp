#ifndef __IDX__BENCHMARK__BENCHMARK_HELPER__
#define __IDX__BENCHMARK__BENCHMARK_HELPER__


#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>

#include <idx/utils/CommandParser.hpp>
#include <idx/utils/DataSetGenerators.hpp>

#include "idx/benchmark/BenchmarkConfiguration.hpp"

namespace idx { namespace benchmark {



class BenchmarkCommandlineHelper {

private:
	int mArgc;
	char **mArgv;
	std::map<std::string, std::string> mAdditionalConfigOptions;
	idx::utils::CommandParser mParser;

public:
	BenchmarkCommandlineHelper(int argc, char **argv, std::string dataStructureName, std::map<std::string, std::string> const & additionalConfigOptions) : mArgc(argc), mArgv(argv), mAdditionalConfigOptions(additionalConfigOptions), mParser(argc, argv, [=]() {
		std::cout << std::endl;
		std::cout << "Usage: " << argv[0] << " -insert=<insertType> [-insertModifier=<modifierType>] [-input=<insertFileName>] -size=<size> [-lookup=<lookupType>] [-lookupFile=<lookupFileName>] [-help] [-verbose=<true/false>] [-insertModifier=<true/false>]";
		for(auto const & entry : additionalConfigOptions) {
			std::cout << " [-" << entry.first << "=<" << entry.first << ">]";
		}
		std::cout << std::endl;
		std::cout << "\tdescription: " << argv[0] << "Inserts <size> values of a given generator type (<insertType>) into the " << dataStructureName << std::endl;
		std::cout << "\t\tAfter that lookup is executed with provided data. Either a modification of the input type or a separate data file" << std::endl;
		std::cout << "\t\tThe lookup is executed n times the size of the lookup data set, where n is the smallest natural number which results in at least 100 million lookup operations" << std::endl << std::endl;
		std::cout << "\tparameters:" << std::endl;
		std::cout << "\t\t<insertType>: is either dense/pseudorandom/random or file. In case of file the -insertFile parameter must be provided." << std::endl;
		std::cout << "\t\t<modifierType>: is on of sequential/random/reverse and modifies the input data before it is inserted." << std::endl;
		std::cout << "\t\t<lookupType>: is either a modifier (\"sequential\"/\"random\" or \"reverse\") on the input data which will be used to modify the input data before executing the lookup" << std::endl;
		std::cout << "\t\t\tor \"file\" which requires the additional parameter lookupFile specifying a file containing  the lookup data." << std::endl;
		std::cout << "\t\t\tIn case no lookup type is specified the same data which was used for insert is used for the lookup." << std::endl;
		std::cout << "\t\t<insertFileName>: Only used in case the <modifierType> \"file\" is specified. It specifies the name of a dat file containing the input data." << std::endl;
		std::cout << "\t\t<insertFileName>: Only used in case the <lookupType> \"file\" is specified. It specifies the name of a dat file containing the lookup data." << std::endl;
		std::cout << std::endl << std::endl;
		for(auto const & entry : additionalConfigOptions) {
			std::cout << "\t\t<" << entry.first << ">: " << entry.second << std::endl;
		}
		std::cout << std::endl;
		std::cout << "\t" << "-verbose: specifies whether verbose debug output should be printed or not." << std::endl;
		std::cout << "\t" << "-insertOnly: specifies whether only the insert operation should be executed." << std::endl;
		std::cout << "\t" << "-threads: specifies the number of threads used for inserts as well as lookups." << std::endl;
		std::cout << "\t" << "-help: prints this usage message and terminates the application." << std::endl;
		std::cout << std::endl;
	} ) {
	}

	BenchmarkConfiguration parseArguments() {
		std::set<std::string> allowedConfigOptions { "insert", "insertOnly", "insertModifier", "input", "size", "lookup", "lookupFile", "verbose", "threads" };
		for(auto const & entry : mAdditionalConfigOptions) {
			allowedConfigOptions.insert(entry.first);
		}
		mParser.checkAllowedParams(allowedConfigOptions);
		return { mParser };
	}
};

} }

#endif //__IDX__BENCHMARK__BENCHMARK_HELPER__