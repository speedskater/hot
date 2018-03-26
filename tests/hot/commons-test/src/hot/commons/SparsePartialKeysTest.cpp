//
//  @author robert.binna@uibk.ac.at
//

#include <cstdlib>

#include <initializer_list>
#include <iostream>
#include <memory>

#include <sstream>
#include <boost/test/unit_test.hpp>

#include <hot/commons/SparsePartialKeys.hpp>

constexpr size_t MAXIMUM_NUMBER_MASK_ENTRIES = 32;
constexpr uint32_t ALL_ENTRIES_USED_MASK = 0xFFFFFFFF;

namespace hot { namespace commons {

template<typename PartialKeyType> SparsePartialKeys<PartialKeyType>* allocateMasks() {
	size_t rawSize = sizeof(PartialKeyType) * MAXIMUM_NUMBER_MASK_ENTRIES;

	void* memoryForMasks = nullptr;
	unsigned int error = posix_memalign(&memoryForMasks, alignof(SparsePartialKeys<PartialKeyType>), rawSize);
	if(error != 0) {
		std::cout << "Got error on alignment" << std::endl;
		exit(1);
	}

	return reinterpret_cast<SparsePartialKeys<PartialKeyType>*>(memoryForMasks);
}

BOOST_AUTO_TEST_SUITE(EntriesMasksTest)

BOOST_AUTO_TEST_CASE(getRelevantBitsForEntriesSimple) {
	SparsePartialKeys<uint8_t>* entriesMasks = allocateMasks<uint8_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = 0;
	}

	entriesMasks->mEntries[MAXIMUM_NUMBER_MASK_ENTRIES - 1] = 1;

	BOOST_REQUIRE_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0);
	BOOST_REQUIRE_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), 1);

	free(entriesMasks);
}

BOOST_AUTO_TEST_CASE(getRelevantBitsForEntries) {
	SparsePartialKeys<uint8_t>* entriesMasks = allocateMasks<uint8_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = static_cast<uint8_t>(1 << i);
	}


	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 2u), 2);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 2u), 4);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 3u), 6);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), 0xFE);

	free(entriesMasks);
}

BOOST_AUTO_TEST_CASE(getRelevantBitsForEntries8BitsAndIgnoreIrreleantBits) {
	SparsePartialKeys<uint8_t>* entriesMasks = allocateMasks<uint8_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = (1 << i) | 2;
	}
	std::swap(entriesMasks->mEntries[0], entriesMasks->mEntries[1]);

	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 2u), 1);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 2u), 4);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 3u), 5);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), 0b11111101);

	free(entriesMasks);
}


BOOST_AUTO_TEST_CASE(getRelevantBitsForEntries16Bit) {
	SparsePartialKeys<uint16_t>* entriesMasks = allocateMasks<uint16_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = 1 << i;
	}

	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 1u), 0);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 2u), 2);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 2u), 4);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 3u), 6);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), 0xFFFE);

	free(entriesMasks);
}

BOOST_AUTO_TEST_CASE(getRelevantBitsForEntries32Bit) {
	SparsePartialKeys<uint32_t>* entriesMasks = allocateMasks<uint32_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = 1 << i;
	}


	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 1u), 0u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 2u), 2u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 2u), 4u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 3u), 6u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), ALL_ENTRIES_USED_MASK - 1u);

	free(entriesMasks);
}

BOOST_AUTO_TEST_CASE(getBitsForEntries32BitAndIgnoreIrreleantBits) {
	SparsePartialKeys<uint32_t>* entriesMasks = allocateMasks<uint32_t>();

	for(size_t i=0; i < MAXIMUM_NUMBER_MASK_ENTRIES; ++i) {
		entriesMasks->mEntries[i] = (1 << i) | 1;
	}

	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 1u), 0u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 1u), 0u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 2u), 2u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 2u), 4u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 3u), 6u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 32u), ALL_ENTRIES_USED_MASK - 1u);

	free(entriesMasks);
}

BOOST_AUTO_TEST_CASE(getRelevantBitsFor32BitWithRealisticSubtree) {
	SparsePartialKeys<uint32_t>* entriesMasks = allocateMasks<uint32_t>();
	//                          01234567
	entriesMasks->mEntries[0] = 0b00000000u;
	entriesMasks->mEntries[1] = 0b00000001u;
	entriesMasks->mEntries[2] = 0b00001000u;
	entriesMasks->mEntries[3] = 0b00001010u;
	entriesMasks->mEntries[4] = 0b01000000u;
	entriesMasks->mEntries[5] = 0b01100000u;
	entriesMasks->mEntries[6] = 0b01110000u;
	entriesMasks->mEntries[7] = 0b10000000u;
	entriesMasks->mEntries[8] = 0b10000100u;
	entriesMasks->mEntries[9] = 0b10000110u;

	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 10u), 0b11111111u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 9u),  0b11111110u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(3u, 3u),  0b01100000u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 4u),  0b01001010u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 7u),  0b11111010u);
}

BOOST_AUTO_TEST_CASE(getRelevantBitsFor32BitWithRealisticSubtreePermutated) {
	SparsePartialKeys<uint32_t>* entriesMasks = allocateMasks<uint32_t>();
	//                          74253106
	entriesMasks->mEntries[0] = 0b00000000u;
	entriesMasks->mEntries[1] = 0b10000000u;
	entriesMasks->mEntries[2] = 0b01000000u;
	entriesMasks->mEntries[3] = 0b01000001u;
	entriesMasks->mEntries[4] = 0b00000100u;
	entriesMasks->mEntries[5] = 0b00100100u;
	entriesMasks->mEntries[6] = 0b00101100u;
	entriesMasks->mEntries[7] = 0b00000010u;
	entriesMasks->mEntries[8] = 0b00010010u;
	entriesMasks->mEntries[9] = 0b00010011u;

	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(0u, 10u), 0b11111111u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 9u),  0b01111111u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(3u, 3u),  0b00100100u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 4u),  0b01000101u);
	BOOST_CHECK_EQUAL(entriesMasks->getRelevantBitsForRange(1u, 7u),  0b01101111u);
}


BOOST_AUTO_TEST_CASE(testEstimateSizeForMasks8) {
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(1), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(2), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(8), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(9), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(16), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(17), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(24), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(25), 32u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint8_t>::estimateSize(32), 32u);
}

BOOST_AUTO_TEST_CASE(testEstimateSizeForMasks16) {
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(1), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(4), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(5), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(8), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(9), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(12), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(13), 32u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(16), 32u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(17), 40u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(20), 40u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(21), 48u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(24), 48u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(25), 56u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(28), 56u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(29), 64u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint16_t>::estimateSize(32), 64u);
}

BOOST_AUTO_TEST_CASE(testEstimateSizeForMasks32) {
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(1), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(2), 8u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(3), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(4), 16u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(5), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(6), 24u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(7), 32u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(8), 32u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(9), 40u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(10), 40u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(11), 48u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(12), 48u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(13), 56u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(14), 56u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(15), 64u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(16), 64u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(17), 72u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(18), 72u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(19), 80u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(20), 80u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(21), 88u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(22), 88u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(23), 96u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(24), 96u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(25), 104u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(26), 104u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(27), 112u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(28), 112u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(29), 120u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(30), 120u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(31), 128u);
	BOOST_REQUIRE_EQUAL(SparsePartialKeys<uint32_t>::estimateSize(32), 128u);
}

