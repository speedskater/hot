//
//  @author robert.binna@uibk.ac.at
//

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <hot/commons/MultiMaskPartialKeyMapping.hpp>

#include <hot/testhelpers/PartialKeyMappingTestHelper.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(MultiMaskPartialKeyMappingTest)

BOOST_AUTO_TEST_CASE(testExtractMaskFromRandomBytesStartingWithSuccessiveRandom1AndLeadingToRandom1) {
	auto successiveDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedSingleMaskPartialKeyMapping({ 75, 48, 61, 96, 95  });
	//                                                                                     48               61                   75                          95   96
	std::array<uint8_t, 13> allExtractionBitsSet1 = { 0, 0, 0, 0, 0b00000000, 0b00000000, 0b10000000, 0b00000100, 0b00000000, 0b00010000, 0b00000000, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(successiveDiscriminativeBitsRepresentation.extractMask(allExtractionBitsSet1.data()), 0b11111l);

	auto randomDiscriminativeBitsRepresentation2 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 75, 48, 61, 96, 95, 34  });
	//                                                               34                    48               61                   75                          95   96
	std::array<uint8_t, 13> allExtractionBitsSet2 = { 0, 0, 0, 0, 0b00100000, 0b00000000, 0b10000000, 0b00000100, 0b00000000, 0b00010000, 0b00000000, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(randomDiscriminativeBitsRepresentation2.extractMask(allExtractionBitsSet2.data()), 0b111111l);

	auto randomDiscriminativeBitsRepresentation3 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 75, 48, 61, 96, 95, 34, 87  });
	//                                                               34                    48               61                   75                87          95   96
	std::array<uint8_t, 13> allExtractionBitsSet3 = { 0, 0, 0, 0, 0b00100000, 0b00000000, 0b10000000, 0b00000100, 0b00000000, 0b00010000, 0b00000001, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(randomDiscriminativeBitsRepresentation3.extractMask(allExtractionBitsSet3.data()), 0b1111111l);

	auto randomDiscriminativeBitsRepresentation4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 75, 48, 61, 96, 95, 34, 87, 44  });
	//                                                               34             44      48               61                     75              87          95   96
	std::array<uint8_t, 13> allExtractionBitsSet4 = { 0, 0, 0, 0, 0b00100000, 0b00001000, 0b10000000, 0b00000100, 0b00000000, 0b00010000, 0b00000001, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(randomDiscriminativeBitsRepresentation4.extractMask(allExtractionBitsSet4.data()), 0b11111111l);
}

BOOST_AUTO_TEST_CASE(testInsertIntoMultiMaskPartialKeyMapping1InsertingIntoExistingBytePosition) {
	auto randomDiscriminativeBitsRepresentation5 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 75, 48, 61, 96, 95, 34, 87, 44, 76  });
	//                                                               34             44      48               61                    75,76           87          95   96
	std::array<uint8_t, 13> allExtractionBitsSet5 = { 0, 0, 0, 0, 0b00100000, 0b00001000, 0b10000000, 0b00000100, 0b00000000, 0b00011000, 0b00000001, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(randomDiscriminativeBitsRepresentation5.extractMask(allExtractionBitsSet5.data()), 0b111111111l);
}

BOOST_AUTO_TEST_CASE(testExtractMaskFromRandomBytesStartingWithSuccessiveRandom1AndLeadingToRandom2) {
	auto extractionInformationPermutatedInsert = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 75, 48, 61, 61, 96, 95, 34, 87, 44, 76, 72, 68, 65, 45  });

	//                                                               34            44,45   48               61      65 68      72 75,76           87          95   96
	std::array<uint8_t, 13> allExtractionBitsSet = { 0, 0, 0, 0, 0b00100000, 0b00001100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10000000 };
	BOOST_REQUIRE_EQUAL(extractionInformationPermutatedInsert.extractMask(allExtractionBitsSet.data()), 0b1111111111111l);
}

/**
 * The decision that extraction informatino for more than 16 bit are automatically upgrade to random 4 is solely made by the SIMDCobTrieNode
 */
