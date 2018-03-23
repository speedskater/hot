#ifndef __HOT__COMMONS__SPARSE_PARTIAL_KEYS_256__
#define __HOT__COMMONS__SPARSE_PARTIAL_KEYS_256__

#include <immintrin.h>

#include <bitset>
#include <cassert>
#include <cstdint>
#include <map>

#include <hot/commons/Algorithms.hpp>
#include <hot/commons/SIMDHelper.hpp>
#include <hot/commons/UsedEntriesMask256.hpp>

namespace hot { namespace commons {

constexpr uint16_t alignToNextHighestValueDivisableBy8(uint16_t size) {
	return (size % 8 == 0) ? size : ((size & (~7)) + 8);
}

/**
 * A tuple of entry masks represent a tuple of binary masks representing compressed paths of binary trees. based on solely the
 * the compressed masks the original tree cannot be reconstructed.
 *
 * To grasp the idea of compressed paths consider the folowing three level-uncompressed binary tree.
 *
 *  level1                  l    R    r
 *	                        +----+----+
 *                          |         |
 *  level2    		    l   n      l  n   r
 * 	                    +---+      +--+---+
 * 	                    |          |      |
 *  level3            l n r        n r  l n
 * 	                  +-+-+        +-+  +-+
 * 	                  |   |          |  |
 * 	                  |   |          |  |
 *  leafMask         lll llr        rlr rrl
 *
 * Each path (corresponding to a leaf node) of this binary tree can be represented by a string of characters with each character representing
 * a path part. Paths can be described by a string of characters l and r denoting the branching decisions (left or right) taken at the nodes along the path
 *
 * Assume we want to ommit inner nodes having only a single child and therefore we mark them with U.
 * The subsquent tree represents the previous tree with inner nodes with single child replaced with U.
 *
 *  level1                  l    R    r
 *	                        +----+----+
 *                          |         |
 *  level2    		        U       l n r
 * 	                        |       +-+-+
 * 	                        |       |   |
 * 	level3                l n r     U   U
 * 	                      +-+-+     |   |
 * 	                      |   |     |   |
 *  leafMask             lUl lUr   rlU rrU
 *
 * By replacing inner nodes with U we loose the information how to reconstruct the original path in the uncompressed tree.
 * Furthermore introducing the letter U extends our binary alphabet consisting of l and r with U resulting in a three letter alphabet.
 *
 * To still be able to describe paths with a two character alphabet we interpret left branches (l) as well as ommited nodes (U) as 0 and right branches as 1
 *
 *  For the path strings of the previous example, the resulting binary masks will be mapped in the follwing way:
 *
 *  leafMask          compressed mask
 *  lUl               000
 *  lUr               001
 *  rlU               100
 *  rrU               110
 *
 *  This results in masks with 0 having a twofold interpretation. On the one hand side 0 represents a left branch on the other hand
 *  side 0 represents an ommited node and hence no information is available whether it is L or R.
 *
 *  Although the interpretation of the binary representation is not unique anymore, the encoding is still order preserving.
 *  (Nach korrektur Eva: das muß ausgebaut werden, da argumentativ nicht verständlich ist, warum dies so ist, eigentlich dieser ganze Absatz)
 *  Masks of a tuple of leaf entries X which are left of a tuple of leaf entries Y will result in strictly smaller binary representations
 *  regardless of the value of the ommited nodes (if any).
 *  The reason is that all entries x and y originate from a common parent node, with entries x all having 0 at the bit
 *  corresponding to this parent node and entries y all having 1 at the bit corresponding to this parent node.
 *
 *  Because parent nodes correspond to more significant bits than child nodes, the binary representation
 *  of each leaf entry in X is strictly smaller than of each entry in Y.
 *
 *  To search whether a given path of the uncompressed tree is contained, the path has to be converted into a binary search mask and compared to the stored masks.
 *  Based on the ambiguous nature of 0s in the compressed masks only the 1s occuring in the compressed masks may be considered.
 *  Therefore the bits used for the comparison solely depend on the bits set in the compressed mask.
 *
 *  By definition, a compressed path matches an uncompressed mask if all bits which are one occur in the uncompressed mask as well.
 *  Executing this comparison for all masks stored in an entries masks tuple may yield multiple results.
 *  To determine which compressed mask has the highest probability of being the matching one, the largest compressed matching mask is chosen.
 *
 *  The reason is that larger masks share more significant bits with the uncompressed mask and hence can falsify the smaller compressed masks.
 *  To illustrate this circumstance, consider the compressed masks 011 and 100 and the uncompressed search mask 111. By definition both compressed masks comply
 *  to the uncompressed search mask 111. In case of the compressed mask 011, the first bit might represent an ommited node in case of the compressed
 *  mask 100 bits 2 and 3 may represent omitted nodes.
 *  In case of the second mask it can be determined that the first bit is set, which implies that the first bit in the first
 *  compressed mask must be interpreted as 0 and hence the first compressed mask can be removed from the set of matching compressed masks.
 *
 *  Although a single match can always be determined (in case no other compressed mask matches the most left mask trivially matches because it is always 0)
 *  it might be a false positive.
 *
 *	Furthermore as all operations executed on compressed masks solely depend on a unique labeling of the bits. The compressed masks
 *	might be stored with permutated bits.
 *	The only requirement is that all compressed masks occuring in an entries masks tuple where permutated using the same permutation.
 *	To execute operations like search on such permutated compressed paths the search path must be permutated according to the same permutation.
 *
 */
template<typename PartialKeyType>
struct alignas(8) SparsePartialKeys256 {
	static constexpr size_t PART_SIZE = 32;

