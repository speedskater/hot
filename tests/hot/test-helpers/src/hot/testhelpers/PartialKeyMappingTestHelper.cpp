#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <functional>

#include <boost/test/unit_test.hpp>

#include <hot/commons/DiscriminativeBit.hpp>

#include "hot/testhelpers/PartialKeyMappingTestHelper.hpp"

namespace hot { namespace testhelpers {

hot::commons::DiscriminativeBit keyInformationForBit(uint16_t absoluteBitIndexToSet, bool bitValue) {
	return { absoluteBitIndexToSet, bitValue };
}

template<size_t arraySize> std::array<uint8_t, arraySize> getInitializedArray(uint8_t valueToInitializeElementsWith) {
	std::array<uint8_t, arraySize> array;
	std::fill(array.begin(), array.end(), valueToInitializeElementsWith);
	return array;
};

template<typename ResultType> ResultType expectReturnTypeForOperation(auto extractionInformation, std::string const & operationName) {
	if(typeid(extractionInformation) !=  typeid(ResultType)) {
		BOOST_FAIL(operationName << " should have resulted in " << hot::testhelpers::getExtractionTypeName<ResultType>() << " but was " << hot::testhelpers::getExtractionTypeName<decltype(extractionInformation)>());
	}
	auto extractionInformationPointer = &extractionInformation;
	ResultType const* expectedReturnTypePointer = reinterpret_cast<ResultType const*>(extractionInformationPointer);
	return *expectedReturnTypePointer;
}

template<typename ResultType> ResultType getExtractionInformationForBits(auto extractionInformation, auto it, auto end) {
	if(it != end) {
		return extractionInformation.insert(keyInformationForBit(*(it++)), [&](auto const newExtractionInformation) {
			return getExtractionInformationForBits<ResultType>(newExtractionInformation, it, end);
		});
	} else {
		return expectReturnTypeForOperation<ResultType>(extractionInformation, "Insertion");
	}
}

std::array<uint8_t, 256> getRawBytesWithBitsSet(std::set<uint16_t> bitPositions) {
	std::array<uint8_t, 256> rawBytes = getInitializedArray<256>(0u);
	for(uint16_t absoluteBitPosition : bitPositions) {
		auto keyInformation = keyInformationForBit(absoluteBitPosition);
		rawBytes[keyInformation.mByteIndex] |= keyInformation.getExtractionByte();
	}
	return rawBytes;
}

std::array<uint8_t, 256> getRawBytesWithSingleBitSet(uint16_t bitPosition) {
	std::set<uint16_t> bits;
	bits.insert(bitPosition);
	return getRawBytesWithBitsSet(bits);
}


template<size_t arraySize> std::array<uint8_t, arraySize> invert(std::array<uint8_t, arraySize> arrayToInvert) {
	std::array<uint8_t, arraySize> invertedArray;
	for(size_t i=0; i < arraySize; ++i) {
		invertedArray[i] = ~arrayToInvert[i];
	}
	return invertedArray;
};

template<unsigned int numberExtractionMasks> void checkRandomExtractionTypeSpecificConstraints(hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks> const & extractionInformation) {
	int expectedNumberExtractionBits = extractionInformation.calculateNumberBitsUsed();
	int allExtractionBitsSet = 0;

	for(int i=0; i < extractionInformation.getNumberExtractionBytes(); ++i) {
		uint8_t extractionByte =  extractionInformation.getExtractionByte(i);
		BOOST_REQUIRE_NE(extractionByte, 0);

		allExtractionBitsSet += _mm_popcnt_u32(extractionByte);
	}
	BOOST_REQUIRE_EQUAL(allExtractionBitsSet, expectedNumberExtractionBits);
}

template<typename ExtractionInformationType> void checkExtractionTypeSpecificConstraints(ExtractionInformationType const & extractionInformation);

template<> void checkExtractionTypeSpecificConstraints(hot::commons::SingleMaskPartialKeyMapping const &) {
}

template<> void checkExtractionTypeSpecificConstraints(hot::commons::MultiMaskPartialKeyMapping<1> const & extractionInformation) {
	checkRandomExtractionTypeSpecificConstraints(extractionInformation);
}

template<> void checkExtractionTypeSpecificConstraints(hot::commons::MultiMaskPartialKeyMapping<2> const & extractionInformation) {
	checkRandomExtractionTypeSpecificConstraints(extractionInformation);
}

template<> void checkExtractionTypeSpecificConstraints(hot::commons::MultiMaskPartialKeyMapping<4> const & extractionInformation) {
	checkRandomExtractionTypeSpecificConstraints(extractionInformation);
}

template<typename ExtractionInformationType> void checkExtractionInformationOnExpectedBits(ExtractionInformationType const & extractionInformation, std::set<uint16_t> const & expectedBits) {
	std::array<uint8_t, 256> allBytesZero = getInitializedArray<256>(0u);
	uint32_t zeroMask = extractionInformation.extractMask(allBytesZero.data());
	BOOST_REQUIRE_EQUAL(zeroMask, static_cast<uint32_t>(0u));

	BOOST_REQUIRE_EQUAL(extractionInformation.calculateNumberBitsUsed(), expectedBits.size());

	std::set<uint16_t> resultingExtractionBits = extractionInformation.getDiscriminativeBits();
	BOOST_REQUIRE_EQUAL_COLLECTIONS(resultingExtractionBits.begin(), resultingExtractionBits.end(), expectedBits.begin(), expectedBits.end());

	uint32_t allBitsSet = UINT32_MAX >> (32 - expectedBits.size());
	std::array<uint8_t, 256> allBytesSet = getInitializedArray<256>(UINT8_MAX);
	BOOST_REQUIRE_EQUAL(extractionInformation.extractMask(allBytesSet.data()), allBitsSet);

	std::array<uint8_t, 256> onlyExtractionBitsSet = getRawBytesWithBitsSet(expectedBits);
	BOOST_REQUIRE_EQUAL(extractionInformation.extractMask(onlyExtractionBitsSet.data()), allBitsSet);


	std::array<uint8_t, 256> allBitsExceptExtractionBitsSet = invert(onlyExtractionBitsSet);
	BOOST_REQUIRE_EQUAL(extractionInformation.extractMask(allBitsExceptExtractionBitsSet.data()), 0u);

	uint32_t extractedMask = 0u;
	for(uint16_t singleExtractionBitPosition : expectedBits) {
		std::array<uint8_t, 256> rawDataWithSingleExtractionBit = getRawBytesWithSingleBitSet(singleExtractionBitPosition);
		uint32_t singleExtractedBit = extractionInformation.extractMask(rawDataWithSingleExtractionBit.data());
		BOOST_REQUIRE_EQUAL(singleExtractedBit & extractedMask, 0u);
		BOOST_REQUIRE_EQUAL(extractionInformation.getMaskFor({ singleExtractionBitPosition, 1 }), singleExtractedBit);
		extractedMask = singleExtractedBit | extractedMask;

		BOOST_REQUIRE_EQUAL(extractionInformation.getLeastSignificantBitIndex(extractedMask), singleExtractionBitPosition);
	}
	BOOST_REQUIRE_EQUAL(extractedMask, allBitsSet);

	uint32_t reverseExtractedMask = 0u;
	int expectedNumberReverseMasksBits=1;
	for(auto reverseBitsPositionIterator = expectedBits.rbegin(); reverseBitsPositionIterator != expectedBits.rend(); ++reverseBitsPositionIterator) {
		std::array<uint8_t, 256> rawDataWithSingleExtractionBit = getRawBytesWithSingleBitSet(*reverseBitsPositionIterator);
		uint32_t singleExtractedBit = extractionInformation.extractMask(rawDataWithSingleExtractionBit.data());
		BOOST_REQUIRE_EQUAL(_mm_popcnt_u32(singleExtractedBit), 1);
		BOOST_REQUIRE_EQUAL(singleExtractedBit & reverseExtractedMask, 0u);

		reverseExtractedMask |= singleExtractedBit;
		BOOST_REQUIRE_EQUAL(_mm_popcnt_u32(reverseExtractedMask), expectedNumberReverseMasksBits);
		BOOST_REQUIRE_EQUAL(extractionInformation.getMostSignifikantMaskBit(reverseExtractedMask), singleExtractedBit);
		++expectedNumberReverseMasksBits;
	}



	BOOST_REQUIRE_EQUAL(_mm_popcnt_u32(extractionInformation.getAllMaskBits()), extractionInformation.calculateNumberBitsUsed());
	BOOST_REQUIRE_EQUAL(extractionInformation.getAllMaskBits(), allBitsSet);

	uint16_t expectedMostLeftBit = *expectedBits.begin();
	uint16_t expectedMostRightBit = *expectedBits.rbegin();

	BOOST_REQUIRE_EQUAL(extractionInformation.mMostSignificantDiscriminativeBitIndex, expectedMostLeftBit);
	BOOST_REQUIRE_EQUAL(extractionInformation.mLeastSignificantDiscriminativeBitIndex, expectedMostRightBit);

	std::array<uint8_t, 256> rawDataWithHighestBitSet = getRawBytesWithSingleBitSet(expectedMostLeftBit);
	BOOST_REQUIRE_EQUAL(extractionInformation.getMaskForHighestBit(), extractionInformation.extractMask(rawDataWithHighestBitSet.data()));



	checkExtractionTypeSpecificConstraints<ExtractionInformationType>(extractionInformation);
}

template<typename ResultType> ResultType getExtractionInformationForBits(std::initializer_list<uint16_t> initializerList) {
	assert(initializerList.size() >= 1);

	auto it = initializerList.begin();
	hot::commons::SingleMaskPartialKeyMapping extractionInformation { keyInformationForBit(*it) };
	ResultType const & result = getExtractionInformationForBits<ResultType>(extractionInformation, ++it, initializerList.end());

	checkExtractionInformationOnExpectedBits(result, { initializerList.begin(), initializerList.end() });

	return result;
}

hot::commons::SingleMaskPartialKeyMapping getCheckedSingleMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList) {
	return getExtractionInformationForBits<hot::commons::SingleMaskPartialKeyMapping>(initializerList);
}

template<unsigned int numberExtractionMasks> hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks> getCheckedMultiMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList) {
	return getExtractionInformationForBits<hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks>>(initializerList);
}

