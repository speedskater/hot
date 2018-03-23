#ifndef __HOT__TREE_TEST_HELPER__
#define __HOT__TREE_TEST_HELPER__

#include <pwd.h>
#include <unistd.h>

#include <array>
#include <string>
#include <set>
#include <vector>

#include <idx/contenthelpers/KeyUtilities.hpp>
#include <idx/contenthelpers/TidConverters.hpp>

#include <idx/utils/RandomRangeGenerator.hpp>
#include <idx/utils/8ByteDatFileIO.hpp>

#include <hot/rowex/HOTRowexChildPointer.hpp>

namespace hot { namespace rowex {

inline bool isPartitionCorrect(HOTRowexChildPointer const * childPointer) {
	return childPointer->executeForSpecificNodeType(false, [&](const auto & node) {
		return node.isPartitionCorrect();
	});
}

template<typename ValueType, template <typename> typename KeyExtractor> inline bool isSubTreeValid(HOTRowexChildPointer const * currentSubtreeRoot) {
	KeyExtractor<ValueType> extractKey;
	using KeyType = decltype(extractKey(std::declval<ValueType>()));

	std::array<HOTRowexChildPointer const *, 60> nodeStack;
	std::array<unsigned int, 60> entryIndexStack;

	int stackIndex = 0;
	nodeStack[0] = currentSubtreeRoot;
	entryIndexStack[0] = 0;

	bool isValid = true;
	while(isValid && stackIndex >= 0) {
		if(nodeStack[stackIndex]->isLeaf()) {
			KeyType const & key = extractKey(idx::contenthelpers::tidToValue<ValueType>(nodeStack[stackIndex]->getTid()));
			auto const & fixedSizedKey = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(key));
			uint8_t const * rawValue = idx::contenthelpers::interpretAsByteArray(fixedSizedKey);

			for(int i=0; i < stackIndex; ++i) {
				bool isNodeEntryValid = (*nodeStack[i+1] == (*nodeStack[i]->search(rawValue)));
				if(!isNodeEntryValid) {
					std::cout << "node entry is invalid for path [";
					for(int j = 0; j <= i; ++j) {
						if(j > 0) {
							std::cout << ", ";
						}
						std::cout << entryIndexStack[j];
					}
					std::cout << "]" << std::endl;
					isValid = false;
				}
			}
			--stackIndex;
		} else {
			isValid &= isPartitionCorrect(nodeStack[stackIndex]);

			HOTRowexChildPointer nodePointer = *nodeStack[stackIndex];
			HOTRowexNodeBase* node = nodePointer.getNode();
			unsigned int indexInNode = entryIndexStack[stackIndex]++;
			if(indexInNode < node->getNumberEntries()) {
				HOTRowexChildPointer* childPointer = node->getPointers() + indexInNode;
				nodeStack[++stackIndex] = childPointer;

				if(!childPointer->isLeaf()) {
					uint16_t leastSignificantBitIndexForEntry = nodePointer.executeForSpecificNodeType(false, [&](const auto & node) {
						return node.getLeastSignificantDiscriminativeBitForEntry(indexInNode);
					});
					uint16_t mostSignificantBitIndexForChildNode = childPointer->executeForSpecificNodeType(false, [&](const auto & node) {
						return node.mDiscriminativeBitsRepresentation.mMostSignificantDiscriminativeBitIndex;
					});
					bool isLeastSignificantBitIndexInParentEntryStrictlySmallerThanMostSignificantBitIndexInChild
						= leastSignificantBitIndexForEntry < mostSignificantBitIndexForChildNode;
					if(!isLeastSignificantBitIndexInParentEntryStrictlySmallerThanMostSignificantBitIndexInChild) {
						//uint anotherLeastSignifcantBitIndexForEntry = nodePointer.getLeastSignificantDiscriminativeBitForEntry(indexInNode);
						std::cout << "node entry has larger bit index for least significant bit than the index of the most significan bit in the chlid node [";
						for(int j = 0; j < stackIndex; ++j) {
							if(j > 0) {
								std::cout << ", ";
							}
							std::cout << (entryIndexStack[j] - 1);
						}
						std::cout << "]" << std::endl;
						std::cout << "leastSignificantBitIndexForEntry :: " << leastSignificantBitIndexForEntry << " vs. mostSignificantBitIndexForChildNode :: " << mostSignificantBitIndexForChildNode << std::endl;
						isValid = false;
					}
				}

				entryIndexStack[stackIndex] = 0;
			} else {
				--stackIndex;
			}
		}
	}
	return isValid;
}

std::string getHomeDirectory() {
	struct passwd *pw = getpwuid(getuid());

	return std::string(pw->pw_dir);
}

std::vector<uint64_t> getTriples(size_t numberTriples) {
	return idx::utils::readDatFile(getHomeDirectory() + std::string("/datasets/yago.triples.dat"), numberTriples);
}

std::pair<std::set<uint64_t>, std::vector<uint64_t>> getRandomNumbers(size_t numberRandomNumbersToGenerate) {
	std::pair<std::set<uint64_t>, std::vector<uint64_t>> generatedData;
	std::vector<uint64_t> & valuesToInsert = generatedData.second;
	std::set<uint64_t> uniqueRandomNumbers = generatedData.first;
	idx::utils::RandomRangeGenerator<uint64_t> rnd{12344567, 0, INT64_MAX};

	//uint numberValues = 10000;
	//uint numberValues = 16000;

	while (uniqueRandomNumbers.size() < numberRandomNumbersToGenerate) {
		uint64_t newRandomNumber = rnd();
		uniqueRandomNumbers.insert(newRandomNumber);
		valuesToInsert.push_back(newRandomNumber);
	}

	return generatedData;
}

}}

#endif