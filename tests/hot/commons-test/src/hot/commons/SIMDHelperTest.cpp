
#include <array>
#include <cstdint>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <hot/commons/SIMDHelper.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(SIMDHelperTest)

uint8_t numberEntriesToPartial(size_t numberEntries) {
	assert(numberEntries < 8);
	uint8_t partialMask = 0u;
	switch(numberEntries) {
		case 0:
			partialMask = 0;
			break;
		case 1:
			partialMask = 0b1000'0000;
			break;
		case 2:
			partialMask = 0b1100'0000;
			break;
		case 3:
			partialMask = 0b1110'0000;
			break;
		case 4:
			partialMask = 0b1111'0000;
			break;
		case 5:
			partialMask = 0b1111'1000;
			break;
		case 6:
			partialMask = 0b1111'1100;
			break;
		case 7:
			partialMask = 0b1111'1110;
			break;
		default:
			assert(false);
	}
	return partialMask;
}

BOOST_AUTO_TEST_SUITE_END()

}}