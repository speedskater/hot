#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <initializer_list>
#include <string>

#include "idx/benchmark/TempFile.hpp"

namespace idx { namespace benchmark {

TempFile::TempFile(std::initializer_list<std::string> const &lines) : mFileName(createUniqueTempFileName()) {
	initiTempFileContent(lines);
}

TempFile::~TempFile() {
	std::remove(mFileName.c_str());
}

inline std::string TempFile::createUniqueTempFileName() {
	char *tmpname = strdup("/tmp/TempFileXXXXXX");
	mkstemp(tmpname);
	return tmpname;
}

inline void TempFile::initiTempFileContent(std::initializer_list<std::string> const &lines) {
	std::ofstream ofStream(mFileName);
	for (std::string const &line : lines) {
		ofStream << line << std::endl;
	}
}

} }