uint32_t getMaskForAllUsedEntries(size_t numberEntries) {
	return UINT32_MAX >> (32 - numberEntries);
}

template<typename PartialKeyType> std::shared_ptr<SparsePartialKeys<PartialKeyType>> getEntriesMasks(std::initializer_list<PartialKeyType> const & masks) {
	BOOST_REQUIRE_GT(masks.size(), 1u);
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks { new (masks.size()) SparsePartialKeys<PartialKeyType>() };
	std::copy(masks.begin(), masks.end(), entriesMasks->mEntries);
	return entriesMasks;
}

template<typename PartialKeyType> uint32_t searchMatchingEntries(std::shared_ptr<SparsePartialKeys<PartialKeyType>> const & entriesMasks, size_t numberEntries, PartialKeyType needle) {
	return entriesMasks->search(needle) & getMaskForAllUsedEntries(numberEntries);
}

template<typename PartialKeyType> uint32_t findMasksByPattern(std::shared_ptr<SparsePartialKeys<PartialKeyType>> const & entriesMasks, size_t numberEntries, PartialKeyType searchMask) {
	return entriesMasks->findMasksByPattern(searchMask) & getMaskForAllUsedEntries(numberEntries);
}

template<typename PartialKeyType> uint32_t getAffectedSubtreeMask(std::shared_ptr<SparsePartialKeys<PartialKeyType>> const & entriesMasks, size_t numberEntries, PartialKeyType usedPrefixBits, PartialKeyType expectedPrefixBitValues) {
	return entriesMasks->getAffectedSubtreeMask(usedPrefixBits, expectedPrefixBitValues) & getMaskForAllUsedEntries(numberEntries);
}


template<typename PartialKeyType> void testMaskWith8BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks2Entries = getEntriesMasks<PartialKeyType>(std::initializer_list<PartialKeyType> { static_cast<PartialKeyType>(0b00000000), static_cast<PartialKeyType>(0b00000001) });
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks2Entries, 2, 0b00000000), 0b01u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks2Entries, 2, 0b00000001), 0b11u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks2Entries, 2, 0b00000011), 0b11u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks2Entries, 2, 0b11111111), 0b11u);

	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks6Entries = getEntriesMasks<PartialKeyType>({
		static_cast<PartialKeyType>(0b00000000),
		static_cast<PartialKeyType>(0b00000010),
		static_cast<PartialKeyType>(0b00001000),
		static_cast<PartialKeyType>(0b00001001),
		static_cast<PartialKeyType>(0b00001100),
		static_cast<PartialKeyType>(0b00001110)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00000000), 0b000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00000010), 0b000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001000), 0b000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001001), 0b001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001100), 0b010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001110), 0b110111u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001101), 0b011101u);


	std::shared_ptr<SparsePartialKeys<PartialKeyType>> permutatedEntriesMasks6Entries = getEntriesMasks<PartialKeyType>(std::initializer_list<PartialKeyType> {
		static_cast<PartialKeyType>(0b00000000),
		static_cast<PartialKeyType>(0b00000010),
		static_cast<PartialKeyType>(0b00000001),
		static_cast<PartialKeyType>(0b00001001),
		static_cast<PartialKeyType>(0b00000101),
		static_cast<PartialKeyType>(0b00000111)
	});

	//permutated entries
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000000), 0b000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000010), 0b000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000001), 0b000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00001001), 0b001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000101), 0b010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000111), 0b110111u);
}


template<typename PartialKeyType> void testFindMasksByPatternWith8BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks2Entries = getEntriesMasks<PartialKeyType>(std::initializer_list<PartialKeyType> {
		static_cast<PartialKeyType>(0b00000000),
		static_cast<PartialKeyType>(0b00000001)
	});
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks2Entries, 2, 0b00000000), 0b11u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks2Entries, 2, 0b00000001), 0b10u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks2Entries, 2, 0b00000011), 0b00u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks2Entries, 2, 0b11111111), 0b00u);

	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks6Entries = getEntriesMasks<PartialKeyType>({
	 static_cast<PartialKeyType>(0b00000000),
	 static_cast<PartialKeyType>(0b00000010),
	 static_cast<PartialKeyType>(0b00001000),
	 static_cast<PartialKeyType>(0b00001001),
	 static_cast<PartialKeyType>(0b00001100),
	 static_cast<PartialKeyType>(0b00001110)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00000000), 0b111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00000010), 0b100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00001000), 0b111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00001001), 0b001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00001100), 0b110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks6Entries, 6, 0b00001110), 0b100000u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks6Entries, 6, 0b00001101), 0b011101u);


	std::shared_ptr<SparsePartialKeys<PartialKeyType>> permutatedEntriesMasks6Entries = getEntriesMasks<PartialKeyType>(std::initializer_list<PartialKeyType> {
		static_cast<PartialKeyType>(0b00000000),
		static_cast<PartialKeyType>(0b00000010),
		static_cast<PartialKeyType>(0b00000001),
		static_cast<PartialKeyType>(0b00001001),
		static_cast<PartialKeyType>(0b00000101),
		static_cast<PartialKeyType>(0b00000111)
	});

	//permutated entries
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000000), 0b111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000010), 0b100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000001), 0b111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00001001), 0b001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000101), 0b110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(permutatedEntriesMasks6Entries, 6, 0b00000111), 0b100000u);
}


void testPrintMasksWith8BitSet() {
	std::shared_ptr<SparsePartialKeys<uint8_t>> entriesMasks6Entries = getEntriesMasks<uint8_t>({
	  static_cast<uint8_t>(0b00000000),
	  static_cast<uint8_t>(0b00000010),
	  static_cast<uint8_t>(0b00000001),
	  static_cast<uint8_t>(0b00001001),
	  static_cast<uint8_t>(0b00000101),
	  static_cast<uint8_t>(0b00000111),
	});

	std::ostringstream masksOutput;
	entriesMasks6Entries->printMasks(getMaskForAllUsedEntries(6), masksOutput);

	std::ostringstream expectedStrings;

	expectedStrings << "mask[0] = " <<  "00000000" << std::endl;
	expectedStrings << "mask[1] = " <<  "00000010" << std::endl;
	expectedStrings << "mask[2] = " <<  "00000001" << std::endl;
	expectedStrings << "mask[3] = " <<  "00001001" << std::endl;
	expectedStrings << "mask[4] = " <<  "00000101" << std::endl;
	expectedStrings << "mask[5] = " <<  "00000111" << std::endl;


	BOOST_REQUIRE_EQUAL(masksOutput.str(), expectedStrings.str());

	std::shared_ptr<SparsePartialKeys<uint8_t>> permutatedEntriesMasks11Entries = getEntriesMasks<uint8_t>({
		static_cast<uint8_t>(0b00000000), //0
		static_cast<uint8_t>(0b00000100), //1
		static_cast<uint8_t>(0b00000010), //2
		static_cast<uint8_t>(0b00010010), //3
		static_cast<uint8_t>(0b00001010), //4
		static_cast<uint8_t>(0b00001110), //5
	});


	//the mapping is from target bit -> source Bit
	//be aware the source bit index is little endian and the target bit index is big endian
	std::map<uint16_t, uint16_t> masksBitMappings {
		{ 0, 0 },
		{ 1, 7 },
		{ 2, 6 },
		{ 3, 5 },
		{ 4, 4 },
		{ 5, 3 },
		{ 6, 2 },
		{ 7, 1 }
	};

	std::ostringstream expectedPermutatedString;

	expectedPermutatedString << "mask[0] = \toriginal: "  << "00000000\tmapped: 00000000" << std::endl;
	expectedPermutatedString << "mask[1] = \toriginal: "  << "00000100\tmapped: 00000010" << std::endl;
	expectedPermutatedString << "mask[2] = \toriginal: "  << "00000010\tmapped: 00000001" << std::endl;
	expectedPermutatedString << "mask[3] = \toriginal: "  << "00010010\tmapped: 00001001" << std::endl;
	expectedPermutatedString << "mask[4] = \toriginal: "  << "00001010\tmapped: 00000101" << std::endl;
	expectedPermutatedString << "mask[5] = \toriginal: "  << "00001110\tmapped: 00000111" << std::endl;

	std::ostringstream permutatedMasks;
	permutatedEntriesMasks11Entries->printMasks(getMaskForAllUsedEntries(6), masksBitMappings, permutatedMasks);

	BOOST_REQUIRE_EQUAL(permutatedMasks.str(), expectedPermutatedString.str());
}