BOOST_AUTO_TEST_CASE(testExtractMaskFromRandomBytesStartingWithSuccessiveRandom2AndLeadingToRandom2ByteMoreThan16Bits) {
	auto extractionInformationRandom2 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 75, 48, 61, 61, 96, 95, 34, 87, 44, 76, 100, 72, 68, 65, 15, 42, 45  });
	//                                                                   15           34         42 44,45   48               61      65 68      72 75,76           87          95   96  100
	std::array<uint8_t, 13> allExtractionBitsSetForRandom2 { 0, 0b00000001, 0, 0, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000 };
	BOOST_REQUIRE_EQUAL(extractionInformationRandom2.extractMask(allExtractionBitsSetForRandom2.data()), 0b1111111111111111l);

	auto extractionInformationRandom4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 75, 48, 61, 61, 96, 95, 34, 87, 44, 76, 100, 72, 68, 65, 15, 42, 45, 2  });
	//                                                           2                15           34         42 44,45   48               61      65 68      72 75,76           87          95   96  100
	std::array<uint8_t, 13> allExtractionBitsSetForRandom4 { 0b00100000, 0b00000001, 0, 0, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000 };
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4.extractMask(allExtractionBitsSetForRandom4.data()), 0b11111111111111111l);
}

BOOST_AUTO_TEST_CASE(testInsertRandomExtractionByteAfterAllExistingBytes) {
	auto extractionInformationRandom2 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141  });
	//                                                           2                15         22         34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 18> allExtractionBitsSetForRandom2 { 0b00100000, 0b00000001, 0b00000010, 0, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                       0           1           2           3   4           5          6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom2.extractMask(allExtractionBitsSetForRandom2.data()), 0b1111111111111111111111l);

}

BOOST_AUTO_TEST_CASE(testExtractMaskFromRandomBytesStartingWithSuccessiveRandom2AndLeadingToRandom4ByMoreThan16RandomBytes) {
	auto extractionInformationRandom2 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141  });
	//                                                           2                15         22         34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 18> allExtractionBitsSetForRandom2 { 0b00100000, 0b00000001, 0b00000010, 0, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                       0           1           2           3   4           5          6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom2.extractMask(allExtractionBitsSetForRandom2.data()), 0b1111111111111111111111l);

	auto extractionInformationRandom4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24  });

	//                                                           2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 18> allExtractionBitsSetForRandom4 { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                       0           1           2           3           4           5          6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4.extractMask(allExtractionBitsSetForRandom4.data()), 0b11111111111111111111111l);
}

BOOST_AUTO_TEST_CASE(testExtractInformationFromRandomBytesInsertOnRandom4Behind) {
	auto extractionInformationRandom4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24  });
	//                                                           2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 18> allExtractionBitsSetForRandom4 { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                       0           1           2           3           4           5          6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4.extractMask(allExtractionBitsSetForRandom4.data()), 0b11111111111111111111111l);

	auto extractionInformationRandom4InsertBehind = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24, 160  });
	//                                                           2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141                            160
	std::array<uint8_t, 21> allExtractionBitsSetForRandom4Behind { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100, 0b00000000, 0b00000000, 0b10000000 };
	//                                                       0           1           2           3           4           5          6           7           8           9           10          11          12          13          14          15          16          17           18          19          20
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4InsertBehind.extractMask(allExtractionBitsSetForRandom4Behind.data()), 0b111111111111111111111111l);
}

BOOST_AUTO_TEST_CASE(testExtractInformationFromRandomBytesInsertOnRandom4Before) {
	auto extractionInformationRandom4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 160, 141, 24  });
	//                                                                                  15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141                            160
	std::array<uint8_t, 21> allExtractionBitsSetForRandom4 { 0b00000000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100, 0b00000000, 0b00000000, 0b10000000 };
	//                                                       0           1           2           3           4           5          6           7           8           9           10          11          12          13          14          15          16          17           18          19          20
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4.extractMask(allExtractionBitsSetForRandom4.data()), 0b11111111111111111111111l);

	auto extractionInformationRandom4InsertBefore = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24, 160  });
	//                                                                 2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141                            160
	std::array<uint8_t, 21> allExtractionBitsSetForRandom4Before { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100, 0b00000000, 0b00000000, 0b10000000 };
	//                                                       0           1           2           3           4           5          6           7           8           9           10          11          12          13          14          15          16          17           18          19          20
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4InsertBefore.extractMask(allExtractionBitsSetForRandom4Before.data()), 0b111111111111111111111111l);
}

