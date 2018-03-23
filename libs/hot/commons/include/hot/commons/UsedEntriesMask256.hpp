#ifndef __HOT__COMMONS__USED_ENTRIES_MASK_256__
#define __HOT__COMMONS__USED_ENTRIES_MASK_256__

#include "hot/commons/Algorithms.hpp"
#include "hot/commons/SIMDHelper.hpp"

namespace hot { namespace commons {

constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_ONES_IN_BYTE = 32;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_ZEROS_IN_BYTE = 32;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_SUB_MASKS_IN_BYTE = 8;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_TRAILING_0_BYTES = 32;

constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__1s_OFFSET = 0;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__0s_OFFSET = HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__1s_OFFSET + HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_ONES_IN_BYTE;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__SUB_MASKS_OFFSET = HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__0s_OFFSET + HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_ZEROS_IN_BYTE;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TRAILING_0s_OFFSET = HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__SUB_MASKS_OFFSET + HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_SUB_MASKS_IN_BYTE;
constexpr uint32_t HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TOTAL_SIZE_IN_BYTES = HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TRAILING_0s_OFFSET + HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_TRAILING_0_BYTES;

std::array<uint8_t, HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TOTAL_SIZE_IN_BYTES> inline createHelperArrayToConvertNumberEntriesToEntriesMask() {
	std::array<uint8_t, HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TOTAL_SIZE_IN_BYTES> helperArray;
	for(size_t i=HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__1s_OFFSET; i < HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__0s_OFFSET; ++i) {
		helperArray[i] = 0xFFu;
	}
	for(size_t i=HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__0s_OFFSET; i < HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__SUB_MASKS_OFFSET; ++i) {
		helperArray[i] = 0u;
	}
	for(size_t i=0; i < HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__NUMBER_SUB_MASKS_IN_BYTE; ++i) {
		helperArray[HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__SUB_MASKS_OFFSET + i] = static_cast<uint8_t>(0xFFu >> (8 - i));
	}
	helperArray[HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__SUB_MASKS_OFFSET] = 0xFFu;
	for(size_t i=HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TRAILING_0s_OFFSET; i < HELPER_ARRAY_FOR_CONVERTING_NUMBER_ENTRIES__TOTAL_SIZE_IN_BYTES; ++i) {
		helperArray[i] = 0u;
	}
	return helperArray;
};

class UsedEntriesMask256 {

public:
		static inline  __attribute__((always_inline,  __artificial__)) __m256i createForNumberEntries(uint32_t numberEntries) {
					assert(numberEntries <= 256);
				assert(numberEntries > 0);

				uint32_t chunkPosition = (numberEntries - 1) / 64;
				uint64_t remainingPart = UINT64_MAX >> (64 - (numberEntries % 64));

			__m256i remainingPartVector = _mm256_set_epi64x(remainingPart, remainingPart, remainingPart, remainingPart);
			__m256i chunkPositionVector = _mm256_set_epi64x(chunkPosition, chunkPosition, chunkPosition, chunkPosition);
			__m256i chunkPositionOffsets = _mm256_set_epi64x(3, 2, 1, 0);

				return SIMDHelper<256>::binaryOr(
					_mm256_cmpgt_epi64(chunkPositionVector, chunkPositionOffsets),
					SIMDHelper<256>::binaryAnd(_mm256_cmpeq_epi64(chunkPositionOffsets, chunkPositionVector), remainingPartVector)
				);
		}

private:
		static inline __attribute__((always_inline)) uint32_t getLeastSignificantUsedBitPositionInMask(uint32_t mask) {
			assert(mask != 0);
			return __builtin_ctz(mask);
		}

		static inline  __attribute__((always_inline)) uint32_t getMostSignificantUsedBitPositionInMask(uint32_t mask) {
			assert(mask != 0);
			return static_cast<uint32_t>(getMostSignificantBitIndex(mask));
		}

public:
		static inline  __attribute__((always_inline)) uint32_t getUsedBytesMask(__m256i usedEntries) {
			uint32_t unusedBytesMask = SIMDHelper<256>::moveMask8(SIMDHelper<256>::cmpeq_epi8(SIMDHelper<256>::zero(), usedEntries));
			return static_cast<uint32_t>(~unusedBytesMask);
		}

		static inline uint32_t getFirstUsedEntry(__m256i usedEntries) {
			std::array<uint8_t, 32> targetArray;
			SIMDHelper<256>::store(usedEntries, targetArray.data());
			uint32_t usedBytes = getUsedBytesMask(usedEntries);
			uint32_t firstByteIndex = getLeastSignificantUsedBitPositionInMask(usedBytes);

			uint32_t firstByteRelativeIndex = getLeastSignificantUsedBitPositionInMask(targetArray[firstByteIndex]);
			return (firstByteIndex * 8) + firstByteRelativeIndex;
		}

		static inline  __attribute__((always_inline)) uint32_t getLastUsedEntry(__m256i usedEntries) {
			std::array<uint8_t, 32> targetArray;
			SIMDHelper<256>::store(usedEntries, targetArray.data());
			uint32_t usedBytes = getUsedBytesMask(usedEntries);
			uint32_t lastByteIndex = getMostSignificantUsedBitPositionInMask(usedBytes);
			uint32_t lastByteRelativeIndex = getMostSignificantUsedBitPositionInMask(targetArray[lastByteIndex]);
			return (lastByteIndex * 8) + lastByteRelativeIndex;
		}

		static inline __m256i createSingleEntryMask(uint32_t bitIndex) {
			// constants that will (hopefully) be hoisted out of a loop after inlining
			__m256i indices = _mm256_set_epi32(224,192,160,128,96,64,32,0);

			__m256i one = _mm256_set1_epi32(-1);
			one = _mm256_srli_epi32(one, 31);    // set1(0x1)

			__m256i kvec = _mm256_set1_epi32(bitIndex);
			// if 0<=k<=255 then kvec-indices has exactly one element with a value between 0 and 31
			__m256i shiftcounts = _mm256_sub_epi32(kvec, indices);
			return  _mm256_sllv_epi32(one, shiftcounts);   // shift counts outside 0..31 shift the bit out of the element
		}

		static inline __m256i setUsedEntry(__m256i usedEntries, uint32_t bitIndex) {
			return SIMDHelper<256>::binaryOr(usedEntries, createSingleEntryMask(bitIndex));
		}

		static inline __m256i removeUsedEntry(__m256i usedEntries, uint32_t bitIndex) {
			return SIMDHelper<256>::binaryAndNot(createSingleEntryMask(bitIndex), usedEntries);
		}

		static inline uint32_t getNumberUsedEntries(__m256i usedEntries) {
			return _mm_popcnt_u64(_mm256_extract_epi64(usedEntries, 0))
				+ _mm_popcnt_u64(_mm256_extract_epi64(usedEntries, 1))
				+ _mm_popcnt_u64(_mm256_extract_epi64(usedEntries, 2))
			  + _mm_popcnt_u64(_mm256_extract_epi64(usedEntries, 3));
		}

		static inline bool isEmpty(__m256i usedEntries) {
			return static_cast<uint32_t>(_mm256_movemask_epi8(SIMDHelper<256>::cmpeq_epi8(usedEntries, SIMDHelper<256>::zero()))) == UINT32_MAX;
		}
};

}}

#endif