template hot::commons::MultiMaskPartialKeyMapping<1> getCheckedMultiMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList);
template hot::commons::MultiMaskPartialKeyMapping<2> getCheckedMultiMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList);
template hot::commons::MultiMaskPartialKeyMapping<4> getCheckedMultiMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList);

template<typename SourceExtractionInformationType, typename ResultExtractionInformationType> ResultExtractionInformationType extractCheckedSubInformation(
	SourceExtractionInformationType sourceExtractionInformation, std::initializer_list<uint16_t> const & bitsToExtract, bool requireThatAllExtractionBitsAreContained
) {
	std::set<uint16_t> uniqueBitsToExtract(bitsToExtract);
	std::set<uint16_t> existingBits = sourceExtractionInformation.getDiscriminativeBits();

	std::set<uint16_t> validExtractionBits;
	std::set_intersection(uniqueBitsToExtract.begin(), uniqueBitsToExtract.end(), existingBits.begin(), existingBits.end(), std::inserter(validExtractionBits, validExtractionBits.end()));

	if(requireThatAllExtractionBitsAreContained) {
		BOOST_REQUIRE_EQUAL_COLLECTIONS(validExtractionBits.begin(), validExtractionBits.end(), uniqueBitsToExtract.begin(), uniqueBitsToExtract.end());
	}

	BOOST_REQUIRE_GE(validExtractionBits.size(), 1u);

	std::array<uint8_t, 256> rawExtractionBytesForValidExtractionBits =  getRawBytesWithBitsSet(validExtractionBits);
	uint32_t extractionMask = sourceExtractionInformation.extractMask(rawExtractionBytesForValidExtractionBits.data());

	BOOST_REQUIRE_EQUAL(static_cast<unsigned int>(_mm_popcnt_u32(extractionMask)), validExtractionBits.size());

	ResultExtractionInformationType resultingExtractionMask = sourceExtractionInformation.extract(extractionMask, [](auto intermediateExtractionMask){
		return expectReturnTypeForOperation<ResultExtractionInformationType>(intermediateExtractionMask, "Extract");
	});

	checkExtractionInformationOnExpectedBits(resultingExtractionMask, validExtractionBits);

	return resultingExtractionMask;
};

