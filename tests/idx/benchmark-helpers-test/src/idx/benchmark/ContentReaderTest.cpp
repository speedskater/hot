//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <string>
#include <sstream>
#include <utility>

#include <idx/benchmark/ContentReader.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(ContentReaderTest)

BOOST_AUTO_TEST_CASE(testReadInt) {
	char targetMemory[4096]; //uninitialized page must be enough for test scenario
	ContinuousBuffer const & buffer = ContinuousBuffer(&targetMemory, 4096);
	std::istringstream input("123456");
	std::pair<uint64_t*, ContinuousBuffer> contentLocation = buffer.template allocate<uint64_t>();
	const ContinuousBuffer & remainingBuffer = ContentReader<uint64_t>::read(input, contentLocation);
	BOOST_REQUIRE_EQUAL(contentLocation.second.mRemainingBufferSize, remainingBuffer.mRemainingBufferSize);
	BOOST_REQUIRE_EQUAL(contentLocation.second.mRemainingBufferSize, 4096u - 8u);
	BOOST_REQUIRE_EQUAL(*contentLocation.first, 123456u);
}

BOOST_AUTO_TEST_CASE(testReadString) {
	std::istringstream input("\"string \\\"content\\\"\"");
	char targetMemory[4096]; //uninitialized page must be enough for test scenario
	ContinuousBuffer const & buffer = ContinuousBuffer(&targetMemory, 4096);
	char * content;
	ContinuousBuffer remainingBuffer = ContentReader<char*>::read(input, std::make_pair(&content, buffer));
	BOOST_REQUIRE_EQUAL(content, "string \"content\"");
	BOOST_REQUIRE_EQUAL(remainingBuffer.mRemainingBufferSize, 4096u - (strlen("string \"content\"") + 1));
}

BOOST_AUTO_TEST_CASE(testReadConstString) {
	std::istringstream input("\"string \\\"content\\\"\"");
	char targetMemory[4096]; //uninitialized page must be enough for test scenario
	ContinuousBuffer const & buffer = ContinuousBuffer(&targetMemory, 4096);
	char const * content;
	ContinuousBuffer remainingBuffer = ContentReader<const char*>::read(input, std::make_pair(&content, buffer));
	BOOST_REQUIRE_EQUAL(content, "string \"content\"");
	BOOST_REQUIRE_EQUAL(remainingBuffer.mRemainingBufferSize, 4096u - (strlen("string \"content\"") + 1));
}


BOOST_AUTO_TEST_CASE(testReadPair) {
	std::istringstream input("42 \"fourty Two\"");
	char targetMemory[4096]; //uninitialized page must be enough for test scenario
	ContinuousBuffer const & buffer = ContinuousBuffer(&targetMemory, 4096);
	const std::pair<std::pair<uint64_t, const char *> *, ContinuousBuffer> &contentLocation = buffer.allocate<std::pair<uint64_t, char const *>>();
	ContinuousBuffer remainingBuffer = ContentReader<std::pair<uint64_t, const char *>>::read(input, contentLocation);
	BOOST_REQUIRE_EQUAL(contentLocation.first->first, 42u);
	BOOST_REQUIRE_EQUAL(contentLocation.first->second, "fourty Two");
	//8u sizeof uint64_t
	size_t keySize = 8u;
	//8u sizeof char* pointer rest is length of string + 1 for zero termination
	size_t valueSize = (strlen("fourty Two") + 1u + 8u);
	BOOST_REQUIRE_EQUAL(remainingBuffer.mRemainingBufferSize, 4096u - keySize - valueSize);
}

BOOST_AUTO_TEST_CASE(testStringStringPair) {
	std::istringstream input("\"zweiundvierzig\" \"fourty Two\"");
	char targetMemory[4096]; //uninitialized page must be enough for test scenario
	ContinuousBuffer const & buffer = ContinuousBuffer(&targetMemory, 4096);
	const std::pair<std::pair<const char*, const char *> *, ContinuousBuffer> &contentLocation = buffer.allocate<std::pair<const char*, char const *>>();
	ContinuousBuffer remainingBuffer = ContentReader<std::pair<const char*, const char *>>::read(input, contentLocation);
	BOOST_REQUIRE_EQUAL(contentLocation.first->first, "zweiundvierzig");
	BOOST_REQUIRE_EQUAL(contentLocation.first->second, "fourty Two");
	//8u sizeof char* pointer rest is length of string + 1 for zero termination
	size_t keySize = (strlen("zweiundvierzig") + 1u + 8u);
	//8u sizeof char* pointer rest is length of string + 1 for zero termination
	size_t valueSize = (strlen("fourty Two") + 1u + 8u);
	BOOST_REQUIRE_EQUAL(remainingBuffer.mRemainingBufferSize, 4096u - keySize - valueSize);
}

BOOST_AUTO_TEST_SUITE_END()

}}