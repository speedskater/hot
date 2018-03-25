//
//  Created by Robert Binna
//

#include <iostream>
#include <map>
#include <typeinfo>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>

#include <idx/maphelpers/MapValueExtractor.hpp>

namespace idx { namespace maphelpers {

BOOST_AUTO_TEST_SUITE(MapValueExtractorTest)

BOOST_AUTO_TEST_CASE(testExtractValueForIdentityMapExtractor) {
	MapValueExtractor<uint64_t, idx::contenthelpers::IdentityKeyExtractor> extractValue;

	uint64_t extractedValue = extractValue(0ul);

	BOOST_REQUIRE(extractedValue == 0ul);
}

BOOST_AUTO_TEST_CASE(testExtractValueFromPair) {
	MapValueExtractor<std::pair<uint64_t, int32_t>, idx::contenthelpers::PairKeyExtractor> extractValue;

	BOOST_REQUIRE_EQUAL(extractValue({ 42u, -34 }), -34);
	BOOST_REQUIRE_EQUAL(extractValue({ 42u, 22 }), 22);
}

BOOST_AUTO_TEST_CASE(testExtractValueFromPionterPair) {
	MapValueExtractor<std::pair<uint64_t, int32_t>*, idx::contenthelpers::PairPointerKeyExtractor> extractValue;

	std::pair<uint64_t, int32_t> firstPair = { 42u, -815 };
	std::pair<uint64_t, int32_t> secondPair = { 13u, 10 };

	BOOST_REQUIRE_EQUAL(extractValue(&firstPair), &firstPair);
	BOOST_REQUIRE_EQUAL(extractValue(&secondPair), &secondPair);
}


BOOST_AUTO_TEST_SUITE_END()

}}