template<typename PartialKeyType> void testGetAffectedSubtreeMaskWith8BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks6Entries = getEntriesMasks<PartialKeyType>({
	 static_cast<PartialKeyType>(0b00000000),
	 static_cast<PartialKeyType>(0b00000010),
	 static_cast<PartialKeyType>(0b00010000),
	 static_cast<PartialKeyType>(0b00010100),
	 static_cast<PartialKeyType>(0b00011000),
	 static_cast<PartialKeyType>(0b00011001)
	});

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks6Entries, 6, 0b11110000, 0b00010000), 0b111100u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks6Entries, 6, 0b11111110, 0b00000000), 0b000001u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks6Entries, 6, 0b11111110, 0b00000010), 0b000010u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks6Entries, 6, 0b11111000, 0b00011000), 0b110000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks6Entries, 6, 0b11111111, 0b00011001), 0b100000u);
}

template<typename PartialKeyType> void testMaskWith16BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks12Entries = getEntriesMasks<PartialKeyType>({
		static_cast<PartialKeyType>(0b0000000000000000),
		static_cast<PartialKeyType>(0b0000000000000010),
		static_cast<PartialKeyType>(0b0000000000001000),
		static_cast<PartialKeyType>(0b0000000000001001),
		static_cast<PartialKeyType>(0b0000000000001100),
		static_cast<PartialKeyType>(0b0000000000001110),
		static_cast<PartialKeyType>(0b0000001000000010),
		static_cast<PartialKeyType>(0b0000100000001000),
		static_cast<PartialKeyType>(0b0000100100001001),
		static_cast<PartialKeyType>(0b0000110000001100),
		static_cast<PartialKeyType>(0b0000111100000000),
		static_cast<PartialKeyType>(0b0000111000001110)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000000000), 0b000000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000000010), 0b000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001000), 0b000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001001), 0b000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001100), 0b000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001110), 0b000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000001000000010), 0b000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000100000001000), 0b000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000100100001001), 0b000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000110000001100), 0b001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111100000000), 0b010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111000001110), 0b101011110111u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000110000000011), 0b000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111111000010), 0b010001000011u);
	
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks12EntriesPermutated = getEntriesMasks<PartialKeyType>({
		static_cast<PartialKeyType>(0b000000000000000),
		static_cast<PartialKeyType>(0b000000000000001),
		static_cast<PartialKeyType>(0b001000000000000),
		static_cast<PartialKeyType>(0b101000000000000),
		static_cast<PartialKeyType>(0b001000000000010),
		static_cast<PartialKeyType>(0b001000000000011),
		static_cast<PartialKeyType>(0b000000100000001),
		static_cast<PartialKeyType>(0b001000000010000),
		static_cast<PartialKeyType>(0b101000010010000),
		static_cast<PartialKeyType>(0b001001000010010),
		static_cast<PartialKeyType>(0b000001110010000),
		static_cast<PartialKeyType>(0b001001100010011)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000000000000), 0b000000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000000000001), 0b000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000000), 0b000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b101000000000000), 0b000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000010), 0b000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000011), 0b000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000100000001), 0b000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000010000), 0b000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b101000010010000), 0b000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001001000010010), 0b001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000001110010000), 0b010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001001100010011), 0b101011110111u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b100001000010001), 0b000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000001111110001), 0b010001000011u);
}

template<typename PartialKeyType> void testFindMasksByPatternWith16BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks12Entries = getEntriesMasks<PartialKeyType>({
		static_cast<PartialKeyType>(0b0000000000000000),
		static_cast<PartialKeyType>(0b0000000000000010),
		static_cast<PartialKeyType>(0b0000000000001000),
		static_cast<PartialKeyType>(0b0000000000001001),
		static_cast<PartialKeyType>(0b0000000000001100),
		static_cast<PartialKeyType>(0b0000000000001110),
		static_cast<PartialKeyType>(0b0000001000000010),
		static_cast<PartialKeyType>(0b0000100000001000),
		static_cast<PartialKeyType>(0b0000100100001001),
		static_cast<PartialKeyType>(0b0000110000001100),
		static_cast<PartialKeyType>(0b0000111100000000),
		static_cast<PartialKeyType>(0b0000111000001110)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000000000), 0b111111111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000000010), 0b100001100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001000), 0b101110111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001001), 0b000100001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001100), 0b101000110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000000000001110), 0b100000100000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000001000000010), 0b100001000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000100000001000), 0b101110000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000100100001001), 0b000100000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000110000001100), 0b101000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111100000000), 0b010000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111000001110), 0b100000000000u);

	//artificial
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000110000000011), 0b000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000111111000010), 0b000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12Entries, 12, 0b0000110000001000), 0b101000000000u);
	
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks12EntriesPermutated = getEntriesMasks<PartialKeyType>({
		static_cast<PartialKeyType>(0b000000000000000),
		static_cast<PartialKeyType>(0b000000000000001),
		static_cast<PartialKeyType>(0b001000000000000),
		static_cast<PartialKeyType>(0b101000000000000),
		static_cast<PartialKeyType>(0b001000000000010),
		static_cast<PartialKeyType>(0b001000000000011),
		static_cast<PartialKeyType>(0b000000100000001),
		static_cast<PartialKeyType>(0b001000000010000),
		static_cast<PartialKeyType>(0b101000010010000),
		static_cast<PartialKeyType>(0b001001000010010),
		static_cast<PartialKeyType>(0b000001110010000),
		static_cast<PartialKeyType>(0b001001100010011)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000000000000), 0b111111111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000000000001), 0b100001100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000000), 0b101110111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b101000000000000), 0b000100001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000010), 0b101000110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000000011), 0b100000100000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000000100000001), 0b100001000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001000000010000), 0b101110000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b101000010010000), 0b000100000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001001000010010), 0b101000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000001110010000), 0b010000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b001001100010011), 0b100000000000u);

	//artificial
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b100001000010001), 0b000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks12EntriesPermutated, 12, 0b000001111110001), 0b000000000000u);
}