BOOST_AUTO_TEST_CASE(testExtractInformationFromRandomBytesInsertOnRandom4InBetween) {
	auto extractionInformationRandom4 = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24  });
	//                                                           2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 18> allExtractionBitsSetForRandom4 { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b00000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                       0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4.extractMask(allExtractionBitsSetForRandom4.data()), 0b11111111111111111111111l);

	auto extractionInformationRandom4InsertInBetween = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24, 104  });
	//                                                                    2                15         22    24            34         42 44,45   48               61      65 68      72 75,76           87          95   96  100                  113               127  128              141
	std::array<uint8_t, 21> allExtractionBitsSetForRandom4InBetween { 0b00100000, 0b00000001, 0b00000010, 0b10000000, 0b00100000, 0b00101100, 0b10000000, 0b00000100, 0b01001000, 0b10011000, 0b00000001, 0b00000001, 0b10001000, 0b10000000, 0b01000000, 0b00000001, 0b10000000, 0b00000100 };
	//                                                                0           1           2           3           4           5           6           7           8           9           10          11          12          13          14          15          16          17
	BOOST_REQUIRE_EQUAL(extractionInformationRandom4InsertInBetween.extractMask(allExtractionBitsSetForRandom4InBetween.data()), 0b111111111111111111111111l);
}

BOOST_AUTO_TEST_CASE(testExtractSuccessiveSubInformationFromRandomMask1) {
	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({
	    75, 48, 61, 96, 95, 34, 87, 44, 76
	}), { 75, 48, 61, 96 });
}

BOOST_AUTO_TEST_CASE(testExtractRandom1SubInformationFromRandomMask1) {
	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<1>, 1>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({
	    75, 48, 61, 96, 95, 34, 87, 44, 76
	}), { 96, 34 });

	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<1>, 1>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({
		75, 48, 61, 98, 97, 34, 35, 87, 44, 76
	}), { 97, 35 });
}

BOOST_AUTO_TEST_CASE(testExtractSuccessiveSubInformationFromRandomMask2) {
	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({
	    75, 22, 48, 61, 61, 96, 94, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141
	}), { 75, 45, 61, 94 });

	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({
		75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141
	}), { 75, 42, 61, 95 });
}

BOOST_AUTO_TEST_CASE(testExtractRandom1SubInformationFromRandomMask2) {
	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<2>, 1>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({
	    75, 22, 48, 61, 61, 96, 94, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141
	}), { 75, 48, 61, 96, 95, 34, 87, 44, 76 });

	hot::testhelpers::getCheckedSuccessiveSubInformation(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({
		75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141
	}), { 75, 42, 61, 95 });
}

BOOST_AUTO_TEST_CASE(testExtractSuccessiveSubInformationFromRandomMask4) {
	hot::testhelpers::getCheckedSuccessiveSubInformation<MultiMaskPartialKeyMapping<4>>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({
	   75, 22, 23, 48, 61, 61, 96, 95, 34, 35, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24
	}), { 75, 48, 61, 61, 95, 35 });
}

BOOST_AUTO_TEST_CASE(testExtractRandom1SubInformationFromRandomMask4) {
	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<4>, 1>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({
		75, 22, 23, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24
	}), { 75, 23, 48, 61, 61, 95, 34 });
}

BOOST_AUTO_TEST_CASE(testExtractRandom2SubInformationFromRandomMask4) {
	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<4>, 2>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({
		75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24
	}), { 75, 22, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141 });

	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<4>, 2>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({
	   75, 22, 48, 61, 61, 96, 95, 34, 87, 126, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24
	}), { 75, 48, 61, 61, 96, 95, 34, 87, 126, 44, 76, 100, 72, 68, 65, 113, 15, 42, 45, 2, 141, 24 });
}

BOOST_AUTO_TEST_CASE(testExtractRandom4SubInformationFromRandomMask4) {
	hot::testhelpers::getCheckedRandomSubInformation<MultiMaskPartialKeyMapping<4>, 4>(hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({
		75, 22, 23, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 163, 162, 24
	}), { 75, 23, 48, 61, 61, 96, 95, 34, 87, 127, 44, 76, 100, 128, 72, 68, 65, 113, 15, 42, 45, 2, 141, 162 });
}