template<typename SourceExtractionInformationType, unsigned int numberExtractionMasks> hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks>
getCheckedRandomSubInformation(
	SourceExtractionInformationType const & sourceExtractionInformation,
	std::initializer_list<uint16_t> const & bitsToExtract,
	bool requireThatAllExtractionBitsAreContained
)
{
	return extractCheckedSubInformation<SourceExtractionInformationType, hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks>>(
		sourceExtractionInformation, bitsToExtract, requireThatAllExtractionBitsAreContained
	);
}

template<typename SourceExtractionInformationType> hot::commons::SingleMaskPartialKeyMapping
getCheckedSuccessiveSubInformation(
	SourceExtractionInformationType const & sourceExtractionInformation,
	std::initializer_list<uint16_t> const & bitsToExtract,
	bool requireThatAllExtractionBitsAreContained
)
{
	return extractCheckedSubInformation<SourceExtractionInformationType, hot::commons::SingleMaskPartialKeyMapping>(
		sourceExtractionInformation, bitsToExtract, requireThatAllExtractionBitsAreContained
	);
}

template<typename ExtractionInformationType> uint32_t getMaskForBits(ExtractionInformationType const & extractionInformationType, std::map<uint16_t, bool> const & bitsSet) {
	std::set<uint16_t> bitsToSet;
	int numberBitsSet = 0;
	for(auto bit : bitsSet) {
		if(bit.second) {
			++numberBitsSet;
			bitsToSet.insert(bit.first);
		}
	}
	std::array<uint8_t, 256> rawExtractionBytesForValidExtractionBits =  getRawBytesWithBitsSet(bitsToSet);
	uint32_t resultingMask = extractionInformationType.extractMask(rawExtractionBytesForValidExtractionBits.data());
	BOOST_REQUIRE_EQUAL(_mm_popcnt_u32(resultingMask), numberBitsSet);
	return resultingMask;
}