template<typename PartialKeyType> void testGetAffectedSubtreeMaskWith16BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks11Entries = getEntriesMasks<PartialKeyType>({
	 static_cast<PartialKeyType>(0b0000000000000000),
	 static_cast<PartialKeyType>(0b0000000000000010),
	 static_cast<PartialKeyType>(0b0000000000010000),
	 static_cast<PartialKeyType>(0b0000000000010100),
	 static_cast<PartialKeyType>(0b0000000000011000),
	 static_cast<PartialKeyType>(0b0000000000011001),
	 static_cast<PartialKeyType>(0b0000001000000010),
	 static_cast<PartialKeyType>(0b0001000000010000),
	 static_cast<PartialKeyType>(0b0001010000010100),
	 static_cast<PartialKeyType>(0b0001100000011000),
	 static_cast<PartialKeyType>(0b0001100100011001)
	});

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111110000, 0b0000000000010000), 0b000000111100u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111111110, 0b0000000000000000), 0b000000000001u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111111110, 0b0000000000000010), 0b000000000010u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111111000, 0b0000000000011000), 0b000000110000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111111111, 0b0000000000011001), 0b000000100000u);

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b11111111111100, 0b0001000000010000), 0b000010000000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks11Entries, 11, 0b1111000000000000, 0b0001000000000000), 0b011110000000u);
}


void testPrintMasksWith16BitSet() {
	std::shared_ptr<SparsePartialKeys<uint16_t>> entriesMasks11Entries = getEntriesMasks<uint16_t>({
	  static_cast<uint16_t>(0b0000000000000000),
	  static_cast<uint16_t>(0b0000000000000010),
	  static_cast<uint16_t>(0b0000000000010000),
	  static_cast<uint16_t>(0b0000000000010100),
	  static_cast<uint16_t>(0b0000000000011000),
	  static_cast<uint16_t>(0b0000000000011001),
	  static_cast<uint16_t>(0b0000001000000010),
	  static_cast<uint16_t>(0b0001000000010000),
	  static_cast<uint16_t>(0b0001010000010100),
	  static_cast<uint16_t>(0b0001100000011000),
	  static_cast<uint16_t>(0b0001100100011001)
	});

	std::ostringstream masksOutput;
	entriesMasks11Entries->printMasks(getMaskForAllUsedEntries(11), masksOutput);

	std::ostringstream expectedStrings;

	expectedStrings << "mask[0] = " <<  "0000000000000000" << std::endl;
	expectedStrings << "mask[1] = " <<  "0000000000000010" << std::endl;
	expectedStrings << "mask[2] = " <<  "0000000000010000" << std::endl;
	expectedStrings << "mask[3] = " <<  "0000000000010100" << std::endl;
	expectedStrings << "mask[4] = " <<  "0000000000011000" << std::endl;
	expectedStrings << "mask[5] = " <<  "0000000000011001" << std::endl;
	expectedStrings << "mask[6] = " <<  "0000001000000010" << std::endl;
	expectedStrings << "mask[7] = " <<  "0001000000010000" << std::endl;
	expectedStrings << "mask[8] = " <<  "0001010000010100" << std::endl;
	expectedStrings << "mask[9] = " <<  "0001100000011000" << std::endl;
	expectedStrings << "mask[10] = " << "0001100100011001" << std::endl;

	BOOST_REQUIRE_EQUAL(masksOutput.str(), expectedStrings.str());

	std::shared_ptr<SparsePartialKeys<uint16_t>> permutatedEntriesMasks11Entries = getEntriesMasks<uint16_t>({
		static_cast<uint16_t>(0b0000000000000000), //0
		static_cast<uint16_t>(0b0000000000000100), //1
		static_cast<uint16_t>(0b0000000000100000), //2
		static_cast<uint16_t>(0b0000000000101000), //3
		static_cast<uint16_t>(0b0000000000110000), //4
		static_cast<uint16_t>(0b0000000000110010), //5
		static_cast<uint16_t>(0b0000010000000100), //6
		static_cast<uint16_t>(0b0010000000100000), //7
		static_cast<uint16_t>(0b0010100000101000), //8
		static_cast<uint16_t>(0b0011000000110000), //9
		static_cast<uint16_t>(0b0011001000110010) //10
	});


	//the mapping is from target bit -> source Bit
	//be aware the source bit index is little endian and the target bit index is big endian
	std::map<uint16_t, uint16_t> masksBitMappings {
		{ 0,   0  },
		{ 1,   15 },
		{ 2,   14 },
		{ 3,   13 },
		{ 4,   12 },
		{ 5,   11 },
		{ 6,   10 },
		{ 7,    9 },
		{ 8,    8 },
		{ 9,    7 },
		{ 10,   6 },
		{ 11,   5 },
		{ 12,   4 },
		{ 13,   3 },
		{ 14,   2 },
		{ 15,   1 },
	};

	std::ostringstream expectedPermutatedString;

	expectedPermutatedString << "mask[0] = \toriginal: "  << "0000000000000000\tmapped: 0000000000000000" << std::endl;
	expectedPermutatedString << "mask[1] = \toriginal: "  << "0000000000000100\tmapped: 0000000000000010" << std::endl;
	expectedPermutatedString << "mask[2] = \toriginal: "  << "0000000000100000\tmapped: 0000000000010000" << std::endl;
	expectedPermutatedString << "mask[3] = \toriginal: "  << "0000000000101000\tmapped: 0000000000010100" << std::endl;
	expectedPermutatedString << "mask[4] = \toriginal: "  << "0000000000110000\tmapped: 0000000000011000" << std::endl;
	expectedPermutatedString << "mask[5] = \toriginal: "  << "0000000000110010\tmapped: 0000000000011001" << std::endl;
	expectedPermutatedString << "mask[6] = \toriginal: "  << "0000010000000100\tmapped: 0000001000000010" << std::endl;
	expectedPermutatedString << "mask[7] = \toriginal: "  << "0010000000100000\tmapped: 0001000000010000" << std::endl;
	expectedPermutatedString << "mask[8] = \toriginal: "  << "0010100000101000\tmapped: 0001010000010100" << std::endl;
	expectedPermutatedString << "mask[9] = \toriginal: "  << "0011000000110000\tmapped: 0001100000011000" << std::endl;
	expectedPermutatedString << "mask[10] = \toriginal: " << "0011001000110010\tmapped: 0001100100011001" << std::endl;

	std::ostringstream permutatedMasks;
	permutatedEntriesMasks11Entries->printMasks(getMaskForAllUsedEntries(11), masksBitMappings, permutatedMasks);

	BOOST_REQUIRE_EQUAL(permutatedMasks.str(), expectedPermutatedString.str());
}