BOOST_AUTO_TEST_CASE(testExecuteWithCorrectMaskAndExtractionRandomMask1) {
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 1, 96 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<1>, uint8_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 1, 96, 95, 87, 76, 75, 72, 68 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<1>, uint8_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 1, 96, 95, 87, 76, 75, 72, 68, 65 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<1>, uint16_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 1, 96, 95, 87, 76, 75, 72, 68, 65, 61, 62, 63, 64, 85, 86, 97 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<1>, uint16_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>({ 1, 96, 95, 87, 76, 75, 72, 68, 65, 61, 62, 63, 64, 85, 86, 97, 98 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<1>, uint32_t>()
	);
}

BOOST_AUTO_TEST_CASE(testExecuteWithCorrectMaskAndExtractionRandomMask2) {
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 0, 8, 16, 24, 32, 40, 48, 56, 64 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<2>, uint16_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<2>, uint16_t>()
	);
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>({ 0, 4, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<4>, uint32_t>()
	);
}

BOOST_AUTO_TEST_CASE(testExecuteWithCorrectMaskAndExtractionRandomMask4) {
	hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>({ 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128 }).executeWithCorrectMaskAndDiscriminativeBitsRepresentation(
		hot::testhelpers::MaskAndDiscriminativeBitsRepresentationChecker<MultiMaskPartialKeyMapping<4>, uint32_t>()
	);
}

BOOST_AUTO_TEST_CASE(testGetPrefixBitsMaskRandom1) {
	for(int newBitValue = 0; newBitValue < 2; ++newBitValue) {
		MultiMaskPartialKeyMapping<1> randomDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<1>(
			{1, 44, 45, 48, 96, 95, 87, 68, 65, 61, 62, 63, 64, 85, 86, 97});

		uint32_t subtreeBits = hot::testhelpers::getMaskForBits(randomDiscriminativeBitsRepresentation, {
			{1, true},
			{44, true},
			{45, true},
			{48, true},
			{61, true}
		});
		uint32_t prefixBits = randomDiscriminativeBitsRepresentation.template getPrefixBitsMask<uint32_t>(hot::testhelpers::keyInformationForBit(62, newBitValue));

		BOOST_REQUIRE_EQUAL(prefixBits, subtreeBits);
	}
}

BOOST_AUTO_TEST_CASE(testGetPrefixBitsMaskRandom2) {
	for(int newBitValue = 0; newBitValue < 2; ++newBitValue) {
		MultiMaskPartialKeyMapping<2> randomDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<2>(
			{1, 44, 45, 48, 96, 95, 87, 75, 76, 77, 68, 65, 61, 62, 63, 64, 85, 86, 97});
		uint32_t subtreePrefixBits = hot::testhelpers::getMaskForBits(randomDiscriminativeBitsRepresentation, {
			{1, true},
			{44, true},
			{45, true},
			{48, true},
			{61, true},
			{62, true},
			{63, true},
			{64, true},
			{65, true},
			{68, true},
			{75, true},
			{76, true},
			{77, true},
			{85, true}
		});

		uint32_t prefixBits = randomDiscriminativeBitsRepresentation.template getPrefixBitsMask<uint32_t>(hot::testhelpers::keyInformationForBit(86, newBitValue));
		BOOST_REQUIRE_EQUAL(prefixBits, subtreePrefixBits);

	}
}

BOOST_AUTO_TEST_CASE(testGetPrefixBitsMaskRandom4) {
	for(int newBitValue = 0; newBitValue < 2; ++newBitValue) {
		MultiMaskPartialKeyMapping<4> randomDiscriminativeBitsRepresentation = hot::testhelpers::getCheckedMultiMaskPartialKeyMapping<4>(
			{1, 44, 45, 48, 96, 95, 87, 75, 76, 77, 68, 65, 61, 62, 63, 64, 85, 86, 97, 110, 120, 130, 140, 150, 150, 170, 180, 190, 200});
		uint32_t subtreePrefixBits = hot::testhelpers::getMaskForBits(randomDiscriminativeBitsRepresentation, {
			{1, true},
			{44, true},
			{45, true},
			{48, true},
			{61, true},
			{62, true},
			{63, true},
			{64, true},
			{65, true},
			{68, true},
			{75, true},
			{76, true},
			{77, true},
			{85, true},
			{86, true},
			{87, true},
			{95, true},
			{96, true},
			{97, true},
			{110, true},
			{120, true},
			{130, true},
			{140, true},
			{150, true},
		});
		uint32_t prefixBits = randomDiscriminativeBitsRepresentation.template getPrefixBitsMask<uint32_t>(hot::testhelpers::keyInformationForBit(155, newBitValue));
		BOOST_REQUIRE_EQUAL(prefixBits, subtreePrefixBits);
	}
}

BOOST_AUTO_TEST_SUITE_END()

}}