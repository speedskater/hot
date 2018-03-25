//
//  @author robert.binna@uibk.ac.at
//

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <hot/commons/SingleMaskPartialKeyMapping.hpp>

#include <hot/testhelpers/PartialKeyMappingTestHelper.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(SingleMaskPartialKeyMappingTestSuite)

BOOST_AUTO_TEST_CASE(getMostLeftMostRightBitPositionTest) {
	auto mostLeftBitInformation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 0 });
	BOOST_REQUIRE_EQUAL(mostLeftBitInformation.mMostSignificantDiscriminativeBitIndex, 0);
	BOOST_REQUIRE_EQUAL(mostLeftBitInformation.mLeastSignificantDiscriminativeBitIndex, 0);

	auto mostRightBitInformation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 63 });
	BOOST_REQUIRE_EQUAL(mostRightBitInformation.mMostSignificantDiscriminativeBitIndex, 63);
	BOOST_REQUIRE_EQUAL(mostRightBitInformation.mLeastSignificantDiscriminativeBitIndex, 63);

	auto combined = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 0, 63 });
	BOOST_REQUIRE_EQUAL(combined.mMostSignificantDiscriminativeBitIndex, 0);
	BOOST_REQUIRE_EQUAL(combined.mLeastSignificantDiscriminativeBitIndex, 63);
}

BOOST_AUTO_TEST_CASE(extractMaskForMostSignificantBitTest) {
	std::array<uint8_t, 8> mostLeftBitArray = { 128, 0, 0, 0, 0, 0, 0, 0 };
	std::array<uint8_t, 8> mostRightBitArray = { 0, 0, 0, 0, 0, 0, 0, 1 };

	auto mostLeftBitInformation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 0 });
	BOOST_REQUIRE_EQUAL(mostLeftBitInformation.extractMask(mostLeftBitArray.data()), 1u);
	BOOST_REQUIRE_EQUAL(mostLeftBitInformation.extractMask(mostRightBitArray.data()), 0u);
}

BOOST_AUTO_TEST_CASE(testExtractMaskFromSuccessiveBytes1UsingASingleByte) {
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 0, 1, 2, 3, 4, 5, 6, 7 });
	auto anotherDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({	65, 68, 72, 76, 87, 96, 110, 127 });
	auto anotherDiscriminativeBitsRepresentationReverseInsert = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping( { 127, 110, 96, 87, 76, 72, 68, 65 });
	//                                                                            65 68      72  76             87                                 110                     127
	std::array<uint8_t, 16> anotherExtractionBytes1 = { 0, 0, 0, 0, 0, 0, 0, 0, 0b01001000, 0b10001000, 0b00000001, 0b00000000, 0b00000000, 0b00000010, 0b00000000, 0b00000001 };
	BOOST_REQUIRE_EQUAL(anotherDiscriminativeBitsRepresentation.extractMask(anotherExtractionBytes1.data()), 0b11011111l);
	BOOST_REQUIRE_EQUAL(anotherDiscriminativeBitsRepresentationReverseInsert.extractMask(anotherExtractionBytes1.data()), 0b11011111l);

	std::array<uint8_t, 16> anotherExtractionBytes2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0b01001000, 0b00001000, 0b00000001, 0b00000000, 0b00000000, 0b00000010, 0b00000000, 0b00000001 };
	BOOST_REQUIRE_EQUAL(anotherDiscriminativeBitsRepresentation.extractMask(anotherExtractionBytes2.data()), 0b11010111l);
	BOOST_REQUIRE_EQUAL(anotherDiscriminativeBitsRepresentationReverseInsert.extractMask(anotherExtractionBytes2.data()), 0b11010111l);
}

BOOST_AUTO_TEST_CASE(testExtractMaskFromSuccessiveBytes1UsingMultipleBytes) {
	auto extractionInformation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 34, 44, 45, 48, 61, 65, 68, 72, 75, 76, 87, 94, 95 });
	auto extractionInformationReverseInsert = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 95, 94, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44, 34 });
	auto extractionInformationPermutatedInsert = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 75, 48, 61, 61, 94, 95, 34, 87, 44, 76, 72, 68, 65, 45  });

	std::array<uint8_t, 13> someBitsSet = { 0, 0, 0, 0, 0b00000000, 0b00000100, 0b10000000, 0b00000100, 0b01001000, 0b10000000, 0b00000001, 0b00000000 };

	BOOST_REQUIRE_EQUAL(extractionInformation.extractMask(someBitsSet.data()), 0b0011001111010l);
	BOOST_REQUIRE_EQUAL(extractionInformationReverseInsert.extractMask(someBitsSet.data()), 0b0011001111010l);
	BOOST_REQUIRE_EQUAL(extractionInformationPermutatedInsert.extractMask(someBitsSet.data()), 0b0011001111010l);
}