template<typename PartialKeyType> void testGetAffectedSubtreeMaskWith32BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks21Entries = getEntriesMasks<PartialKeyType>({
	 static_cast<PartialKeyType>(0b00000000000000000000000000000000),
	 static_cast<PartialKeyType>(0b00000000000000000000000000000010),
	 static_cast<PartialKeyType>(0b00000000000000000000000000010000),
	 static_cast<PartialKeyType>(0b00000000000000000000000000010100),
	 static_cast<PartialKeyType>(0b00000000000000000000000000011000),
	 static_cast<PartialKeyType>(0b00000000000000000000000000011001),
	 static_cast<PartialKeyType>(0b00000000000000000000001000000010),
	 static_cast<PartialKeyType>(0b00000000000000000001000000010000),
	 static_cast<PartialKeyType>(0b00000000000000000001010000010100),
	 static_cast<PartialKeyType>(0b00000000000000000001100000011000),
	 static_cast<PartialKeyType>(0b00000000000000000001100100011001),

	 static_cast<PartialKeyType>(0b00000000000000100000000000000010),
	 static_cast<PartialKeyType>(0b00000000000100000000000000010000),
	 static_cast<PartialKeyType>(0b00000000000101000000000000010100),
	 static_cast<PartialKeyType>(0b00000000000110000000000000011000),
	 static_cast<PartialKeyType>(0b00000000000110010000000000011001),
	 static_cast<PartialKeyType>(0b00000010000000100000001000000010),
	 static_cast<PartialKeyType>(0b00010000000100000001000000010000),
	 static_cast<PartialKeyType>(0b00010100000101000001010000010100),
	 static_cast<PartialKeyType>(0b00011000000110000001100000011000),
	 static_cast<PartialKeyType>(0b00011001000110010001100100011001)
	});

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111110000, 0b00000000000000000000000000010000), 0b00000000000000000111100u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111110, 0b00000000000000000000000000000000), 0b00000000000000000000001u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111110, 0b00000000000000000000000000000010), 0b00000000000000000000010u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111000, 0b00000000000000000000000000011000), 0b00000000000000000110000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111111, 0b00000000000000000000000000011001), 0b00000000000000000100000u);

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111100, 0b00000000000000000001000000010000), 0b00000000000000010000000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111000000000000, 0b00000000000000000001000000000000), 0b00000000000011110000000u);

	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11111111111111111111111111111100, 0b00010000000100000001000000010000), 0b00000100000000000000000u);
	BOOST_REQUIRE_EQUAL(getAffectedSubtreeMask<PartialKeyType>(entriesMasks21Entries, 21, 0b11110000000000000000000000000000, 0b00010000000000000000000000000000), 0b00111100000000000000000u);
}

template<typename PartialKeyType> void testMaskWith32BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks23Entries = getEntriesMasks<PartialKeyType>({
	  static_cast<PartialKeyType>(0b00000000000000000000000000000000),
	  static_cast<PartialKeyType>(0b00000000000000000000000000000010),
	  static_cast<PartialKeyType>(0b00000000000000000000000000001000),
	  static_cast<PartialKeyType>(0b00000000000000000000000000001001),
	  static_cast<PartialKeyType>(0b00000000000000000000000000001100),
	  static_cast<PartialKeyType>(0b00000000000000000000000000001110),
	  static_cast<PartialKeyType>(0b00000000000000000000001000000010),
	  static_cast<PartialKeyType>(0b00000000000000000000100000001000),
	  static_cast<PartialKeyType>(0b00000000000000000000100100001001),
	  static_cast<PartialKeyType>(0b00000000000000000000110000001100),
	  static_cast<PartialKeyType>(0b00000000000000000000111100000000),
	  static_cast<PartialKeyType>(0b00000000000000000000111000001110),
	  static_cast<PartialKeyType>(0b00000000000000100000000000000010),
	  static_cast<PartialKeyType>(0b00000000000010000000000000001000),
	  static_cast<PartialKeyType>(0b00000000000010010000000000001001),
	  static_cast<PartialKeyType>(0b00000000000011000000000000001100),
	  static_cast<PartialKeyType>(0b00000000000011100000000000001110),
	  static_cast<PartialKeyType>(0b00000010000000100000001000000010),
	  static_cast<PartialKeyType>(0b00001000000010000000100000001000),
	  static_cast<PartialKeyType>(0b00001001000010010000100100001001),
	  static_cast<PartialKeyType>(0b00001100000011000000110000001100),
	  static_cast<PartialKeyType>(0b00001111000000000000111100000000),
	  static_cast<PartialKeyType>(0b00001110000011100000111000001110)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000000000), 0b00000000000000000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000000010), 0b00000000000000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001000), 0b00000000000000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001001), 0b00000000000000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001100), 0b00000000000000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001110), 0b00000000000000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000001000000010), 0b00000000000000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000100000001000), 0b00000000000000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000100100001001), 0b00000000000000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000110000001100), 0b00000000000001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000111100000000), 0b00000000000010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000111000001110), 0b00000000000101011110111u);

	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000100000000000000010), 0b00000000001000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000010000000000000001000), 0b00000000010000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000010010000000000001001), 0b00000000110000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000011000000000000001100), 0b00000001010000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000011100000000000001110), 0b00000011011000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000010000000100000001000000010), 0b00000100001000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00001000000010000000100000001000), 0b00001000010000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00001001000010010000100100001001), 0b00011000110000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00001100000011000000110000001100), 0b00101001010001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00001111000000000000111100000000), 0b01000000000010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00001110000011100000111000001110), 0b10101111011101011110111u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000110000000011), 0b00000000000000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000111111000010), 0b00000000000010001000011u);
	
	
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks23EntriesPermutated = getEntriesMasks<PartialKeyType>({
	  static_cast<PartialKeyType>(0b0000000000000000000000000000000),
	  static_cast<PartialKeyType>(0b0000000000000000000000000000001),
	  static_cast<PartialKeyType>(0b0010000000000000000000000000000),
	  static_cast<PartialKeyType>(0b1010000000000000000000000000000),
	  static_cast<PartialKeyType>(0b0010000000000000000000000000010),
	  static_cast<PartialKeyType>(0b0010000000000000000000000000011),
	  static_cast<PartialKeyType>(0b0000000000000000000000100000001),
	  static_cast<PartialKeyType>(0b0010000000000000000010000000000),
	  static_cast<PartialKeyType>(0b1010000000000000000010010000000),
	  static_cast<PartialKeyType>(0b0010000000000000000011000000010),
	  static_cast<PartialKeyType>(0b0000000000000000000011110000000),
	  static_cast<PartialKeyType>(0b0010000000000000000011100000011),
	  static_cast<PartialKeyType>(0b0000000000000100000000000000001),
	  static_cast<PartialKeyType>(0b0010000000001000000000000000000),
	  static_cast<PartialKeyType>(0b1010000000001001000000000000000),
	  static_cast<PartialKeyType>(0b0010000000001000000000000001010),
	  static_cast<PartialKeyType>(0b0010000000001100000000000001011),
	  static_cast<PartialKeyType>(0b0000001000000100000000100000001),
	  static_cast<PartialKeyType>(0b0010100000001000000010000000000),
	  static_cast<PartialKeyType>(0b1010100100001001000010010000000),
	  static_cast<PartialKeyType>(0b0010110000001000000011000001010),
	  static_cast<PartialKeyType>(0b0000111100000000000011110000000),
	  static_cast<PartialKeyType>(0b0010111000001100000011100001011)
	});

	//entries itself
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000000000000000000000), 0b00000000000000000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000000000000000000001), 0b00000000000000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000000000000000), 0b00000000000000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b1010000000000000000000000000000), 0b00000000000000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000000000000010), 0b00000000000000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000000000000011), 0b00000000000000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000000000000100000001), 0b00000000000000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000010000000000), 0b00000000000000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b1010000000000000000010010000000), 0b00000000000000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000011000000010), 0b00000000000001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000000000011110000000), 0b00000000000010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000000000000011100000011), 0b00000000000101011110111u);

	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000100000000000000001), 0b00000000001000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000001000000000000000000), 0b00000000010000000000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b1010000000001001000000000000000), 0b00000000110000000001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000001000000000000001010), 0b00000001010000000010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010000000001100000000000001011), 0b00000011011000000110111u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000001000000100000000100000001), 0b00000100001000001000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010100000001000000010000000000), 0b00001000010000010000101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b1010100100001001000010010000000), 0b00011000110000110001101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010110000001000000011000001010), 0b00101001010001010010101u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000111100000000000011110000000), 0b01000000000010000000001u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0010111000001100000011100001011), 0b10101111011101011110111u);

	//artificial
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b1000000000000000000011000000001), 0b00000000000000000000011u);
	BOOST_REQUIRE_EQUAL(searchMatchingEntries<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b0000000000000000000011111100001), 0b00000000000010001000011u);
}