	void *operator new(size_t baseSize, uint16_t const numberEntries);

	void operator delete(void *);

	PartialKeyType mEntries[1];

	/**
	 * Search returns the masks of all compressed masks complying to the search Mask. A compressed mask complies to the search mask
	 * if all bits set in the compressed mask occur in the search Mask.
	 *
	 * If the stored masks are permuted the searchMask must have been permutated in the same way.
	 * Otherwise no meaningful results can be generated.
	 *
	 * the operation executed for each compressed mask is: haystack & needle == haystack
	 *
	 * To extract the mask with the highest propability of matching the search mask determine the position of the most significant bit set in the result mask.
	 *
	 * @param searchMask the uncompressed mask representing a path in a binary tree
	 * @return the resulting mask with each bit representing the result of a single compressed mask. bit 0 (least significant) correspond to the mask 0, bit 1 corresponds to mask 1 and so forth.
	 */
	inline  __attribute__((always_inline)) __m256i search(PartialKeyType const uncompressedSearchMask, size_t totalNumberEntries) const {
		__m256i searchMask = SIMDHelper<256>::set1(uncompressedSearchMask);
		uint32_t numberParts = ((totalNumberEntries - 1)/PART_SIZE) + 1;

		switch(numberParts)  {
			case 1:
				return _mm256_set_epi32(
					0, 0, 0, 0, 0, 0, 0,
					partSearch(searchMask, mEntries)
				);
			case 2:
				return _mm256_set_epi32(
					0, 0, 0, 0, 0, 0,
					partSearch(searchMask, mEntries + PART_SIZE),
					partSearch(searchMask, mEntries)
				);
			case 3:
				return _mm256_set_epi32(
					0, 0, 0, 0, 0,
					partSearch(searchMask, mEntries + (PART_SIZE * 2)),
					partSearch(searchMask, mEntries + PART_SIZE),
					partSearch(searchMask, mEntries)
				);
			case 4:
				return _mm256_set_epi32(
					0, 0, 0, 0,
					partSearch(searchMask, mEntries + (PART_SIZE * 3)),
					partSearch(searchMask, mEntries + (PART_SIZE * 2)),
					partSearch(searchMask, mEntries + PART_SIZE),
					partSearch(searchMask, mEntries)
				);
			default:
				return _mm256_set_epi32(
					partSearch(searchMask, mEntries + (PART_SIZE * 7)),
					partSearch(searchMask, mEntries + (PART_SIZE * 6)),
					partSearch(searchMask, mEntries + (PART_SIZE * 5)),
					partSearch(searchMask, mEntries + (PART_SIZE * 4)),
					partSearch(searchMask, mEntries + (PART_SIZE * 3)),
					partSearch(searchMask, mEntries + (PART_SIZE * 2)),
					partSearch(searchMask, mEntries + PART_SIZE),
					partSearch(searchMask, mEntries)
				);
		}

		/*for(size_t i=0; i < numberParts; ++i) {
			parts[i] = partSearch(searchMask, mEntries + (PART_SIZE * i));
		}
		return SIMDHelper<256>::toRegister(parts.data());*/
	}