BOOST_AUTO_TEST_CASE(testExtractMaskFromSuccessiveBytesUsingMultipleBytesShouldResultInRandomExtractInformation2) {
	//precondition
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44 });

	//postcondition
	auto extractionInformation = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 34, 44, 45, 48, 61, 65, 68, 72, 75, 76, 87, 95, 96 });
	auto randomDiscriminativeBitsRepresentation2 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44, 34 });

	//                                                               34            44,45   48               61      65 68      72  75,76           87          95   96
	std::array<uint8_t, 13> someExtractionBitsSet = { 0, 0, 0, 0, 0b00000000, 0b00001000, 0b10000000, 0b00000000, 0b01001000, 0b10001000, 0b00000001, 0b00000001, 0b00000000 };
	BOOST_REQUIRE_EQUAL(extractionInformation.extractMask(someExtractionBitsSet.data()), 0b0111011101100l);
	BOOST_REQUIRE_EQUAL(randomDiscriminativeBitsRepresentation2.extractMask(someExtractionBitsSet.data()), 0b0111011101100l);
}

BOOST_AUTO_TEST_CASE(testExtractSubInformationFromSuccessiveMask) {
	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({
	   96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44
	}), { 61, 45, 68, 87 });

	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({
	   96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44
	}), { 96, 72, 44 });

	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({
	   96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44
	}), { 96 });

	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({
	   96, 95, 87, 76, 75, 72, 68, 65, 61, 48, 45, 44
	}), { 44 });
}

BOOST_AUTO_TEST_CASE(testExecuteWithCorrectMaskAndDiscriminativeBitsRepresentation) {
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<SingleMaskPartialKeyMapping, uint8_t>()
	);
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96, 95, 87, 76, 75, 72, 68, 65 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<SingleMaskPartialKeyMapping, uint8_t>()
	);
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96, 95, 87, 76, 75, 72, 68, 65, 61 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<SingleMaskPartialKeyMapping, uint16_t>()
	);
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96, 95, 87, 76, 75, 72, 68, 65, 61, 62, 63, 64, 85, 86, 97, 98 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<SingleMaskPartialKeyMapping, uint16_t>()
	);
	hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 96, 95, 87, 76, 75, 72, 68, 65, 61, 62, 63, 64, 85, 86, 97, 98, 77 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<SingleMaskPartialKeyMapping, uint32_t>()
	);
}

BOOST_AUTO_TEST_CASE(testGetSubtreePrefixBitsWith1Bit) {
	SingleMaskPartialKeyMapping successiveDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({  44, 45, 48, 61, 62, 65, 68, 72, 75, 76, 87, 95, 96 });

	uint32_t prefixBits = successiveDiscriminativeBitsRepresentation.template getPrefixBitsMask<uint32_t>(hot::testhelpers::keyInformationForBit(62, 1));

	uint32_t subtreePrefixBits = hot::testhelpers::getMaskForBits(successiveDiscriminativeBitsRepresentation, {
		{ 44, true },
		{ 45, true },
		{ 48, true },
		{ 61, true }
	});

	BOOST_REQUIRE_EQUAL(prefixBits, subtreePrefixBits);

}

BOOST_AUTO_TEST_CASE(testGetPrefixBitsWith0Bit) {
	SingleMaskPartialKeyMapping successiveDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({  44, 45, 48, 61, 62, 63, 65, 68, 72, 75, 76, 87, 95, 96 });

	const uint32_t & prefixBits= successiveDiscriminativeBitsRepresentation.template getPrefixBitsMask<uint32_t>(
		hot::testhelpers::keyInformationForBit(62, 0));

	uint32_t subtreePrefixBits = hot::testhelpers::getMaskForBits(successiveDiscriminativeBitsRepresentation, {
		{ 44, true },
		{ 45, true },
		{ 48, true },
		{ 61, true }
	});

	BOOST_REQUIRE_EQUAL(prefixBits, subtreePrefixBits);
}


BOOST_AUTO_TEST_SUITE_END()

}}