template<typename PartialKeyType> void testFindMasksByPatternWith32BitSet() {
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks23Entries = getEntriesMasks<PartialKeyType>({
	  static_cast<PartialKeyType>(0b00000000000000000000000000000000), //0
	  static_cast<PartialKeyType>(0b00000000000000000000000000000010), //1
	  static_cast<PartialKeyType>(0b00000000000000000000000000001000), //2
	  static_cast<PartialKeyType>(0b00000000000000000000000000001001), //3
	  static_cast<PartialKeyType>(0b00000000000000000000000000001100), //4
	  static_cast<PartialKeyType>(0b00000000000000000000000000001110), //5
	  static_cast<PartialKeyType>(0b00000000000000000000001000000010), //6
	  static_cast<PartialKeyType>(0b00000000000000000000100000001000), //7
	  static_cast<PartialKeyType>(0b00000000000000000000100100001001), //8
	  static_cast<PartialKeyType>(0b00000000000000000000110000001100), //9
	  static_cast<PartialKeyType>(0b00000000000000000000111100000000), //10
	  static_cast<PartialKeyType>(0b00000000000000000000111000001110), //11
	  static_cast<PartialKeyType>(0b00000000000000100000000000000010), //12
	  static_cast<PartialKeyType>(0b00000000000010000000000000001000), //13
	  static_cast<PartialKeyType>(0b00000000000010010000000000001001), //14
	  static_cast<PartialKeyType>(0b00000000000011000000000000001100), //15
	  static_cast<PartialKeyType>(0b00000000000011100000000000001110), //16
	  static_cast<PartialKeyType>(0b00000010000000100000001000000010), //17
	  static_cast<PartialKeyType>(0b00001000000010000000100000001000), //18
	  static_cast<PartialKeyType>(0b00001001000010010000100100001001), //19
	  static_cast<PartialKeyType>(0b00001100000011000000110000001100), //20
	  static_cast<PartialKeyType>(0b00001110000011100000111000001110), //21
	  static_cast<PartialKeyType>(0b00001111000000000000111100000000)  //22

	});
	//                                                                                                                   20 17 14 11 8  5  2
    //                                                                                                                 22 19 16 13 10 7  4  1
	//entries itself                                                                                                    21 18 15 12 9  6  3  0
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000000000), 0b11111111111111111111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000000010), 0b01000110001100001100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001000), 0b01111011110101110111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001001), 0b00010000100000100001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001100), 0b01100011000101000110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000000000001110), 0b01000010000100000100000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000001000000010), 0b01000100000100001000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000100000001000), 0b01111000000101110000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000100100001001), 0b00010000000000100000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000110000001100), 0b01100000000101000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000111100000000), 0b10000000000010000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000000000111000001110), 0b01000000000100000000000u);

	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000000100000000000000010), 0b01000110001000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000010000000000000001000), 0b01111011110000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000010010000000000001001), 0b00010000100000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000011000000000000001100), 0b01100011000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000000000011100000000000001110), 0b01000010000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000010000000100000001000000010), 0b01000100000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00001000000010000000100000001000), 0b01111000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00001001000010010000100100001001), 0b00010000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00001100000011000000110000001100), 0b01100000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00001110000011100000111000001110), 0b01000000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00001111000000000000111100000000), 0b10000000000000000000000u);

	//artificial
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23Entries, 23, 0b00000001000000000000010000000000), 0b10000000000000000000000u);

	
	std::shared_ptr<SparsePartialKeys<PartialKeyType>> entriesMasks23EntriesPermutated = getEntriesMasks<PartialKeyType>({
	  static_cast<PartialKeyType>(0b00000000000000000000000000000000),
	  static_cast<PartialKeyType>(0b00000000000000000000000000000010),
	  static_cast<PartialKeyType>(0b10000000000000000000000000000000),
	  static_cast<PartialKeyType>(0b10000000000000000000000000000001),
	  static_cast<PartialKeyType>(0b10000000000000000000000000000100),
	  static_cast<PartialKeyType>(0b10000000000000000000000000000110),
	  static_cast<PartialKeyType>(0b00000000000000000000000100000010),
	  static_cast<PartialKeyType>(0b10000000000000000000010000000000),
	  static_cast<PartialKeyType>(0b10000000100000000000010000000001),
	  static_cast<PartialKeyType>(0b10000000000000000000011000000100),
	  static_cast<PartialKeyType>(0b00000000100000000000011100000000),
	  static_cast<PartialKeyType>(0b10000000000000000000011100000110),
	  static_cast<PartialKeyType>(0b00000000000000000000000000100010),
	  static_cast<PartialKeyType>(0b10000100000000000000000000000000),
	  static_cast<PartialKeyType>(0b10000100000001000000000000000001),
	  static_cast<PartialKeyType>(0b10000100000010000000000000000100),
	  static_cast<PartialKeyType>(0b10000100000010000000000000100110),
	  static_cast<PartialKeyType>(0b00000000000000100000000100100010),
	  static_cast<PartialKeyType>(0b10000110000000000000010000000000),
	  static_cast<PartialKeyType>(0b10000110100001000010010000000001),
	  static_cast<PartialKeyType>(0b10000111000010000000011000000100),
	  static_cast<PartialKeyType>(0b10000111000010100000011100100110),
	  static_cast<PartialKeyType>(0b00000011100000100010011100000000)

	});

	//entries itself
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000000000000000000000000000), 0b11111111111111111111111u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000000000000000000000000010), 0b01000110001100001100010u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000000000000000), 0b01111011110101110111100u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000000000000001), 0b00010000100000100001000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000000000000100), 0b01100011000101000110000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000000000000110), 0b01000010000100000100000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000000000000000000100000010), 0b01000100000100001000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000010000000000), 0b01111000000101110000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000100000000000010000000001), 0b00010000000000100000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000011000000100), 0b01100000000101000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000100000000000011100000000), 0b10000000000010000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000000000000000000011100000110), 0b01000000000100000000000u);

	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000000000000000000000100010), 0b01000110001000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000100000000000000000000000000), 0b01111011110000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000100000001000000000000000001), 0b00010000100000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000100000010000000000000000100), 0b01100011000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000100000010000000000000100110), 0b01000010000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000000000000100000000100100010), 0b01000100000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000110000000000000010000000000), 0b01111000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000110100001000010010000000001), 0b00010000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000111000010000000011000000100), 0b01100000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b10000111000010100000011100100110), 0b01000000000000000000000u);
	BOOST_REQUIRE_EQUAL(findMasksByPattern<PartialKeyType>(entriesMasks23EntriesPermutated, 23, 0b00000011100000100010011100000000), 0b10000000000000000000000u);

}

