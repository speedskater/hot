//
//  @author robert.binna@uibk.ac.at
//

#include <algorithm>
#include <iostream>
#include <bitset>

#include <boost/test/unit_test.hpp>
#include <hot/commons/DiscriminativeBit.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(DiscriminativeBitTest)

void requireSignificantKeyInformationToBe(
	DiscriminativeBit const & key,
	uint8_t extractionByte,
	uint16_t significantByteIndex,
	uint16_t byteRelativeSignificantBitIndex,
	uint16_t absoluteSignificantBitIndex,
	unsigned int newBitValue
) {
	BOOST_REQUIRE_EQUAL(key.getExtractionByte(), extractionByte);
	BOOST_REQUIRE_EQUAL(key.mByteIndex, significantByteIndex);
	BOOST_REQUIRE_EQUAL(key.mByteRelativeBitIndex, byteRelativeSignificantBitIndex);
	BOOST_REQUIRE_EQUAL(key.mAbsoluteBitIndex, absoluteSignificantBitIndex);
	BOOST_REQUIRE_EQUAL(key.mValue, newBitValue);
}


BOOST_AUTO_TEST_CASE(testSignificantKeyInformationConstructionByAbsoluteBitIndex) {
	requireSignificantKeyInformationToBe({ 42, 0u }, 0b00100000, 5, 2, 42, 0 );
	requireSignificantKeyInformationToBe({ 42, 1u }, 0b00100000, 5, 2, 42, 1 );
	requireSignificantKeyInformationToBe({ 0, 1u }, 0b10000000, 0, 0, 0, 1 );
	requireSignificantKeyInformationToBe({ (256 * 8) - 1, 0u }, 0b00000001, 255, 7, (256 * 8) - 1, 0 );
}


BOOST_AUTO_TEST_CASE(testSignifciantKeyInformationConstructionByByteIndexAndMissmatchingBytes) {
	//uint8_t const significantByteIndex, uint8_t const existingByte, uint8_t const newKeyByte
	requireSignificantKeyInformationToBe({ 42, 0b00000000, 0b00000001 }, 0b00000001, 42, 7, 343, 1);
	requireSignificantKeyInformationToBe({ 22, 0b00000001, 0b00000000 }, 0b00000001, 22, 7, 183, 0);
	requireSignificantKeyInformationToBe({ 19, 0b11111111, 0b10101010 }, 0b01000000, 19, 1, 153, 0);
	requireSignificantKeyInformationToBe({ 7, 0b11101111, 0b11111111 }, 0b00010000, 7, 3, 59, 1);
}

BOOST_AUTO_TEST_CASE(testSignifciantKeyInformationConstructionByMissmatch) {
	uint64_t first = 0ul;
	uint64_t second = 0ul;

	uint8_t* firstByteArray = reinterpret_cast<uint8_t *>(&first);
	uint8_t* secondByteArray = reinterpret_cast<uint8_t *>(&second);

	firstByteArray[1] = 0b00101010;
	secondByteArray[1] = 0b00011010;

	executeForDiffingKeys(firstByteArray, secondByteArray, 8u, [](DiscriminativeBit const & significantKeyInformation){
		requireSignificantKeyInformationToBe(significantKeyInformation, 0b00100000, 1, 2, 10, 0);
	});
}

BOOST_AUTO_TEST_SUITE_END()

}}