	inline uint32_t search32(PartialKeyType const uncompressedSearchMask) const {
		return partSearch(SIMDHelper<256>::set1(uncompressedSearchMask), mEntries);
	}

private:
		inline  __attribute__((always_inline)) uint32_t partSearch(__m256i searchRegister, PartialKeyType const * partStart) const;

public:

	/**
	 * Searches all compressed masks stored in this mask tuple whether they match a given pattern.
	 * A compressed mask matches a given pattern if all bits set to one in the pattern are also set to one in the compressed pattern.
	 *
	 * This method exectues for each compressed mask (compressedMask[i]): compressedMask[i] & searchPattern == searchPattern
	 *
	 * @param searchPattern the pattern to use for searchin compressed mask
	 * @return the resulting mask with each bit representing the result of a single compressed mask. bit 0 (least significant) correspond to the mask 0, bit 1 corresponds to mask 1 and so forth.
	 */
	inline __m256i findMasksByPattern(PartialKeyType const searchPattern, size_t totalNumberEntries) const {
		__m256i searchRegister = SIMDHelper<256>::set1(searchPattern);
		uint32_t numberParts = ((totalNumberEntries - 1)/PART_SIZE) + 1;
		std::array<uint32_t, 8> parts;
		for(size_t i=0; i < numberParts; ++i) {
			parts[i] = partFindMasksByPattern(searchRegister, searchRegister, mEntries + (PART_SIZE * i));
		}
		return SIMDHelper<256>::toRegister(parts.data());
	}

private:
	inline uint32_t partFindMasksByPattern(__m256i const usedBitsMask, __m256i const expectedBitsMask, PartialKeyType const * rangeStart) const;

public:
	//compressedMask & consideredPrefixBits == Prefix
	inline __m256i getAffectedSubtreeMask(PartialKeyType usedPrefixBits, PartialKeyType const expectedPrefixBits, size_t totalNumberEntries) const {
		//assumed this should be the same as
		//affectedMaskBitsRegister = SIMDHelper<256>::set1(affectedBitsMask)
		//expectedMaskBitsRegister = SIMDHelper<256>::set1(expectedMaskBits)
		//return findMasksByPattern(affectedMaskBitsRegister, expectedMaskBitsRegister) & usedEntriesMask;

		__m256i prefixBITSSIMDMask = SIMDHelper<256>::set1(usedPrefixBits);
		__m256i prefixMask = SIMDHelper<256>::set1(expectedPrefixBits);

		uint32_t numberParts = ((totalNumberEntries - 1)/PART_SIZE) + 1;
		std::array<uint32_t, 8> parts;
		for(size_t i=0; i < numberParts; ++i) {
			parts[i] =  partFindMasksByPattern(prefixBITSSIMDMask, prefixMask, mEntries + (PART_SIZE * i));
		}
		return SIMDHelper<256>::toRegister(parts.data());
	}

