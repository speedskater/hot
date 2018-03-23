#ifndef __IDX__BENCHMARK__TEMP_FILE__HPP__
#define __IDX__BENCHMARK__TEMP_FILE__HPP__

#include <initializer_list>
#include <string>

namespace idx { namespace benchmark {

struct TempFile {
	std::string mFileName;

	TempFile(std::initializer_list<std::string> const & lines);
	~TempFile();

private:
	static std::string createUniqueTempFileName();
	void initiTempFileContent(std::initializer_list<std::string> const & lines);
};

}}

#endif