void testPrintMasksWith32BitSet() {
	std::shared_ptr<SparsePartialKeys<uint32_t>> entriesMasks23Entries = getEntriesMasks<uint32_t>({
	  static_cast<uint32_t>(0b00000000000000000000000000000000), //0
	  static_cast<uint32_t>(0b00000000000000000000000000000010), //1
	  static_cast<uint32_t>(0b00000000000000000000000000001000), //2
	  static_cast<uint32_t>(0b00000000000000000000000000001001), //3
	  static_cast<uint32_t>(0b00000000000000000000000000001100), //4
	  static_cast<uint32_t>(0b00000000000000000000000000001110), //5
	  static_cast<uint32_t>(0b00000000000000000000001000000010), //6
	  static_cast<uint32_t>(0b00000000000000000000100000001000), //7
	  static_cast<uint32_t>(0b00000000000000000000100100001001), //8
	  static_cast<uint32_t>(0b00000000000000000000110000001100), //9
	  static_cast<uint32_t>(0b00000000000000000000111100000000), //10
	  static_cast<uint32_t>(0b00000000000000000000111000001110), //11
	  static_cast<uint32_t>(0b00000000000000100000000000000010), //12
	  static_cast<uint32_t>(0b00000000000010000000000000001000), //13
	  static_cast<uint32_t>(0b00000000000010010000000000001001), //14
	  static_cast<uint32_t>(0b00000000000011000000000000001100), //15
	  static_cast<uint32_t>(0b00000000000011100000000000001110), //16
	  static_cast<uint32_t>(0b00000010000000100000001000000010), //17
	  static_cast<uint32_t>(0b00001000000010000000100000001000), //18
	  static_cast<uint32_t>(0b00001001000010010000100100001001), //19
	  static_cast<uint32_t>(0b00001100000011000000110000001100), //20
	  static_cast<uint32_t>(0b00001110000011100000111000001110), //21
	  static_cast<uint32_t>(0b00001111000000000000111100000000)  //22
	});

	std::ostringstream masksOutput;
	entriesMasks23Entries->printMasks(getMaskForAllUsedEntries(23), masksOutput);

	std::ostringstream expectedStrings;

	expectedStrings << "mask[0] = " <<  "00000000000000000000000000000000" << std::endl;
	expectedStrings << "mask[1] = " <<  "00000000000000000000000000000010" << std::endl;
	expectedStrings << "mask[2] = " <<  "00000000000000000000000000001000" << std::endl;
	expectedStrings << "mask[3] = " <<  "00000000000000000000000000001001" << std::endl;
	expectedStrings << "mask[4] = " <<  "00000000000000000000000000001100" << std::endl;
	expectedStrings << "mask[5] = " <<  "00000000000000000000000000001110" << std::endl;
	expectedStrings << "mask[6] = " <<  "00000000000000000000001000000010" << std::endl;
	expectedStrings << "mask[7] = " <<  "00000000000000000000100000001000" << std::endl;
	expectedStrings << "mask[8] = " <<  "00000000000000000000100100001001" << std::endl;
	expectedStrings << "mask[9] = " <<  "00000000000000000000110000001100" << std::endl;
	expectedStrings << "mask[10] = " << "00000000000000000000111100000000" << std::endl;
	expectedStrings << "mask[11] = " << "00000000000000000000111000001110" << std::endl;
	expectedStrings << "mask[12] = " << "00000000000000100000000000000010" << std::endl;
	expectedStrings << "mask[13] = " << "00000000000010000000000000001000" << std::endl;
	expectedStrings << "mask[14] = " << "00000000000010010000000000001001" << std::endl;
	expectedStrings << "mask[15] = " << "00000000000011000000000000001100" << std::endl;
	expectedStrings << "mask[16] = " << "00000000000011100000000000001110" << std::endl;
	expectedStrings << "mask[17] = " << "00000010000000100000001000000010" << std::endl;
	expectedStrings << "mask[18] = " << "00001000000010000000100000001000" << std::endl;
	expectedStrings << "mask[19] = " << "00001001000010010000100100001001" << std::endl;
	expectedStrings << "mask[20] = " << "00001100000011000000110000001100" << std::endl;
	expectedStrings << "mask[21] = " << "00001110000011100000111000001110" << std::endl;
	expectedStrings << "mask[22] = " << "00001111000000000000111100000000" << std::endl;

	BOOST_REQUIRE_EQUAL(masksOutput.str(), expectedStrings.str());

	std::shared_ptr<SparsePartialKeys<uint32_t>> permutatedEntriesMasks23Entries = getEntriesMasks<uint32_t>({
	  static_cast<uint32_t>(0b00000000000000000000000000000000), //0
	  static_cast<uint32_t>(0b00000000000000000000000000000100), //1
	  static_cast<uint32_t>(0b00000000000000000000000000010000), //2
	  static_cast<uint32_t>(0b00000000000000000000000000010010), //3
	  static_cast<uint32_t>(0b00000000000000000000000000011000), //4
	  static_cast<uint32_t>(0b00000000000000000000000000011100), //5
	  static_cast<uint32_t>(0b00000000000000000000010000000100), //6
	  static_cast<uint32_t>(0b00000000000000000001000000010000), //7
	  static_cast<uint32_t>(0b00000000000000000001001000010010), //8
	  static_cast<uint32_t>(0b00000000000000000001100000011000), //9
	  static_cast<uint32_t>(0b00000000000000000001111000000000), //10
	  static_cast<uint32_t>(0b00000000000000000001110000011100), //11
	  static_cast<uint32_t>(0b00000000000001000000000000000100), //12
	  static_cast<uint32_t>(0b00000000000100000000000000010000), //13
	  static_cast<uint32_t>(0b00000000000100100000000000010010), //14
	  static_cast<uint32_t>(0b00000000000110000000000000011000), //15
	  static_cast<uint32_t>(0b00000000000111000000000000011100), //16
	  static_cast<uint32_t>(0b00000100000001000000010000000100), //17
	  static_cast<uint32_t>(0b00010000000100000001000000010000), //18
	  static_cast<uint32_t>(0b00010010000100100001001000010010), //19
	  static_cast<uint32_t>(0b00011000000110000001100000011000), //20
	  static_cast<uint32_t>(0b00011100000111000001110000011100), //21
	  static_cast<uint32_t>(0b00011110000000000001111000000000)  //22
	});


	//the mapping is from target bit -> source Bit
	//be aware the source bit index is little endian and the target bit index is big endian
	std::map<uint16_t, uint16_t> masksBitMappings {
		{ 0,   0  },
		{ 1,   31 },
		{ 2,   30 },
		{ 3,   29 },
		{ 4,   28 },
		{ 5,   27 },
		{ 6,   26 },
		{ 7,   25 },
		{ 8,   24 },
		{ 9,   23 },
		{ 10,  22 },
		{ 11,  21 },
		{ 12,  20 },
		{ 13,  19 },
		{ 14,  18 },
		{ 15,  17 },
		{ 16,  16 },
		{ 17,  15 },
		{ 18,  14 },
		{ 19,  13 },
		{ 20,  12 },
		{ 21,  11 },
		{ 22,  10 },
		{ 23,   9 },
		{ 24,   8 },
		{ 25,   7 },
		{ 26,   6 },
		{ 27,   5 },
		{ 28,   4 },
		{ 29,   3 },
		{ 30,   2 },
		{ 31,   1 }
	};

	std::ostringstream expectedPermutatedString;

	expectedPermutatedString << "mask[0] = \toriginal: "  << "00000000000000000000000000000000\tmapped: 00000000000000000000000000000000" << std::endl;
	expectedPermutatedString << "mask[1] = \toriginal: "  << "00000000000000000000000000000100\tmapped: 00000000000000000000000000000010" << std::endl;
	expectedPermutatedString << "mask[2] = \toriginal: "  << "00000000000000000000000000010000\tmapped: 00000000000000000000000000001000" << std::endl;
	expectedPermutatedString << "mask[3] = \toriginal: "  << "00000000000000000000000000010010\tmapped: 00000000000000000000000000001001" << std::endl;
	expectedPermutatedString << "mask[4] = \toriginal: "  << "00000000000000000000000000011000\tmapped: 00000000000000000000000000001100" << std::endl;
	expectedPermutatedString << "mask[5] = \toriginal: "  << "00000000000000000000000000011100\tmapped: 00000000000000000000000000001110" << std::endl;
	expectedPermutatedString << "mask[6] = \toriginal: "  << "00000000000000000000010000000100\tmapped: 00000000000000000000001000000010" << std::endl;
	expectedPermutatedString << "mask[7] = \toriginal: "  << "00000000000000000001000000010000\tmapped: 00000000000000000000100000001000" << std::endl;
	expectedPermutatedString << "mask[8] = \toriginal: "  << "00000000000000000001001000010010\tmapped: 00000000000000000000100100001001" << std::endl;
	expectedPermutatedString << "mask[9] = \toriginal: "  << "00000000000000000001100000011000\tmapped: 00000000000000000000110000001100" << std::endl;
	expectedPermutatedString << "mask[10] = \toriginal: " << "00000000000000000001111000000000\tmapped: 00000000000000000000111100000000" << std::endl;
	expectedPermutatedString << "mask[11] = \toriginal: " << "00000000000000000001110000011100\tmapped: 00000000000000000000111000001110" << std::endl;
	expectedPermutatedString << "mask[12] = \toriginal: " << "00000000000001000000000000000100\tmapped: 00000000000000100000000000000010" << std::endl;
	expectedPermutatedString << "mask[13] = \toriginal: " << "00000000000100000000000000010000\tmapped: 00000000000010000000000000001000" << std::endl;
	expectedPermutatedString << "mask[14] = \toriginal: " << "00000000000100100000000000010010\tmapped: 00000000000010010000000000001001" << std::endl;
	expectedPermutatedString << "mask[15] = \toriginal: " << "00000000000110000000000000011000\tmapped: 00000000000011000000000000001100" << std::endl;
	expectedPermutatedString << "mask[16] = \toriginal: " << "00000000000111000000000000011100\tmapped: 00000000000011100000000000001110" << std::endl;
	expectedPermutatedString << "mask[17] = \toriginal: " << "00000100000001000000010000000100\tmapped: 00000010000000100000001000000010" << std::endl;
	expectedPermutatedString << "mask[18] = \toriginal: " << "00010000000100000001000000010000\tmapped: 00001000000010000000100000001000" << std::endl;
	expectedPermutatedString << "mask[19] = \toriginal: " << "00010010000100100001001000010010\tmapped: 00001001000010010000100100001001" << std::endl;
	expectedPermutatedString << "mask[20] = \toriginal: " << "00011000000110000001100000011000\tmapped: 00001100000011000000110000001100" << std::endl;
	expectedPermutatedString << "mask[21] = \toriginal: " << "00011100000111000001110000011100\tmapped: 00001110000011100000111000001110" << std::endl;
	expectedPermutatedString << "mask[22] = \toriginal: " << "00011110000000000001111000000000\tmapped: 00001111000000000000111100000000" << std::endl;

	std::ostringstream permutatedMasks;
	permutatedEntriesMasks23Entries->printMasks(getMaskForAllUsedEntries(23), masksBitMappings, permutatedMasks);

	BOOST_REQUIRE_EQUAL(permutatedMasks.str(), expectedPermutatedString.str());
}