	/**
	 *
	 * in the case of the following tree structure:
	 *               d
	 *            /    \
	 *           b      f
	 *          / \    / \
	 *         a  c   e  g
	 * Index   0  1   2  3
	 *
	 * If the provided index is 2 corresponding the smallest common subtree containing e consists of the nodes { e, f, g }
	 * and therefore the discriminative bit value for this entry is 0 (in the left side of the subtree).
	 *
	 * If the provided index is 1 corresponding to entry c, the smallest common subtree containing c consists of the nodes { a, b, c }
	 * and therefore the discriminative bit value of this entries 1 (in the right side of the subtree).
	 *
	 * in the case of the following tree structure
	 *                    f
	 *                 /    \
	 *                d      g
	 *              /   \
	 *             b     e
	 *            / \
	 *           a   c
	 * Index     0   1   2   3
	 *
	 * If the provided index is 2 correspondig to entry e, the smallest common subtree containing e consists of the nodes { a, b, c, d, e }
	 * As e is on the right side of this subtree, the discriminative bit's value in this case is 1
	 *
	 *
	 * @param indexOfEntry The index of the entry to obtain the discriminative bits value for
	 * @return The discriminative bit value of the discriminative bit discriminating an entry from its direct neighbour (regardless if the direct neighbour is an inner or a leaf node).
	 */
	inline bool determineValueOfDiscriminatingBit(size_t indexOfEntry, size_t mNumberEntries) const {
		bool discriminativeBitValue;

		if(indexOfEntry == 0) {
			discriminativeBitValue = false;
		} else if(indexOfEntry == (mNumberEntries - 1)) {
			discriminativeBitValue = true;
		} else {
			//Be aware that the masks are not order preserving, as the bits may not be in the correct order little vs. big endian and several included bytes
			discriminativeBitValue = (mEntries[indexOfEntry - 1]&mEntries[indexOfEntry]) >= (mEntries[indexOfEntry]&mEntries[indexOfEntry + 1]);
		}
		return discriminativeBitValue;
	}

	/**
	 * Get Relevant bits detects the key bits used for discriminating new entries in the given range.
	 * These bits are determined by comparing successing masks in this range.
	 * Whenever a mask has a bit set which is not set in its predecessor these bit is added to the set of relevant bits.
	 * The reason is that if masks are stored in an orderpreserving way for a mask to be large than its predecessor it has to set
	 * exactly one more bit.
	 * By using this algorithm the bits of the first mask occuring in the range of masks are always ignored.
	 *
	 * @param firstIndexInRange the first index of the range of entries to determine the relevant bits for
	 * @param numberEntriesInRange the number entries in the range of entries to use for determining the relevant bits
	 * @return a mask with only the relevant bits set.
	 */
	inline PartialKeyType getRelevantBitsForRange(uint32_t const firstIndexInRange, uint32_t const numberEntriesInRange) const {
		PartialKeyType relevantBits = 0;

		uint32_t firstIndexOutOfRange = firstIndexInRange + numberEntriesInRange;
		for(uint32_t i = firstIndexInRange + 1; i < firstIndexOutOfRange; ++i) {
			relevantBits |= (mEntries[i] & ~mEntries[i - 1]);
		}
		return relevantBits;
	}

	inline PartialKeyType getRelevantBitsForAllExceptOneEntry(uint32_t const numberEntries, uint32_t indexOfEntryToIgnore) const {
		size_t numberEntriesInFirstRange = indexOfEntryToIgnore + static_cast<size_t>(!determineValueOfDiscriminatingBit(indexOfEntryToIgnore, numberEntries));

		PartialKeyType relevantBitsInFirstPart = getRelevantBitsForRange(0, numberEntriesInFirstRange);
		PartialKeyType relevantBitsInSecondPart = getRelevantBitsForRange(numberEntriesInFirstRange, numberEntries - numberEntriesInFirstRange);
		return relevantBitsInFirstPart | relevantBitsInSecondPart;
	}

	static inline uint16_t estimateSize(uint16_t numberEntries) {
		return alignToNextHighestValueDivisableBy8(numberEntries * sizeof(PartialKeyType));
	}

