//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <hot/commons/Algorithms.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(AlgorithmsTest)

BOOST_AUTO_TEST_CASE(testGetLeastSignificantBitIndexInByte) {
	for(int i=0; i < 8; ++i) {
		BOOST_REQUIRE_EQUAL(hot::commons::getLeastSignificantBitIndexInByte(0b00000001 << (7-i)), i);
		BOOST_REQUIRE_EQUAL(hot::commons::getLeastSignificantBitIndexInByte(0b11111111 << (7-i)), i);
		BOOST_REQUIRE_EQUAL(hot::commons::getLeastSignificantBitIndexInByte(0b11111101 << (7-i)), i);
	}
}

BOOST_AUTO_TEST_CASE(testGetMostSignificantBitIndexInByte) {
	for(int i=0; i < 8; ++i) {
		BOOST_REQUIRE_EQUAL(hot::commons::getMostSignificantBitIndexInByte(0b10000000 >> i), i);
		BOOST_REQUIRE_EQUAL(hot::commons::getMostSignificantBitIndexInByte(0b11111111 >> i), i);
		BOOST_REQUIRE_EQUAL(hot::commons::getMostSignificantBitIndexInByte(0b10111111 >> i), i);
	}
}

BOOST_AUTO_TEST_SUITE_END()

} }