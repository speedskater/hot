#ifndef __HOT__STRING_TEST_DATA__
#define __HOT__STRING_TEST_DATA__

#include <string>
#include <vector>

namespace hot { namespace rowex {

std::vector<const char*> const & getLongCStrings();
std::vector<std::string> const &getLongStrings();
std::vector<std::string> createLongStrings();

}}

#endif