	/**
	 * @param maskBitMapping maps from the absoluteBitPosition to its maskPosition
	 */
	inline void printMasks(__m256i maskOfEntriesToPrint, std::map<uint16_t, uint16_t> const & maskBitMapping, std::ostream & outputStream = std::cout) const {
		while(!UsedEntriesMask256::isEmpty(maskOfEntriesToPrint)) {
			uint32_t entryIndex = UsedEntriesMask256::getFirstUsedEntry(maskOfEntriesToPrint);
			std::bitset<sizeof(PartialKeyType) * 8> maskBits(mEntries[entryIndex]);
			outputStream << "mask[" << entryIndex << "] = \toriginal: " << maskBits << "\tmapped: ";
			printMaskWithMapping(mEntries[entryIndex], maskBitMapping, outputStream);
			outputStream << std::endl;
			maskOfEntriesToPrint = UsedEntriesMask256::removeUsedEntry(maskOfEntriesToPrint, entryIndex);
		}
	}

	static inline void printMaskWithMapping(PartialKeyType mask, std::map<uint16_t, uint16_t> const & maskBitMapping, std::ostream & outputStream) {
		std::bitset<convertBytesToBits(sizeof(PartialKeyType))> maskBits(mask);
		for(auto mapEntry : maskBitMapping) {
			uint64_t maskBitIndex = mapEntry.second;
			outputStream << maskBits[maskBitIndex];
		}
	}


	inline void printMasks(__m256i maskOfEntriesToPrint, std::ostream & outputStream = std::cout) const {
		while(!UsedEntriesMask256::isEmpty(maskOfEntriesToPrint)) {
			size_t entryIndex = UsedEntriesMask256::getFirstUsedEntry(maskOfEntriesToPrint);

			std::bitset<sizeof(PartialKeyType) * 8> maskBits(mEntries[entryIndex]);
			outputStream << "mask[" << entryIndex << "] = " << maskBits << std::endl;

			maskOfEntriesToPrint = UsedEntriesMask256::removeUsedEntry(maskOfEntriesToPrint, entryIndex);
		}
	}

private:
	// Prevent heap allocation
	void * operator new   (size_t);
	void * operator new[] (size_t);
	void operator delete[] (void*);
};

template<typename PartialKeyType> void* SparsePartialKeys256<PartialKeyType>::operator new (size_t baseSize, uint16_t const numberEntries) {
	assert(numberEntries >= 2);

	constexpr size_t paddingElements = (32 - 8)/sizeof(PartialKeyType);
	size_t estimatedNumberElements = estimateSize(numberEntries)/sizeof(PartialKeyType);


	return new PartialKeyType[estimatedNumberElements + paddingElements];
};
template<typename PartialKeyType> void SparsePartialKeys256<PartialKeyType>::operator delete (void * rawMemory) {
	PartialKeyType* masks = reinterpret_cast<PartialKeyType*>(rawMemory);
	delete [] masks;
}

template<>
inline __attribute__((always_inline)) uint32_t SparsePartialKeys256<uint8_t>::partSearch(__m256i searchRegister, uint8_t const * partStart) const {
	__m256i haystack = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart)); //3 instr
	__m256i searchResult = _mm256_cmpeq_epi8(_mm256_and_si256(haystack, searchRegister), haystack);
	uint32_t const resultMask = static_cast<uint32_t>(_mm256_movemask_epi8(searchResult));
	return resultMask;
}

template<>
inline __attribute__((always_inline)) uint32_t SparsePartialKeys256<uint16_t>::partSearch(__m256i searchRegister, uint16_t const * partStart) const {
	__m256i haystack1 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart)); //3 instr
	__m256i haystack2 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart + 16)); //4 instr

	__m256i const perm_mask = _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0); //35 instr

	__m256i searchResult1 = _mm256_cmpeq_epi16(_mm256_and_si256(haystack1, searchRegister), haystack1);
	__m256i searchResult2 = _mm256_cmpeq_epi16(_mm256_and_si256(haystack2, searchRegister), haystack2);

	__m256i intermediateResult = _mm256_permutevar8x32_epi32(_mm256_packs_epi16( //43 + 6 = 49
		searchResult1, searchResult2
	), perm_mask);

	return static_cast<uint32_t>(_mm256_movemask_epi8(intermediateResult));
}