template<typename ExtractionInformationType> uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex(ExtractionInformationType const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit) {
	unsigned int bitsSet = 0u;
	unsigned int bitsNotSet = 0u;
	std::set<uint16_t> bitsToSet;
	bool valueOfCurrentBit = valueOfFirstBit;
	for(uint16_t absolutBitIndex : extractionInformation.getDiscriminativeBits()) {
		if(bitIndexThreshold < absolutBitIndex && valueOfCurrentBit) {
			++bitsSet;
			bitsToSet.insert(absolutBitIndex);
		} else {
			++bitsNotSet;
		}
		valueOfCurrentBit = (!valueOfCurrentBit);
	}
	std::array<uint8_t, 256> rawExtractionBytesForValidExtractionBits =  getRawBytesWithBitsSet(bitsToSet);
	uint32_t resultingMask = extractionInformation.extractMask(rawExtractionBytesForValidExtractionBits.data());
	BOOST_REQUIRE_EQUAL(static_cast<unsigned int>(_mm_popcnt_u32(resultingMask)), bitsSet);

	return resultingMask;
}

template<typename ExtractionInformationType> uint32_t getMaskForBitsWithIndexLargerThan(ExtractionInformationType const & extractionInformation, uint16_t bitIndexThreshold) {
	std::set<uint16_t> allBitsSet = extractionInformation.getDiscriminativeBits();
	std::set<uint16_t> bitsLargerThanTreshold;

	std::copy_if(
		allBitsSet.begin(), allBitsSet.end(),
		std::inserter(bitsLargerThanTreshold, bitsLargerThanTreshold.end()),
		std::bind2nd(std::greater<uint16_t>(), bitIndexThreshold)
	);



	std::array<uint8_t, 256> rawExtractionBytesForBitsLargerThanTreshold =  getRawBytesWithBitsSet(bitsLargerThanTreshold);
	uint32_t bitsLargerThanThresholdMask = extractionInformation.extractMask(rawExtractionBytesForBitsLargerThanTreshold.data());
	BOOST_REQUIRE_EQUAL(static_cast<unsigned int>(_mm_popcnt_u32(bitsLargerThanThresholdMask)), bitsLargerThanTreshold.size());

	return bitsLargerThanThresholdMask;
}

std::vector<char const *> stdStringsToCStrings(std::vector<std::string> const & stringsToInsert) {
	std::vector<char const *> rawStringsToIndex;

	for(std::string const & string : stringsToInsert) {
		rawStringsToIndex.push_back(string.c_str());
	}

	return rawStringsToIndex;
}


