#ifndef __HOT__TESTHELPERS__PARTIAL_KEY_MAPPING_TEST_HELPER___
#define __HOT__TESTHELPERS__PARTIAL_KEY_MAPPING_TEST_HELPER___

#include <map>
#include <set>
#include <string>

#include <hot/commons/DiscriminativeBit.hpp>

#include <hot/commons/SingleMaskPartialKeyMapping.hpp>
#include <hot/commons/MultiMaskPartialKeyMapping.hpp>

namespace hot { namespace testhelpers {

hot::commons::DiscriminativeBit keyInformationForBit(uint16_t absoluteBitIndexToSet, bool bitValue = 1);

template<typename extractionInformation> inline std::string getExtractionTypeName();
template<typename PartialKeyType> inline std::string getPartialKeyTypeName();

template<> inline std::string getExtractionTypeName<hot::commons::SingleMaskPartialKeyMapping>() {
	return "SingleMaskPartialKeyMapping";
}

template<> inline std::string getExtractionTypeName<hot::commons::MultiMaskPartialKeyMapping<1u>>() {
	return "MultiMaskPartialKeyMapping<1u>";
}

template<> inline std::string getExtractionTypeName<hot::commons::MultiMaskPartialKeyMapping<2u>>() {
	return "MultiMaskPartialKeyMapping<2u>";
}

template<> inline std::string getExtractionTypeName<hot::commons::MultiMaskPartialKeyMapping<4u>>() {
	return "MultiMaskPartialKeyMapping<4u>";
}

template<> inline std::string getPartialKeyTypeName<uint8_t>() {
	return "uint8_t";
}

template<> inline std::string getPartialKeyTypeName<uint16_t>() {
	return "uint16_t";
}

template<> inline std::string getPartialKeyTypeName<uint32_t>() {
	return "uint32_t";
}

hot::commons::SingleMaskPartialKeyMapping getCheckedSingleMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList);
template<unsigned int numberExtractionMasks> hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks> getCheckedMultiMaskPartialKeyMapping(std::initializer_list<uint16_t> initializerList);

template<typename SourceExtractionInformationType, unsigned int numberExtractionMasks>
	hot::commons::MultiMaskPartialKeyMapping<numberExtractionMasks> getCheckedRandomSubInformation(
		SourceExtractionInformationType const & sourceExtractionInformation,
		std::initializer_list<uint16_t> const & bitsToExtract,
		bool requireThatAllExtractionBitsAreContained = true
	);

template<typename SourceExtractionInformationType> hot::commons::SingleMaskPartialKeyMapping getCheckedSuccessiveSubInformation(
	SourceExtractionInformationType const & sourceExtractionInformation,
	std::initializer_list<uint16_t> const & bitsToExtract,
	bool requireThatAllExtractionBitsAreContained = true
);

template<typename ExtractionInformationType> uint32_t getMaskForBits(ExtractionInformationType  const & extractionInformationType, std::map<uint16_t, bool>  const & bitsSet);
template<typename ExtractionInformationType> uint32_t getMaskWithAlteratingBitsSetForBitsWithIndexLargerToGivenIndex(ExtractionInformationType  const & extractionInformation, uint16_t bitIndexThreshold, bool valueOfFirstBit = true);
template<typename ExtractionInformationType> uint32_t getMaskForBitsWithIndexLargerThan(ExtractionInformationType  const & extractionInformation, uint16_t bitIndexThreshold);


template<typename ExpectedExtractionType, typename ExpectedPartialKeyType> class MaskAndDiscriminativeBitsRepresentationChecker {
public:
	template<typename AssumedExtractionType, typename AssumedPartialKeyType> void operator()(AssumedExtractionType const &, AssumedPartialKeyType) const {
		BOOST_REQUIRE_MESSAGE(typeid(AssumedExtractionType) == typeid(ExpectedExtractionType), "Assumed Extraction Type (" << getExtractionTypeName<AssumedExtractionType>() << ") was not expected Extraction Type (" << getExtractionTypeName<ExpectedExtractionType>() << ")");
		BOOST_REQUIRE_MESSAGE(typeid(AssumedPartialKeyType) == typeid(ExpectedPartialKeyType), "Assumed Mask Type (" << getPartialKeyTypeName<AssumedPartialKeyType>() << ") was not expected Mask Type (" << getPartialKeyTypeName<ExpectedPartialKeyType>() << ")");
	}
};

std::vector<char const *> stdStringsToCStrings(std::vector<std::string> const & stringsToInsert);


}}

#endif