template<>
inline __attribute__((always_inline)) uint32_t SparsePartialKeys256<uint32_t>::partSearch(__m256i searchRegister, uint32_t const * partStart) const {
	__m256i haystack1 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart));
	__m256i haystack2 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart + 8));
	__m256i haystack3 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart + 16));
	__m256i haystack4 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(partStart + 24)); //27 instr

	__m256i const perm_mask = _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0); //35 instr

	__m256i searchResult1 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack1, searchRegister), haystack1);
	__m256i searchResult2 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack2, searchRegister), haystack2);
	__m256i searchResult3 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack3, searchRegister), haystack3);
	__m256i searchResult4 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack4, searchRegister), haystack4); //35 + 8 = 43

	__m256i intermediateResult = _mm256_permutevar8x32_epi32(_mm256_packs_epi16( //43 + 6 = 49
		_mm256_permutevar8x32_epi32(_mm256_packs_epi32(searchResult1, searchResult2), perm_mask),
		_mm256_permutevar8x32_epi32(_mm256_packs_epi32(searchResult3, searchResult4), perm_mask)
	), perm_mask);

	return static_cast<uint32_t>(_mm256_movemask_epi8(intermediateResult));
}

template<>
inline uint32_t SparsePartialKeys256<uint8_t>::partFindMasksByPattern(__m256i consideredBitsRegister, __m256i expectedBitsRegister, uint8_t const * rangeStart) const {
	__m256i haystack = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart)); //3 instr
	__m256i searchResult = _mm256_cmpeq_epi8(_mm256_and_si256(haystack, consideredBitsRegister), expectedBitsRegister);
	return static_cast<uint32_t>(_mm256_movemask_epi8(searchResult));
}

template<>
inline uint32_t SparsePartialKeys256<uint16_t>::partFindMasksByPattern(__m256i consideredBitsRegister, __m256i expectedBitsRegister, uint16_t const * rangeStart) const {
	__m256i haystack1 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart)); //3 instr
	__m256i haystack2 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart + 16)); //4 instr

	__m256i const perm_mask = _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0); //35 instr

	__m256i searchResult1 = _mm256_cmpeq_epi16(_mm256_and_si256(haystack1, consideredBitsRegister), expectedBitsRegister);
	__m256i searchResult2 = _mm256_cmpeq_epi16(_mm256_and_si256(haystack2, consideredBitsRegister), expectedBitsRegister);

	__m256i intermediateResult = _mm256_permutevar8x32_epi32(_mm256_packs_epi16( //43 + 6 = 49
		searchResult1, searchResult2
	), perm_mask);

	return static_cast<uint32_t>(_mm256_movemask_epi8(intermediateResult));
}

template<>
inline uint32_t SparsePartialKeys256<uint32_t>::partFindMasksByPattern(__m256i consideredBitsRegister, __m256i expectedBitsRegister, uint32_t const * rangeStart) const {
	__m256i haystack1 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart));
	__m256i haystack2 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart + 8));
	__m256i haystack3 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart + 16));
	__m256i haystack4 = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(rangeStart + 24)); //27 instr

	__m256i const perm_mask = _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0); //35 instr

	__m256i searchResult1 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack1, consideredBitsRegister), expectedBitsRegister);
	__m256i searchResult2 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack2, consideredBitsRegister), expectedBitsRegister);
	__m256i searchResult3 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack3, consideredBitsRegister), expectedBitsRegister);
	__m256i searchResult4 = _mm256_cmpeq_epi32(_mm256_and_si256(haystack4, consideredBitsRegister), expectedBitsRegister); //35 + 8 = 43

	__m256i intermediateResult = _mm256_permutevar8x32_epi32(_mm256_packs_epi16( //43 + 6 = 49
		_mm256_permutevar8x32_epi32(_mm256_packs_epi32(searchResult1, searchResult2), perm_mask),
		_mm256_permutevar8x32_epi32(_mm256_packs_epi32(searchResult3, searchResult4), perm_mask)
	), perm_mask);

	return static_cast<uint32_t>(_mm256_movemask_epi8(intermediateResult));
}

} }

#endif