template hot::commons::SingleMaskPartialKeyMapping getCheckedSuccessiveSubInformation<hot::commons::SingleMaskPartialKeyMapping>(hot::commons::SingleMaskPartialKeyMapping const &, const std::initializer_list<uint16_t> & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::SingleMaskPartialKeyMapping getCheckedSuccessiveSubInformation<hot::commons::MultiMaskPartialKeyMapping<1>>(hot::commons::MultiMaskPartialKeyMapping<1> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::SingleMaskPartialKeyMapping getCheckedSuccessiveSubInformation<hot::commons::MultiMaskPartialKeyMapping<2>>(hot::commons::MultiMaskPartialKeyMapping<2> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::SingleMaskPartialKeyMapping getCheckedSuccessiveSubInformation<hot::commons::MultiMaskPartialKeyMapping<4>>(hot::commons::MultiMaskPartialKeyMapping<4> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);

template hot::commons::MultiMaskPartialKeyMapping<1> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<1>, 1>(hot::commons::MultiMaskPartialKeyMapping<1> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::MultiMaskPartialKeyMapping<1> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<2>, 1>(hot::commons::MultiMaskPartialKeyMapping<2> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::MultiMaskPartialKeyMapping<1> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<4>, 1>(hot::commons::MultiMaskPartialKeyMapping<4> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);

template hot::commons::MultiMaskPartialKeyMapping<2> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<2>, 2>(hot::commons::MultiMaskPartialKeyMapping<2> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);
template hot::commons::MultiMaskPartialKeyMapping<2> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<4>, 2>(hot::commons::MultiMaskPartialKeyMapping<4> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);

template hot::commons::MultiMaskPartialKeyMapping<4> getCheckedRandomSubInformation<hot::commons::MultiMaskPartialKeyMapping<4>, 4>(hot::commons::MultiMaskPartialKeyMapping<4> const &, std::initializer_list<uint16_t> const & initializerList, bool requireThatAllExtractionBitsAreContained);

template uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex<hot::commons::SingleMaskPartialKeyMapping>(hot::commons::SingleMaskPartialKeyMapping const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit);
template uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex<hot::commons::MultiMaskPartialKeyMapping<1>>(hot::commons::MultiMaskPartialKeyMapping<1> const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit);
template uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex<hot::commons::MultiMaskPartialKeyMapping<2>>(hot::commons::MultiMaskPartialKeyMapping<2> const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit);
template uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex<hot::commons::MultiMaskPartialKeyMapping<4>>(hot::commons::MultiMaskPartialKeyMapping<4> const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit);

template uint32_t getMaskForBits<hot::commons::SingleMaskPartialKeyMapping>(hot::commons::SingleMaskPartialKeyMapping const &, std::map<uint16_t, bool> const & bitsSet);
template uint32_t getMaskForBits<hot::commons::MultiMaskPartialKeyMapping<1>>(hot::commons::MultiMaskPartialKeyMapping<1> const &, std::map<uint16_t, bool> const & bitsSet);
template uint32_t getMaskForBits<hot::commons::MultiMaskPartialKeyMapping<2>>(hot::commons::MultiMaskPartialKeyMapping<2> const &, std::map<uint16_t, bool> const & bitsSet);
template uint32_t getMaskForBits<hot::commons::MultiMaskPartialKeyMapping<4>>(hot::commons::MultiMaskPartialKeyMapping<4> const &, std::map<uint16_t, bool> const & bitsSet);

template uint32_t getMaskForBitsWithIndexLargerThan<hot::commons::SingleMaskPartialKeyMapping>(hot::commons::SingleMaskPartialKeyMapping const &, uint16_t);
template uint32_t getMaskForBitsWithIndexLargerThan<hot::commons::MultiMaskPartialKeyMapping<1>>(hot::commons::MultiMaskPartialKeyMapping<1> const &, uint16_t);
template uint32_t getMaskForBitsWithIndexLargerThan<hot::commons::MultiMaskPartialKeyMapping<2>>(hot::commons::MultiMaskPartialKeyMapping<2> const &, uint16_t);
template uint32_t getMaskForBitsWithIndexLargerThan<hot::commons::MultiMaskPartialKeyMapping<4>>(hot::commons::MultiMaskPartialKeyMapping<4> const &, uint16_t);

/*template<typename SourceExtractionInformation, typename ResultExtractionInforamtion> ResultExtractionInforamtion extractBits(
	SourceExtractionInformation const & extractionInformation, std::initializer_list<uint16_t> initializerList
) {
	std::set<uint16_t> uniqueIndexes
}

extractSuccessiveBits(extractionInformation, { 34, 45, 68, 87 })*/

}}