BOOST_AUTO_TEST_CASE(testSearchMasks8) {
	testMaskWith8BitSet<uint8_t>();
}


BOOST_AUTO_TEST_CASE(testFindMasksByPattern8) {
	testFindMasksByPatternWith8BitSet<uint8_t>();
}

BOOST_AUTO_TEST_CASE(testGetAffectedSubtreeMasks8) {
	testGetAffectedSubtreeMaskWith8BitSet<uint8_t>();
}

BOOST_AUTO_TEST_CASE(testSearchMasks16) {
	testMaskWith8BitSet<uint16_t>();
	testMaskWith16BitSet<uint16_t>();
}

BOOST_AUTO_TEST_CASE(testFindMasksByPattern16) {
	testFindMasksByPatternWith8BitSet<uint16_t>();
	testFindMasksByPatternWith16BitSet<uint16_t>();
}

BOOST_AUTO_TEST_CASE(testGetAffectedSubtreeMasks16) {
	testGetAffectedSubtreeMaskWith8BitSet<uint16_t>();
	testGetAffectedSubtreeMaskWith16BitSet<uint16_t>();
}

BOOST_AUTO_TEST_CASE(testSearchMasks32) {
	testMaskWith8BitSet<uint32_t>();
	testMaskWith16BitSet<uint32_t>();
	testMaskWith32BitSet<uint32_t>();
}

BOOST_AUTO_TEST_CASE(testFindMasksByPattern32) {
	testFindMasksByPatternWith8BitSet<uint32_t>();
	testFindMasksByPatternWith16BitSet<uint32_t>();
	testFindMasksByPatternWith32BitSet<uint32_t>();
}

BOOST_AUTO_TEST_CASE(testGetAffectedSubtreeMasks32) {
	testGetAffectedSubtreeMaskWith8BitSet<uint32_t>();
	testGetAffectedSubtreeMaskWith16BitSet<uint32_t>();
	testGetAffectedSubtreeMaskWith32BitSet<uint32_t>();
}

BOOST_AUTO_TEST_CASE(testPrintMasks8) {
	testPrintMasksWith8BitSet();
}


BOOST_AUTO_TEST_CASE(testPrintMasks16) {
	testPrintMasksWith16BitSet();
}


BOOST_AUTO_TEST_CASE(testPrintMasks32) {
	testPrintMasksWith32BitSet();
}

BOOST_AUTO_TEST_SUITE_END()

}}