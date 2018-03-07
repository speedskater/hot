//
//  Created by Robert Binna
//
#define BOOST_TEST_DYN_LINK

#include <cstring>

#include <utility>

#include <boost/test/unit_test.hpp>
#include <spider/contenthelpers/PairKeyExtractor.hpp>

namespace spider { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(PairKeyExtractorTest)

BOOST_AUTO_TEST_CASE(testExtractCStringKeyFromPair) {
	spider::contenthelpers::PairKeyExtractor<std::pair<char const *, uint32_t>> extract;

	std::pair<char const *, uint32_t> pair { "test", 42 };

	BOOST_REQUIRE(strcmp(extract(pair), "test") == 0);
}

BOOST_AUTO_TEST_CASE(testExtractIntegerKeyFromPair) {
	spider::contenthelpers::PairKeyExtractor<std::pair<int, uint32_t>> extract;

	std::pair<int, uint32_t> pair { 4, 42 };

	BOOST_REQUIRE_EQUAL(extract(pair), 4);
}

BOOST_AUTO_TEST_SUITE_END()

}}