//
//  @author robert.binna@uibk.ac.at
//

#include <cstdint>
#include <algorithm>
#include <functional>
#include <memory>
#include <iostream>
#include <unordered_set>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/ContentEquals.hpp>
#include <idx/contenthelpers/KeyUtilities.hpp>
#include <idx/contenthelpers/KeyComparator.hpp>
#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/TidConverters.hpp>

#include <hot/commons/Algorithms.hpp>
#include <hot/commons/BiNode.hpp>
#include <hot/commons/BiNodeInformation.hpp>
#include <hot/commons/DiscriminativeBit.hpp>
#include <hot/commons/InsertInformation.hpp>
#include <hot/commons/SearchResultForInsert.hpp>
#include <hot/commons/TwoEntriesNode.hpp>

#include <hot/singlethreaded/HOTSingleThreadedNode.hpp>
#include <hot/testhelpers/PartialKeyMappingTestHelper.hpp>

namespace hot { namespace singlethreaded {


template<typename ValueType, template <typename> typename KeyExtractor = idx::contenthelpers::IdentityKeyExtractor> hot::commons::DiscriminativeBit getSignifkantKeyInformation(ValueType existingValue, ValueType newValue) {
	KeyExtractor<ValueType> extractKey;
	using KeyType = decltype(extractKey(existingValue));
	KeyType const & key1 = extractKey(existingValue);
	KeyType const & key2 = extractKey(newValue);

	auto const & fixedSizedKey1 = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(key1));
	auto const & fixedSizedKey2 = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(key2));

	uint8_t const *rawKey1 = idx::contenthelpers::interpretAsByteArray(fixedSizedKey1);
	uint8_t const *rawKey2 = idx::contenthelpers::interpretAsByteArray(fixedSizedKey2);

	size_t diffingByteIndex = std::mismatch(rawKey1, rawKey1 + idx::contenthelpers::getMaxKeyLength<KeyType>(), rawKey2).first - rawKey1;

	return {
		static_cast<uint16_t>(diffingByteIndex),
		rawKey1[diffingByteIndex],
		rawKey2[diffingByteIndex]
	};
}

template<typename ValueType> bool isEqual(ValueType value1, ValueType value2) {
	return idx::contenthelpers::contentEquals(value1, value2);
}


template<typename ValueType, template <typename> typename KeyExtractor = idx::contenthelpers::IdentityKeyExtractor> hot::commons::BiNode<HOTSingleThreadedChildPointer> createBiNodeForValues(ValueType value1, ValueType value2) {
	hot::commons::DiscriminativeBit const & keyInformation = getSignifkantKeyInformation<ValueType, KeyExtractor>(value1, value2);
	return hot::commons::BiNode<HOTSingleThreadedChildPointer>::createFromExistingAndNewEntry(
		keyInformation,
		HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(value1)) ,
		HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(value2))
	);
};

template<typename ValueType, typename NodeType> hot::commons::SearchResultForInsert searchForInsert(std::shared_ptr<NodeType> const & node, ValueType newValue) {
	auto const & fixedSizedKey = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(newValue));
	uint8_t const *rawKey = idx::contenthelpers::interpretAsByteArray(fixedSizedKey);

	hot::commons::SearchResultForInsert searchResult;
	node->searchForInsert(searchResult, rawKey);
	return searchResult;
}

template<typename ValueType, typename NodeType> void checkNode(
	std::shared_ptr<NodeType> const & node,
	size_t expectedNumberEntries,
	std::vector<ValueType> const & valuesToInsert,
	size_t numberValuesUsedFromVector
) {
	BOOST_REQUIRE_MESSAGE(node.get() != nullptr, "It seems if the type checking has failed.");
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> temporarySet(valuesToInsert.begin(), valuesToInsert.begin() + numberValuesUsedFromVector);
	std::vector<ValueType> sortedValues(temporarySet.begin(), temporarySet.end());

	BOOST_REQUIRE_EQUAL(sortedValues.size(), expectedNumberEntries);
	BOOST_REQUIRE_EQUAL(node->getNumberEntries(), expectedNumberEntries);

	BOOST_REQUIRE(node->isPartitionCorrect());

	std::vector<ValueType> containedValues;

	std::transform(node->begin(), node->end(), std::back_inserter(containedValues), [](HOTSingleThreadedChildPointer const & childPointer) {
		return idx::contenthelpers::tidToValue<ValueType>(childPointer.getTid());
	});

	BOOST_REQUIRE_EQUAL_COLLECTIONS(containedValues.begin(), containedValues.end(), sortedValues.begin(), sortedValues.end());

	for(size_t i=0; i < sortedValues.size(); ++i) {
		ValueType const & foundValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[i].getTid());
		BOOST_REQUIRE_EQUAL(foundValue, sortedValues[i]);
	}
};

template<typename Typename> struct BaseType {
	typedef typename std::remove_cv<typename std::remove_reference<Typename>::type>::type type;
};

template<typename ExpectedType, typename GivenType> struct ReturnValueOnMatchingTypes {
	std::shared_ptr<ExpectedType const> operator()(std::shared_ptr<GivenType const> /* given */) {
		return std::shared_ptr<ExpectedType const>(nullptr);
	};
};

template<typename GivenType> struct ReturnValueOnMatchingTypes<GivenType, GivenType> {
	std::shared_ptr<GivenType const> operator()(std::shared_ptr<GivenType const> given) {
		return given;
	};
};



template<typename ValueType, typename ExpectedNodeType, typename NodeType> std::shared_ptr<ExpectedNodeType const> insertIntoSingleNode(
	std::shared_ptr<NodeType const> const & node, std::vector<ValueType> const & valuesToInsert, size_t indexOfValueToInsert
) {
	static ReturnValueOnMatchingTypes<typename BaseType<ExpectedNodeType>::type, typename BaseType<NodeType>::type> returnValueOnMatchingTypes;
	if(indexOfValueToInsert == valuesToInsert.size()) {
		if(!std::is_same<NodeType, ExpectedNodeType>::value) {
			BOOST_FAIL("insertIntoSingleNode should have resulted in Node With extraction type "
					 << hot::testhelpers::getExtractionTypeName<typename ExpectedNodeType::DiscriminativeBitsRepresentationType>()
					 << " and mask type"
					 << hot::testhelpers::getPartialKeyTypeName<typename ExpectedNodeType::PartialKeyType>()
					 << " instead of "
					 << hot::testhelpers::getExtractionTypeName<typename NodeType::DiscriminativeBitsRepresentationType>()
					 << " and mask type"
					 << hot::testhelpers::getPartialKeyTypeName<typename NodeType::PartialKeyType>()
			);
		}

		std::set<ValueType> sortedUniqueEntries { valuesToInsert.begin(), valuesToInsert.end() };
		std::vector<ValueType> sortedEntries { sortedUniqueEntries.begin(), sortedUniqueEntries.end() };

		return returnValueOnMatchingTypes(node);
	} else {
		ValueType newValue = valuesToInsert[indexOfValueToInsert];
		hot::commons::SearchResultForInsert const & searchResult = searchForInsert<ValueType>(node, newValue);
		ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());

		if(idx::contenthelpers::contentEquals(newValue, existingValue)) {
			return insertIntoSingleNode<ValueType, ExpectedNodeType, NodeType>(node, valuesToInsert, indexOfValueToInsert + 1);
		} else {
			//std::cout << "inserting value with index :: " << indexOfValueToInsert << " and value " << valuesToInsert[indexOfValueToInsert] << std::endl;
			HOTSingleThreadedChildPointer newNode = node->addEntry(
				node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation<ValueType, idx::contenthelpers::IdentityKeyExtractor>(existingValue, newValue)),
				HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(newValue))
			);
			//std::cout << "inserted value with index :: " << indexOfValueToInsert << " and value " << valuesToInsert[indexOfValueToInsert] << std::endl;
			return newNode.executeForSpecificNodeType(false, [&](auto const & newNodeReference) {
				typedef typename BaseType<decltype(newNodeReference)>::type ResultingNodeType;
				std::shared_ptr<ResultingNodeType const> nodePointer { &newNodeReference };
				size_t numberValuesInserted = indexOfValueToInsert + 1;
				//newNode.printPartialKeysWithMappings();
				checkNode(nodePointer, numberValuesInserted, valuesToInsert, numberValuesInserted);
				return insertIntoSingleNode<ValueType, ExpectedNodeType>(nodePointer, valuesToInsert, numberValuesInserted);
			});
		}
	}
}

template<typename DiscriminativeBitsRepresentationType, typename PartialKeyType> std::shared_ptr<HOTSingleThreadedNode<DiscriminativeBitsRepresentationType, PartialKeyType> const> expectNodeType(HOTSingleThreadedChildPointer const & childPointer) {
	return childPointer.executeForSpecificNodeType(false, [&](auto const & newNodeReference) {
		typedef typename BaseType<decltype(newNodeReference)>::type ResultingNodeType;
		typedef HOTSingleThreadedNode<DiscriminativeBitsRepresentationType, PartialKeyType> ExpectedNodeType;
		static ReturnValueOnMatchingTypes<ExpectedNodeType, ResultingNodeType> returnValueOnMatchingTypes;

		return returnValueOnMatchingTypes(std::shared_ptr<const ResultingNodeType> { &newNodeReference });
	});
};

template<typename ValueType, typename ExpectedNodeType> std::shared_ptr<ExpectedNodeType const> testValuesForSingleNode(std::vector<ValueType> const & valuesToInsert) {
	BOOST_REQUIRE(valuesToInsert.size() >= 2);
	std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> initialNode { hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues<ValueType, idx::contenthelpers::IdentityKeyExtractor>(valuesToInsert[0], valuesToInsert[1])) };
	checkNode(initialNode, 2, valuesToInsert, 2);
	std::shared_ptr<ExpectedNodeType const> node{
		insertIntoSingleNode<ValueType, ExpectedNodeType>(initialNode, valuesToInsert, 2)
	};
	checkNode(node, valuesToInsert.size(), valuesToInsert, valuesToInsert.size());
	return node;
}

template<typename ValueType, typename NodeType, typename ResultHandler> void insertIntoSingleNodeWithoutTypeCheck(
	std::shared_ptr<NodeType const> const & node, std::vector<ValueType> const & valuesToInsert, size_t indexOfValueToInsert, ResultHandler const & handler
) {
	if(indexOfValueToInsert == valuesToInsert.size()) {
		std::set<ValueType> sortedUniqueEntries { valuesToInsert.begin(), valuesToInsert.end() };
		std::vector<ValueType> sortedEntries { sortedUniqueEntries.begin(), sortedUniqueEntries.end() };

		checkNode(node, sortedUniqueEntries.size(), valuesToInsert, valuesToInsert.size());
		handler(node);
	} else {
		ValueType newValue = valuesToInsert[indexOfValueToInsert];
		hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, newValue);
		ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
		if(isEqual(newValue, existingValue)) {
			insertIntoSingleNodeWithoutTypeCheck<ValueType, NodeType, ResultHandler>(node, valuesToInsert, indexOfValueToInsert + 1, handler);
		} else {
			//std::cout << "inserting value with index :: " << indexOfValueToInsert << " and value " << valuesToInsert[indexOfValueToInsert] << std::endl;
			HOTSingleThreadedChildPointer newNode = node->addEntry(
				node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, newValue)),
				HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(newValue))
			);
			//std::cout << "inserted value with index :: " << indexOfValueToInsert << " and value " << valuesToInsert[indexOfValueToInsert] << std::endl;
			return newNode.executeForSpecificNodeType(false, [&](auto const & newNodeReference) {
				typedef typename BaseType<decltype(newNodeReference)>::type ResultingNodeType;
				std::shared_ptr<ResultingNodeType const> nodePointer { &newNodeReference };
				size_t numberValuesInserted = indexOfValueToInsert + 1;
				//newNode.printPartialKeysWithMappings();
				checkNode(nodePointer, numberValuesInserted, valuesToInsert, numberValuesInserted);
				insertIntoSingleNodeWithoutTypeCheck<ValueType, ResultingNodeType, ResultHandler>(nodePointer, valuesToInsert, numberValuesInserted, handler);
			});
		}
	}
}

template<typename ValueType, typename ResultHandler> void insertIntoSingleNodeWithoutTypeCheck(std::vector<ValueType> const & valuesToInsert, ResultHandler const & resultHandler) {
	BOOST_REQUIRE(valuesToInsert.size() >= 2);
	std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> initialNode { hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues(valuesToInsert[0], valuesToInsert[1])) };
	checkNode(initialNode, 2, valuesToInsert, 2);
	insertIntoSingleNodeWithoutTypeCheck(
			initialNode,
			valuesToInsert,
			2,
			resultHandler
	);
}

HOTSingleThreadedDeletionInformation getDeletionInformation(HOTSingleThreadedChildPointer const & nodePointer, uint32_t indexOfEntryToRemove) {
	return nodePointer.executeForSpecificNodeType(false, [indexOfEntryToRemove](auto const & node) {
		return node.getDeletionInformation(indexOfEntryToRemove);
	});
}

template<typename ValueType, typename NodeType> HOTSingleThreadedChildPointer deleteStepwiseFromNode(std::shared_ptr<NodeType const> const & node, std::vector<ValueType> const & remainingValues) {
	std::vector<ValueType> sortedRemainingValues(remainingValues.begin(), remainingValues.end());
	typename idx::contenthelpers::KeyComparator<ValueType>::type comp;
	std::sort(sortedRemainingValues.begin(), sortedRemainingValues.end(), comp);
	ptrdiff_t indexOfEntryToRemove =
		std::find(sortedRemainingValues.begin(), sortedRemainingValues.end(), remainingValues.back()) -
		sortedRemainingValues.begin();
	HOTSingleThreadedDeletionInformation const & deletionInformation = node->getDeletionInformation(indexOfEntryToRemove);
	HOTSingleThreadedChildPointer pointerToNodeWithElementRemoved = node->removeEntry(deletionInformation);
	std::vector<ValueType> nexStepRemainingElements;
	std::copy(sortedRemainingValues.begin(), sortedRemainingValues.begin() + indexOfEntryToRemove, std::back_inserter(nexStepRemainingElements));
	std::copy(sortedRemainingValues.begin() + indexOfEntryToRemove + 1, sortedRemainingValues.end(), std::back_inserter(nexStepRemainingElements));

	HOTSingleThreadedChildPointer pointerToLastElement;
	if (nexStepRemainingElements.size() == 1) {
		BOOST_REQUIRE_EQUAL(idx::contenthelpers::tidToValue<ValueType>(pointerToNodeWithElementRemoved.getTid()), nexStepRemainingElements[0]);
		pointerToLastElement = pointerToNodeWithElementRemoved;
	} else {
		pointerToLastElement = pointerToNodeWithElementRemoved.executeForSpecificNodeType(false,
			[&](auto const &nodeWithElementRemoved) {
				typedef typename BaseType<decltype(nodeWithElementRemoved)>::type ResultingNodeType;
				std::shared_ptr<ResultingNodeType const> currentNodePointer{&nodeWithElementRemoved};
				std::shared_ptr<ResultingNodeType const> expectedNode = testValuesForSingleNode<ValueType, ResultingNodeType>(
					nexStepRemainingElements);
				checkNode(
					currentNodePointer,
					nexStepRemainingElements.size(),
					nexStepRemainingElements,
					nexStepRemainingElements.size()
				);
				return deleteStepwiseFromNode(currentNodePointer, nexStepRemainingElements);
			}
		);
	}
	return pointerToLastElement;
}

template<typename ValueType> void testStepwiseDeletion(std::vector<ValueType> const & valuesToInsertAndRemove) {
	insertIntoSingleNodeWithoutTypeCheck(valuesToInsertAndRemove, [&](auto const & initialNode) {
		return deleteStepwiseFromNode(initialNode, valuesToInsertAndRemove);
	});
}

template<typename ValueType> std::vector<ValueType> getUniqueValues(std::vector<ValueType> const & values) {
	std::vector<ValueType> originalOrderUniqueValues;
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> temporarySetUniqueValues;
	for(ValueType const & current : values) {
		if(temporarySetUniqueValues.find(current) == temporarySetUniqueValues.end()) {
			temporarySetUniqueValues.insert(current);
			originalOrderUniqueValues.push_back(current);
		}
	}
	return originalOrderUniqueValues;
}

template<typename ValueType> void testRemove(std::vector<ValueType> const & valuesToInsert) {
	typename idx::contenthelpers::KeyComparator<ValueType>::type comp;

	std::vector<ValueType> const & uniqueValues = getUniqueValues(valuesToInsert);
	std::vector<ValueType> sortedUniqueValues { uniqueValues.begin(), uniqueValues.end() };
	std::sort(sortedUniqueValues.begin(), sortedUniqueValues.end(), comp);
	std::vector<ValueType> reverseValues { sortedUniqueValues.rbegin(), sortedUniqueValues.rend() };
	std::vector<ValueType> randomOrderedValues { uniqueValues.begin(), uniqueValues.end() };
	std::random_shuffle(randomOrderedValues.begin(), randomOrderedValues.end());

	testStepwiseDeletion(uniqueValues);
	testStepwiseDeletion(sortedUniqueValues);
	testStepwiseDeletion(reverseValues);
	testStepwiseDeletion(randomOrderedValues);
}

template<typename ValueType, typename InitialNodeType> void testCompressToSingleEntry(std::shared_ptr<const InitialNodeType> const & initialNode, std::vector<ValueType> const & sortedEntries) {
	for(uint32_t i=1; i < sortedEntries.size(); ++i) {
		hot::singlethreaded::HOTSingleThreadedChildPointer childPointer = initialNode->compressEntries(i, 1u);
		BOOST_REQUIRE(childPointer.isLeaf());
		BOOST_REQUIRE_EQUAL(idx::contenthelpers::tidToValue<ValueType>(childPointer.getTid()), sortedEntries[i]);
	}
}

template<typename ValueType> void checkNodePointerAndDeallocate(HOTSingleThreadedChildPointer childPointer, std::vector<ValueType> expectedSortedValues, std::vector<ValueType> unexpectedValues = std::vector<ValueType>{}) {
	childPointer.executeForSpecificNodeType(false, [&](auto const & newNodeReference) {
		typedef typename BaseType<decltype(newNodeReference)>::type ResultingNodeType;
		std::shared_ptr<ResultingNodeType const> node { &newNodeReference };
		std::shared_ptr<ResultingNodeType const> expectedNode = testValuesForSingleNode<ValueType, ResultingNodeType>(expectedSortedValues);

		BOOST_REQUIRE_EQUAL(childPointer.getNumberEntries(), expectedSortedValues.size());

		if(!std::equal(node->begin(), node->end(), expectedNode->begin(), expectedNode->end())) {
			std::ostringstream message;
			message << "Node contains values: " << std::endl;
			std::for_each(node->begin(), node->end(), [&](HOTSingleThreadedChildPointer const & entry) {
				message << idx::contenthelpers::tidToValue<ValueType>(entry.getTid()) << std::endl;
			});
			message << "but expected node should contain: " << std::endl;
			std::for_each(expectedNode->begin(), expectedNode->end(), [&](HOTSingleThreadedChildPointer const & entry) {
				message << idx::contenthelpers::tidToValue<ValueType>(entry.getTid()) << std::endl;
			});
			BOOST_FAIL("Nodes differ" << message.str());
		}



		for (ValueType const & expectedValue : expectedSortedValues) {
			auto const & key = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(expectedValue));
			uint8_t const * rawKey = idx::contenthelpers::interpretAsByteArray(key);
			ValueType const & foundValue = idx::contenthelpers::tidToValue<ValueType>(node->search(rawKey)->getTid());
			BOOST_REQUIRE_EQUAL(expectedValue, foundValue);
		}

		for (ValueType const &  unexpectedValue : unexpectedValues) {
			auto const & key = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(unexpectedValue));
			uint8_t const * rawKey = idx::contenthelpers::interpretAsByteArray(key);

			ValueType const & foundValue = idx::contenthelpers::tidToValue<ValueType>(node->search(rawKey)->getTid());
			BOOST_REQUIRE_NE(unexpectedValue, foundValue);
		}
	});
}

template<typename ValueType, typename InitialNodeType> void testCompressRange(std::shared_ptr<const InitialNodeType> const & initialNode, std::vector<ValueType> const & sortedEntries, uint32_t startIndex, uint32_t numberEntries) {
	BOOST_REQUIRE_GE(numberEntries, 2u);

	std::vector<ValueType> expectedValues { sortedEntries.begin() + startIndex, sortedEntries.begin() + startIndex + numberEntries };
	std::vector<ValueType> unexpectedValues { sortedEntries.begin(), sortedEntries.end() };
	unexpectedValues.erase( unexpectedValues.begin() + startIndex, unexpectedValues.begin() + startIndex + numberEntries );

	HOTSingleThreadedChildPointer const & compressedNodePointer = initialNode->compressEntries(startIndex, numberEntries);
	checkNodePointerAndDeallocate(compressedNodePointer, expectedValues, unexpectedValues);
}

template<typename ValueType, typename InitialNodeType> void testCompressRangeOfValues(std::shared_ptr<const InitialNodeType> const & initialNode, std::vector<ValueType> const & sortedEntries) {

	for(unsigned int rangeSize = 2; rangeSize < initialNode->getNumberEntries(); ++rangeSize) {
		testCompressRange(initialNode, sortedEntries, 0u, rangeSize);
		testCompressRange(initialNode, sortedEntries, initialNode->getNumberEntries() - rangeSize, rangeSize);
	}
}


template<typename ValueType, typename InitialNodeType> void testCompressNode(std::shared_ptr<const InitialNodeType> node, std::vector<ValueType> const & initialEntries) {
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> sortedUniqueEntries { initialEntries.begin(), initialEntries.end() };
	std::vector<ValueType> sortedEntries { sortedUniqueEntries.begin(), sortedUniqueEntries.end() };

	testCompressToSingleEntry(node, sortedEntries);
	testCompressRangeOfValues(node, sortedEntries);
}

template<typename ExpectedNodeType> std::shared_ptr<const ExpectedNodeType> testIntSingleNode(std::vector<std::uint64_t> const & valuesToInsert) {
	std::shared_ptr<const ExpectedNodeType> node { testValuesForSingleNode<uint64_t, ExpectedNodeType>(valuesToInsert) };
	testCompressNode(node, valuesToInsert);
	testRemove(valuesToInsert);
	return node;
}


template<typename ExpectedNodeType> std::shared_ptr<const ExpectedNodeType> testStringValuesForSingleNode(std::vector<std::string> const & stringsToInsert) {
	std::vector<char const *> rawStringsToIndex = hot::testhelpers::stdStringsToCStrings(stringsToInsert);

	std::shared_ptr<const ExpectedNodeType> node { testValuesForSingleNode<char const *, ExpectedNodeType>(rawStringsToIndex) };
	testCompressNode(node, rawStringsToIndex);
	testRemove(rawStringsToIndex);

	return node;
}


BOOST_AUTO_TEST_SUITE(SIMDCobTrieIncrementalNodeTest)

BOOST_AUTO_TEST_CASE(testSequentialValuesIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert;
	for(int i=0; i < 32; ++i) {
		valuesToInsert.push_back(i);
	}
	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t >>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testTwoValuesReverseIntoSingleNode) {
	std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>> twoValuesNodes { hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues<uint64_t>(31, 30)) };
	checkNode(twoValuesNodes, 2, std::vector<uint64_t> { 31, 30 }, 2);
}

BOOST_AUTO_TEST_CASE(testSequentialValuesReverseIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert;

	for(int i = 31; i >= 0; --i) {
		valuesToInsert.push_back(i);
	}

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testSomeReverseValuesIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert = { 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21 };
	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(valuesToInsert);
}

template<typename ValueType> void testCompressAndAdd(std::vector<ValueType> const & valueToInsert) {
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> uniqueValues { valueToInsert.begin(), valueToInsert.end() };
	std::vector<ValueType> sortedUniqueValues { uniqueValues.begin(), uniqueValues.end() };

	for(size_t absoluteIndexOfValueToLeaveOut = 0; absoluteIndexOfValueToLeaveOut < sortedUniqueValues.size(); ++absoluteIndexOfValueToLeaveOut) {
		ValueType const & valueToLeaveOut = sortedUniqueValues[absoluteIndexOfValueToLeaveOut];
		std::vector<ValueType> valuesToInsertWithSpecifcValueLeftOut { valueToInsert.begin(), valueToInsert.end() };
		valuesToInsertWithSpecifcValueLeftOut.erase(
			std::remove_if(
				valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end(),
				std::bind(std::equal_to<ValueType>(), valueToLeaveOut, std::placeholders::_1)
			),
			valuesToInsertWithSpecifcValueLeftOut.end()
		);



		insertIntoSingleNodeWithoutTypeCheck(valuesToInsertWithSpecifcValueLeftOut, [&](auto const & node) {
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, valueToLeaveOut);
			ValueType existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, valueToLeaveOut));
			std::uint32_t largestIndexInAffectedSubtree = insertInformation.getFirstIndexInAffectedSubtree() + insertInformation.getNumberEntriesInAffectedSubtree() - 1;

			//constructs a split with the first range containing the value to leave out -> and hence to insert
			for(unsigned int firstRangeSize = std::max(absoluteIndexOfValueToLeaveOut, static_cast<size_t>(largestIndexInAffectedSubtree + 1)); firstRangeSize < (sortedUniqueValues.size() - 1u); ++firstRangeSize) {
				HOTSingleThreadedChildPointer newPartChildPointer = node->compressEntriesAndAddOneEntryIntoNewNode(0, firstRangeSize, insertInformation,
					HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(valueToLeaveOut))
				);

				std::vector<ValueType> includedValues { sortedUniqueValues.begin(), sortedUniqueValues.begin() + firstRangeSize + 1 };
				std::vector<ValueType> excludedValues { sortedUniqueValues.begin() + firstRangeSize + 1, sortedUniqueValues.end() };

				checkNodePointerAndDeallocate(newPartChildPointer, includedValues, excludedValues);
			}
		});
	}
}

template<typename ValueType> void testSplit(std::vector<ValueType> const &valuesToInsert) {
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> uniqueValues { valuesToInsert.begin(), valuesToInsert.end() };
	std::vector<ValueType> sortedUniqueValues { uniqueValues.begin(), uniqueValues.end() };

	for(size_t absoluteIndexOfValueToLeaveOut = 0; absoluteIndexOfValueToLeaveOut < sortedUniqueValues.size(); ++absoluteIndexOfValueToLeaveOut) {
		ValueType const & valueToLeaveOut = sortedUniqueValues[absoluteIndexOfValueToLeaveOut];
		auto const & keyOfValueToLeaveOut = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(valueToLeaveOut));
		uint8_t const * rawBytesOfValueToLeaveOut = idx::contenthelpers::interpretAsByteArray(keyOfValueToLeaveOut);

		std::vector<ValueType> valuesToInsertWithSpecifcValueLeftOut { valuesToInsert.begin(), valuesToInsert.end() };
		valuesToInsertWithSpecifcValueLeftOut.erase(
			std::remove_if(
				valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end(),
				std::bind(std::equal_to<ValueType>(), valueToLeaveOut, std::placeholders::_1)
			),
			valuesToInsertWithSpecifcValueLeftOut.end()
		);

		insertIntoSingleNodeWithoutTypeCheck<ValueType>(valuesToInsertWithSpecifcValueLeftOut, [&](auto const & node) {
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, valueToLeaveOut);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, valueToLeaveOut));

			if(insertInformation.mKeyInformation.mAbsoluteBitIndex >= node->mDiscriminativeBitsRepresentation.mMostSignificantDiscriminativeBitIndex) {
				const hot::commons::DiscriminativeBit mostLeftBitSignificantKey = hot::commons::DiscriminativeBit {
					node->mDiscriminativeBitsRepresentation.mMostSignificantDiscriminativeBitIndex, 1};

				size_t numberSmallerEntries = std::count_if(node->begin(), node->end(), [&](HOTSingleThreadedChildPointer const &child) {
					ValueType const & value = idx::contenthelpers::tidToValue<ValueType>(child.getTid());
					auto  const & existingKey = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(value));

					uint8_t const *rawBytes = idx::contenthelpers::interpretAsByteArray(existingKey);
					return (rawBytes[mostLeftBitSignificantKey.mByteIndex] &
							mostLeftBitSignificantKey.getExtractionByte()) == 0;
				});
				size_t numberLargerEntries = std::count_if(node->begin(), node->end(), [&](HOTSingleThreadedChildPointer const &child) {
					ValueType const & value = idx::contenthelpers::tidToValue<ValueType>(child.getTid());
					auto  const & existingKey = idx::contenthelpers::toFixSizedKey(idx::contenthelpers::toBigEndianByteOrder(value));

					uint8_t const *rawBytes = idx::contenthelpers::interpretAsByteArray(existingKey);
					return (rawBytes[mostLeftBitSignificantKey.mByteIndex] &
							mostLeftBitSignificantKey.getExtractionByte()) != 0;
				});

				BOOST_REQUIRE_EQUAL(numberSmallerEntries + numberLargerEntries, node->getNumberEntries());

				bool wouldLeftOutValueBeInLargerPart =
					(rawBytesOfValueToLeaveOut[mostLeftBitSignificantKey.mByteIndex] &
					 mostLeftBitSignificantKey.getExtractionByte()) != 0;

				hot::commons::BiNode<HOTSingleThreadedChildPointer> newNodes = node->split(
					insertInformation, HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(valueToLeaveOut))
				);

				size_t expectedNumberEntriesInSmallerNode =
					numberSmallerEntries + ((wouldLeftOutValueBeInLargerPart) ? 0 : 1);
				size_t expectedNumberEntriesInLargerNode =
					numberLargerEntries + ((wouldLeftOutValueBeInLargerPart) ? 1 : 0);

				BOOST_REQUIRE_EQUAL(newNodes.mLeft.getNumberEntries(), expectedNumberEntriesInSmallerNode);
				BOOST_REQUIRE_EQUAL(newNodes.mRight.getNumberEntries(), expectedNumberEntriesInLargerNode);

				std::vector<ValueType> valuesInSmallerNode{sortedUniqueValues.begin(),
																			   sortedUniqueValues.begin() +
																			   expectedNumberEntriesInSmallerNode};
				std::vector<ValueType> valuesInLargerNode{
					sortedUniqueValues.begin() + expectedNumberEntriesInSmallerNode, sortedUniqueValues.end()};

				//std::cout << "Number smaller " << expectedNumberEntriesInSmallerNode << std::endl;
				if(expectedNumberEntriesInSmallerNode > 1) {
					checkNodePointerAndDeallocate(newNodes.mLeft, valuesInSmallerNode, valuesInLargerNode);
				} else {
					BOOST_REQUIRE_EQUAL(idx::contenthelpers::tidToValue<ValueType>(newNodes.mLeft.getTid()), valuesInSmallerNode[0]);
				}

				//std::cout << "Number larger " << expectedNumberEntriesInLargerNode << std::endl;
				if(expectedNumberEntriesInLargerNode > 1) {
					checkNodePointerAndDeallocate(newNodes.mRight, valuesInLargerNode, valuesInSmallerNode);
				} else {
					BOOST_REQUIRE_EQUAL(idx::contenthelpers::tidToValue<ValueType>(newNodes.mRight.getTid()), valuesInLargerNode[0]);
				}


			}
		});
	}
}

template<typename ValueType> void testMerge(std::vector<ValueType> const &valuesToInsert) {
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> uniqueValues { valuesToInsert.begin(), valuesToInsert.end() };
	std::vector<ValueType> sortedUniqueValues { uniqueValues.begin(), uniqueValues.end() };

	for(size_t absoluteIndexOfValueToLeaveOut = 0; absoluteIndexOfValueToLeaveOut < sortedUniqueValues.size(); ++absoluteIndexOfValueToLeaveOut) {
		ValueType const & valueToLeaveOut = sortedUniqueValues[absoluteIndexOfValueToLeaveOut];

		std::vector<ValueType> valuesToInsertWithSpecifcValueLeftOut { valuesToInsert.begin(), valuesToInsert.end() };
		valuesToInsertWithSpecifcValueLeftOut.erase(
			std::remove_if(
				valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end(),
				std::bind(std::equal_to<ValueType>(), valueToLeaveOut, std::placeholders::_1)
			),
			valuesToInsertWithSpecifcValueLeftOut.end()
		);

		insertIntoSingleNodeWithoutTypeCheck<ValueType>(valuesToInsertWithSpecifcValueLeftOut, [&](auto const & node) {
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, valueToLeaveOut);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, valueToLeaveOut));

			if(insertInformation.mKeyInformation.mAbsoluteBitIndex >= node->mDiscriminativeBitsRepresentation.mMostSignificantDiscriminativeBitIndex) {
				hot::commons::BiNode<HOTSingleThreadedChildPointer> newNodes = node->split(
					insertInformation, HOTSingleThreadedChildPointer(idx::contenthelpers::valueToTid(valueToLeaveOut))
				);

				if(newNodes.mLeft.getNumberEntries() >= 2 && newNodes.mRight.getNumberEntries() >= 2) {
					bool isInRightSide = absoluteIndexOfValueToLeaveOut >= newNodes.mLeft.getNumberEntries();
					HOTSingleThreadedDeletionInformation const deletionInformation = isInRightSide
						? getDeletionInformation(newNodes.mRight, absoluteIndexOfValueToLeaveOut - newNodes.mLeft.getNumberEntries())
						: getDeletionInformation(newNodes.mLeft, absoluteIndexOfValueToLeaveOut);
					HOTSingleThreadedChildPointer childPointer = mergeNodesAndRemoveEntryIfPossible(newNodes.mDiscriminativeBitIndex, newNodes.mLeft, newNodes.mRight, deletionInformation, isInRightSide);
					checkNodePointerAndDeallocate(childPointer, valuesToInsertWithSpecifcValueLeftOut);
				}

				if(newNodes.mLeft.getNumberEntries() > 1) {
					newNodes.mLeft.free();
				}
				if(newNodes.mRight.getNumberEntries() > 1) {
					newNodes.mRight.free();
				}
			}
		});
	}
}

template<typename ValueType> void executeTestRemoveAndAdd(std::vector<ValueType> const &initialValues, ValueType firstValue, ValueType lastValue) {
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> initialUniqueValues { initialValues.begin(), initialValues.end() };
	std::vector<ValueType> sortedUniqueValues { initialUniqueValues.begin(), initialUniqueValues.end() };

	HOTSingleThreadedChildPointer firstValuePointer(idx::contenthelpers::valueToTid(firstValue));
	HOTSingleThreadedChildPointer lastValuePointer(idx::contenthelpers::valueToTid(lastValue));

	for(size_t absoluteIndexOfValueToLeaveOut = 0; absoluteIndexOfValueToLeaveOut < initialValues.size(); ++absoluteIndexOfValueToLeaveOut) {
		ValueType const & valueToLeaveOut = sortedUniqueValues[absoluteIndexOfValueToLeaveOut];
		HOTSingleThreadedChildPointer valueToLeaveOutPonter(idx::contenthelpers::valueToTid(valueToLeaveOut));
		std::vector<ValueType> valuesToInsertWithSpecifcValueLeftOut { sortedUniqueValues.begin(), sortedUniqueValues.end() };
		valuesToInsertWithSpecifcValueLeftOut.erase(
			std::remove_if(
				valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end(),
				std::bind(std::equal_to<ValueType>(), valueToLeaveOut, std::placeholders::_1)
			),
			valuesToInsertWithSpecifcValueLeftOut.end()
		);

		if(absoluteIndexOfValueToLeaveOut > 0) {
			ValueType const & firstValueInInitialRange = sortedUniqueValues[0];
			std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> twoEntriesNode(hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues(firstValueInInitialRange, valueToLeaveOut)));
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(twoEntriesNode, firstValue);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(twoEntriesNode->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = twoEntriesNode->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, firstValue));
			HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(twoEntriesNode->toChildPointer(), 0);
			HOTSingleThreadedChildPointer newNode = twoEntriesNode->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, firstValuePointer);
			std::vector<ValueType> expectedValues = { firstValue, valueToLeaveOut };
			checkNodePointerAndDeallocate(newNode, expectedValues);
		}

		if(absoluteIndexOfValueToLeaveOut > 0) {
			ValueType const & firstValueInInitialRange = sortedUniqueValues[0];
			std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> twoEntriesNode(hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues(firstValueInInitialRange, valueToLeaveOut)));
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(twoEntriesNode, firstValue);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(twoEntriesNode->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = twoEntriesNode->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, firstValue));
			HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(twoEntriesNode->toChildPointer(), 1);
			HOTSingleThreadedChildPointer newNode = twoEntriesNode->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, firstValuePointer);
			std::vector<ValueType> expectedValues = { firstValue, firstValueInInitialRange };
			checkNodePointerAndDeallocate(newNode, expectedValues);
		}

		if(absoluteIndexOfValueToLeaveOut > 0) {
			ValueType const & firstValueInInitialRange = sortedUniqueValues[0];
			std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> twoEntriesNode(hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues(firstValueInInitialRange, valueToLeaveOut)));
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(twoEntriesNode, lastValue);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(twoEntriesNode->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = twoEntriesNode->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, lastValue));
			HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(twoEntriesNode->toChildPointer(), 0);
			HOTSingleThreadedChildPointer newNode = twoEntriesNode->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, lastValuePointer);
			std::vector<ValueType> expectedValues = { valueToLeaveOut, lastValue };
			checkNodePointerAndDeallocate(newNode, expectedValues);
		}

		if(absoluteIndexOfValueToLeaveOut > 0) {
			ValueType const & firstValueInInitialRange = sortedUniqueValues[0];
			std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> twoEntriesNode(hot::commons::createTwoEntriesNode<HOTSingleThreadedChildPointer, HOTSingleThreadedNode>(createBiNodeForValues(firstValueInInitialRange, valueToLeaveOut)));
			hot::commons::SearchResultForInsert const & searchResult = searchForInsert(twoEntriesNode, lastValue);
			ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(twoEntriesNode->getPointers()[searchResult.mEntryIndex].getTid());
			hot::commons::InsertInformation const & insertInformation = twoEntriesNode->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, lastValue));
			HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(twoEntriesNode->toChildPointer(), 1);
			HOTSingleThreadedChildPointer newNode = twoEntriesNode->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, lastValuePointer);
			std::vector<ValueType> expectedValues = { firstValueInInitialRange, lastValue };
			checkNodePointerAndDeallocate(newNode, expectedValues);
		}

		if(absoluteIndexOfValueToLeaveOut > 0) {
			insertIntoSingleNodeWithoutTypeCheck<ValueType>(initialValues, [&](auto const & node) {
				hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, firstValue);
				ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
				hot::commons::InsertInformation const & insertInformation = node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, firstValue));

				HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(node->toChildPointer(), absoluteIndexOfValueToLeaveOut);
				HOTSingleThreadedChildPointer newNode = node->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, firstValuePointer);
				std::set<ValueType> finalUniqueValues(valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end());
				finalUniqueValues.insert(firstValue);
				std::vector<ValueType> sortedFinalUniqueValue(finalUniqueValues.begin(), finalUniqueValues.end());
				checkNodePointerAndDeallocate(newNode, sortedFinalUniqueValue);
			});
		}
		if(absoluteIndexOfValueToLeaveOut < (sortedUniqueValues.size() - 1)) {
			insertIntoSingleNodeWithoutTypeCheck<ValueType>(initialValues, [&](auto const & node) {
				hot::commons::SearchResultForInsert const & searchResult = searchForInsert(node, lastValue);
				ValueType const & existingValue = idx::contenthelpers::tidToValue<ValueType>(node->getPointers()[searchResult.mEntryIndex].getTid());
				hot::commons::InsertInformation const & insertInformation = node->getInsertInformation(searchResult.mEntryIndex, getSignifkantKeyInformation(existingValue, lastValue));

				HOTSingleThreadedDeletionInformation const & deletionInformation = getDeletionInformation(node->toChildPointer(), absoluteIndexOfValueToLeaveOut);
				HOTSingleThreadedChildPointer newNode = node->removeAndAddEntry(deletionInformation, insertInformation.mKeyInformation, lastValuePointer);
				std::vector<ValueType> sortedFinalUniqueValue(valuesToInsertWithSpecifcValueLeftOut.begin(), valuesToInsertWithSpecifcValueLeftOut.end());
				sortedFinalUniqueValue.push_back(lastValue);
				checkNodePointerAndDeallocate(newNode, sortedFinalUniqueValue);
			});
		}
	}
}

BOOST_AUTO_TEST_CASE(testCompressAndAddSomeReverseValue) {
	std::vector<uint64_t> valuesToInsert = { 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21 };
	testCompressAndAdd(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testSmallRandomValuesIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert = {
		0b100, //4
		0b000, //0
		0b110, //6
		0b101  //5
	};

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(valuesToInsert);
}


BOOST_AUTO_TEST_CASE(testAnotherSmallRandomValuesIntoSingleNode) {
	//Sorted First four values incremental insert order

	//source   value                                                              masks in node


	std::vector<uint64_t> valuesToInsert = {
		64ul,    // 01000000    0
		5ul,     // 00000101    1
		102ul,   // 01100110    2
		65ul,    // 01000001    3
		7ul,     // 00000111    4
		88ul,    // 01011000    5
		68ul,    // 01000100    6
		35ul,    // 00100011    7
		127ul,   // 01111111    8
		107ul,   // 01101011    9
	};

	/*5ul,     // 00000101    1
	7ul,     // 00000111    4
	35ul,    // 00100011    7
	64ul,    // 01000000    0
	65ul,    // 01000001    3
	68ul,    // 01000100    6
	88ul,    // 01011000    5
	102ul,   // 01100110    2
	107ul,   // 01101011    9
	127ul,   // 01111111    8*/

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(test3SmallRandomValuesIntoSingleNode) {
	/*355488278567739596ul,    // 0000010011101110111100101011010010110101110110000110000011001100    1
	418970542659199878ul,    // 0000010111010000011110110111110100011110100011011110001110000000    4
	1488787817663097441ul,   // 0001010010101001001111001010000111011001101111001110101001100001    16
	2583272014892537200ul,   // 0010001111011001101000000011010111110101111000001001011000000000    7
	3040631852046752166ul,   // 0010101000110010011111101000111100111001111111000001100110100110    13
	3658352497551999605ul,   // 0011001011000101000101000011011010110111110000000000001001110101    15
	4413874586873285858ul,   // 0011110101000001001110111000110100010001101101001100110011100010    11
	4529279813148173111ul,   // 0011111011011011001110111111111000101111000010110110001100110111    14
	4596340717661012313ul,   // 0011111111001001011110111000011110111110110110010100000101011001    10
	4620546740167642908ul,   // 0100000000011111011110101100011110001011110010000000111100011100    0
	4635995468481642529ul,   // 0100000001010110010111010101000011100111001010110100000000100001    3
	5058016125798318033ul,   // 0100011000110001101011101101111000101110011010000000000000000000    6
	5904155143473820934ul,   // 0101000111101111110001011101001001001001100011010111010100000110    12
	6358044926049913402ul,   // 0101100000111100010100000010110010000011001011100000110000000000    5
	7469126240319926998ul,   // 0110011110100111101010101011111000010000110100010111001011010110    2
	7736011505917826031ul,   // 0110101101011011110101010111101000111100100100010001001111101111    9
	9219209713614924562ul,   // 0111111111110001001101100110001110011001110110010010101100010010    8 */

	/*9ul,   // 00001001    1
	11ul,    // 00001011    4
	40ul,    // 00101000    16
	71ul,    // 01000111    7
	84ul,    // 01010100    13
	101ul,   // 01100101    15
	123ul,   // 01111011    11
	125ul,   // 01111101    14
	127ul,   // 01111111    10
	128ul,   // 10000000    0
	129ul,   // 10000001    3
	140ul,   // 10001100    6
	163ul,   // 10100011    12
	176ul,   // 10110000    5
	206ul,   // 11001110    2
	215ul,   // 11010111    9
	255ul    // 11111111    8*/

	std::vector<uint64_t> insertValues = {
		128ul,   // 10000000    0
		9ul,     // 00001001    1
		//206ul,   // 11001110    2
		129ul,   // 10000001    3
		11ul,    // 00001011    4
		//176ul,   // 10110000    5
		140ul,   // 10001100    6
		71ul,    // 01000111    7
		//255ul,   // 11111111    8
		//215ul,   // 11010111    9
		127ul,   // 01111111    10
		123ul,   // 01111011    11
		163ul,   // 10100011    12
		84ul,    // 01010100    13
		125ul,   // 01111101    14
		101ul,   // 01100101    15
		40ul    // 00101000    16
	};

	/*std::vector<uint64_t> insertValues = {
		9ul,     // 00001001    1
		11ul,    // 00001011    4
		40ul,    // 00101000    16
		71ul,    // 01000111    7
		84ul,    // 01010100    13
		101ul,   // 01100101    15
		123ul,   // 01111011    11
		125ul,   // 01111101    14
		127ul,   // 01111111    10
		128ul,   // 10000000    0
		129ul,   // 10000001    3
		140ul,   // 10001100    6
		163ul,   // 10100011    12
		176ul,   // 10110000    5
		206ul,   // 11001110    2
		215ul,   // 11010111    9
		255ul,   // 11111111    8
	};*/

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(insertValues);
}

BOOST_AUTO_TEST_CASE(testValuesWithUpto16differentBitsIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert;

	for(int i=0; i < 16; ++i) {
		valuesToInsert.push_back(1ul << i);
	}

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint16_t>>(valuesToInsert);
}


BOOST_AUTO_TEST_CASE(testValuesWithUpto32differentBitsIntoSingleNode) {
	std::vector<uint64_t> valuesToInsert;

	for(int i=0; i < 32; ++i) {
		valuesToInsert.push_back(1ul << i);
	}

	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint32_t>>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testCompressEntriesWithSuccessiveAnd32BitSet) {
	std::vector<uint64_t> valuesToInsert;
	for(uint64_t i=0; i < 32; ++i) {
		valuesToInsert.push_back(1ul << (i*2));
	}
	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint32_t>>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testRandomValuesIntoSingleNode) {
	//Sorted First four values incremental insert order

	//source   value                                                              masks in node
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    -

	//idx 1    0000010011101110111100101011010010110101110110000110000011001100    0    bit index 1                                           0
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    1                                                          1

	//idx 1    0000010011101110111100101011010010110101110110000110000011001100    00   bit index 1 2                                        00
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    10                                                        10 //Byte order bleibt gleich da im gleichen byte
	//idx 2    0110011110100111101010101011111000010000110100010111001011010110    11                                                        11

	//idx 1    0000010011101110111100101011010010110101110110000110000011001100    000  bit index 1 2 9                                     000
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    100                                                      010
	//idx 3    0100000001010110010111010101000011100111001010110100000000100001    101                              						110
	//idx 2    0110011110100111101010101011111000010000110100010111001011010110    110                                                      011


	//idx 1    0000010011101110111100101011010010110101110110000110000011001100    0000  bit index 1 2 7 9                                 0000
	//idx 4    0000010111010000011110110111110100011110100011011110001110000000    0010                                                    0001
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    1000                                                    0100
	//idx 3    0100000001010110010111010101000011100111001010110100000000100001    1001                              					   1100
	//idx 2    0110011110100111101010101011111000010000110100010111001011010110    1100                                                    0110

	//idx 1    0000010011101110111100101011010010110101110110000110000011001100    00000  bit index 1 2 3 7 9                               00000
	//idx 4    0000010111010000011110110111110100011110100011011110001110000000    00010                                                    00001
	//idx 0    0100000000011111011110101100011110001011110010000000111100011100    10000                                                    01000
	//idx 3    0100000001010110010111010101000011100111001010110100000000100001    10001                              					    11000
	//idx 5    0101100000111100010100000010110010000011001011100000110000000000    10100                                                    01010
	//idx 2    0110011110100111101010101011111000010000110100010111001011010110    11000                                                    01100

	std::vector<uint64_t> valuesToInsert = {
		4620546740167642908ul,   // 0100000000011111011110101100011110001011110010000000111100011100    0
		355488278567739596ul,    // 0000010011101110111100101011010010110101110110000110000011001100    1
		7469126240319926998ul,   // 0110011110100111101010101011111000010000110100010111001011010110    2
		4635995468481642529ul,   // 0100000001010110010111010101000011100111001010110100000000100001    3
		418970542659199878ul,    // 0000010111010000011110110111110100011110100011011110001110000000    4
		6358044926049913402ul,   // 0101100000111100010100000010110010000011001011100000110000000000    5
		5058016125798318033ul,   // 0100011000110001101011101101111000101110011010000000000000000000    6
		2583272014892537200ul,   // 0010001111011001101000000011010111110101111000001001011000000000    7
		9219209713614924562ul,
		7736011505917826031ul,
		4596340717661012313ul,
		4413874586873285858ul,
		5904155143473820934ul,
		3040631852046752166ul,
		4529279813148173111ul,
		3658352497551999605ul,
		1488787817663097441ul,
		8484116316263611556ul,
		4745643133208116498ul,
		8081518017574486547ul,
		5945178879512507902ul,
		4728986788662773602ul,
		840062144447779464ul,
		1682692516156909696ul,
		570275675392078508ul,
		2804578118555336986ul,
		5511269538150904327ul,
		6665263661402689669ul,
		8872308438533970361ul,
		5494304472256329401ul,
		5260777597240341458ul,
		1288265801825883165ul
	};
	testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(valuesToInsert);
}

BOOST_AUTO_TEST_CASE(testSwitchToRandomInSingleNodeRANDOM_8_MASK_8) {
	std::vector<std::string> strings;

	strings.push_back("Ein langer String");
	strings.push_back("Ein langer String2");
	strings.push_back("Ein kurzer String");

	testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t >>(strings);

	std::vector<std::string> stringsReverse;

	stringsReverse.push_back("Ein kurzer String");
	stringsReverse.push_back("Ein langer String");
	stringsReverse.push_back("Ein langer String2");

	testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t >>(stringsReverse);
}

BOOST_AUTO_TEST_CASE(testRandomStringsIntoSingleNodeRANDOM_32_MASK_32) {
	std::vector<std::string> strings;

	strings.push_back("Ein langer String");
	strings.push_back("Ein langer String 123455555555");
	strings.push_back("Ein langer String 1234555555559");
	strings.push_back("Ein langer String 123455555555910");
	strings.push_back("Ein langer String 123455555555911");
	strings.push_back("Ein langer String 1234555555559111");
	strings.push_back("Ein langer String 1234555555559113");
	strings.push_back("Ein langer String 1234555555559114");
	strings.push_back("Ein langer String 12345555555591158");
	strings.push_back("Ein langer String 123455555555911589");
	strings.push_back("Ein langer String 12345555555591158910");
	strings.push_back("Ein langer String 1234555555559115891011");
	strings.push_back("Ein langer String 1234555555559115891012");

	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555910");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559111");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559113");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559114");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891011");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910124");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589101246");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012467");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910124678");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589101246789");
	strings.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012467890");

	testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<4>, uint32_t >>(strings);
}


BOOST_AUTO_TEST_CASE(testLargeBitCompressAndAdd) {
	std::vector<uint64_t> testValues;

	for(uint64_t i=0; i < 32; ++i) {
		testValues.push_back(i);
	}
	testValues.push_back(1ul << 62);
	testCompressAndAdd(testValues);
}

BOOST_AUTO_TEST_CASE(testLargeBitCompressAndAddMoreThan32Entries) {
	std::vector<uint64_t> testValues;

	for(uint64_t i=0; i < 32; ++i) {
		testValues.push_back(1ul << i);
	}
	testValues.push_back(1ul << 62);
	testCompressAndAdd(testValues);
}

BOOST_AUTO_TEST_CASE(testCompressAndAddByShrinkingFrom32BitNodeTo8BitNodeAndExtendingTo16BitNode) {
	std::vector<uint64_t> initialEntries;

	for(uint64_t i=0; i < 32; ++i) {
		initialEntries.push_back(1ul << (i*2));
	}

	for(uint64_t i=0; i < 32; ++i) {
		std::vector<uint64_t> allEntries { initialEntries.begin(), initialEntries.end() };
		allEntries.emplace_back((1ul << (i + 31)) + 1);
		testCompressAndAdd(allEntries);
	}
}

BOOST_AUTO_TEST_CASE(testCompressAndAddByShrinkingFrom32BitNodeTo8BitNodeAndExtendingTo16BitNodeOtherSide) {
	std::vector<uint64_t> initialEntries;

	for(uint64_t i=0; i < 32; ++i) {
		initialEntries.push_back(1ul << (i*2));
	}

	for(uint64_t i=0; i < 32; ++i) {
		std::vector<uint64_t> allEntries { initialEntries.begin(), initialEntries.end() };
		allEntries.emplace_back((i * 2) + 1);
		testCompressAndAdd(allEntries);
	}
}

BOOST_AUTO_TEST_CASE(testCompressAndAddRandomNode) {
	std::vector<std::string> strings;
	std::string prefix { "" };

	for(size_t i = 0u; i < 33; ++i) {
		strings.emplace_back(prefix + "a");
		prefix += " ";
	}

	testCompressAndAdd(hot::testhelpers::stdStringsToCStrings(strings));
}

BOOST_AUTO_TEST_CASE(testSplitAndMergeNode) {
	for(unsigned int firstIndexInUpper = 1; firstIndexInUpper < 32; ++firstIndexInUpper) {
		std::vector<uint64_t> initialEntries;
		for (unsigned int  j = 0; j < 32; ++j) {
			if (j < firstIndexInUpper) {
				initialEntries.push_back(1ul << (j + 1));
			} else {
				initialEntries.push_back((1ul << 62) + (1ul << (j + 1)));
			}
		}

		std::vector<uint64_t> entriesForSplitOnTheLeftSide = std::vector<uint64_t>(initialEntries.begin(), initialEntries.end());
		entriesForSplitOnTheLeftSide.emplace_back(initialEntries[firstIndexInUpper - 1] + 1);

		testSplit(entriesForSplitOnTheLeftSide);
		testMerge(entriesForSplitOnTheLeftSide);

		std::vector<uint64_t> entriesForSplitOnTheRightSide = std::vector<uint64_t>(initialEntries.begin(), initialEntries.end());
		entriesForSplitOnTheRightSide.emplace_back(initialEntries[firstIndexInUpper] + 1);

		testSplit(entriesForSplitOnTheRightSide);
		testMerge(entriesForSplitOnTheRightSide);
	}
}

BOOST_AUTO_TEST_CASE(testRemoveAndAdd) {
	std::vector<uint64_t> initialEntries;
	for(size_t i=32; i < 64; ++i) {
		initialEntries.push_back(i);
	}
	executeTestRemoveAndAdd(initialEntries, 1ul, 65ul);
}

BOOST_AUTO_TEST_CASE(testSplitAndMergeStrings) {
	std::vector<std::string> strings;
	std::string prefix { "" };

	for(size_t i = 0u; i < 33; ++i) {
		strings.emplace_back(prefix + "a");
		prefix += " ";
	}
	//always result in split 32:1
	testSplit(hot::testhelpers::stdStringsToCStrings(strings));
	testMerge(hot::testhelpers::stdStringsToCStrings(strings));
}

BOOST_AUTO_TEST_CASE(testSplitAndMergeStrings2) {
	for(size_t i = 0u; i < 33; ++i) {
		std::vector<std::string> strings;
		std::string infix { "" };
		for(size_t j=0u; j < 33; ++j) {
			std::string prefixString = j < i ? "<" : ">";
			strings.emplace_back(prefixString + infix + "a");
			infix += " ";
		}
		testSplit(hot::testhelpers::stdStringsToCStrings(strings));
		testMerge(hot::testhelpers::stdStringsToCStrings(strings));
	}
}

BOOST_AUTO_TEST_CASE(testGetMostRightBitForEntry8BitMask) {
	std::vector<std::string> initialEntries;

	initialEntries.push_back("Ein langer String");
	initialEntries.push_back("Ein langer String 123455555555");

	std::shared_ptr<const HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>> successiveNode
		= testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(initialEntries);


	BOOST_REQUIRE_EQUAL(successiveNode->getLeastSignificantDiscriminativeBitForEntry(0u), 138);
	BOOST_REQUIRE_EQUAL(successiveNode->getLeastSignificantDiscriminativeBitForEntry(1u), 138);

	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555");

	std::shared_ptr<const HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t>> randomNode1
		= testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t>>(initialEntries);

	BOOST_REQUIRE_EQUAL(randomNode1->getLeastSignificantDiscriminativeBitForEntry(0u), 138);
	BOOST_REQUIRE_EQUAL(randomNode1->getLeastSignificantDiscriminativeBitForEntry(1u), 138);
	BOOST_REQUIRE_EQUAL(randomNode1->getLeastSignificantDiscriminativeBitForEntry(2u), 4);

	//becomes third entry
	initialEntries.push_back("Lorem ipsum blabla");

	std::shared_ptr<const HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t>> randomNode2
		= testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<1>, uint8_t>>(initialEntries);
	BOOST_REQUIRE_EQUAL(randomNode2->getLeastSignificantDiscriminativeBitForEntry(0u), 138);
	BOOST_REQUIRE_EQUAL(randomNode2->getLeastSignificantDiscriminativeBitForEntry(1u), 138);
	BOOST_REQUIRE_EQUAL(randomNode2->getLeastSignificantDiscriminativeBitForEntry(2u), 101);
	BOOST_REQUIRE_EQUAL(randomNode2->getLeastSignificantDiscriminativeBitForEntry(3u), 101);
}

BOOST_AUTO_TEST_CASE(testStringPrefixes) {
	std::vector<std::string> initialEntries = { "fernando@terras.com.bt", "fernando@terras.com" };

	testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>(initialEntries);
}

BOOST_AUTO_TEST_CASE(testGetMostRightBitForEntry16BitMask) {
	std::vector<std::string> initialEntries;

	/**
	 *  correct Order
	 *
	 *  Ein langer String");
	 *  Ein langer String 123455555555");
	 *  Ein langer String 1234555555559");
	 *  Ein langer String 123455555555910");
	 *  Ein langer String 123455555555911");
	 *  Ein langer String 1234555555559111");
	 *  Ein langer String 1234555555559113");
	 *  Ein langer String 1234555555559114");
	 *  Ein langer String 12345555555591158");
	 *  Ein langer String 123455555555911589");
	 *  Ein langer String 12345555555591158910");
	 *  Ein langer String 1234555555559115891011");
	 *  Ein langer String 1234555555559115891012");
	 */

	initialEntries.push_back("Ein langer String 1234555555559111");
	initialEntries.push_back("Ein langer String");
	initialEntries.push_back("Ein langer String 123455555555");
	initialEntries.push_back("Ein langer String 1234555555559");
	initialEntries.push_back("Ein langer String 123455555555911589");
	initialEntries.push_back("Ein langer String 123455555555910");
	initialEntries.push_back("Ein langer String 123455555555911");
	initialEntries.push_back("Ein langer String 1234555555559113");
	initialEntries.push_back("Ein langer String 1234555555559114");
	initialEntries.push_back("Ein langer String 12345555555591158");

	initialEntries.push_back("Ein langer String 12345555555591158910");
	initialEntries.push_back("Ein langer String 1234555555559115891011");
	initialEntries.push_back("Ein langer String 1234555555559115891012");

	std::shared_ptr<const HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<2>, uint16_t>> node
		= testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<2>, uint16_t>>(initialEntries);

	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(0u), 138);
	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(1u), 242);
	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(3u), 263);

	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(11u), 318);
	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(12u), 318);
}

BOOST_AUTO_TEST_CASE(testGetMostRightBitForEntry32BitMask) {
	std::vector<std::string> initialEntries;

	initialEntries.push_back("Ein langer String");
	initialEntries.push_back("Ein langer String 123455555555");
	initialEntries.push_back("Ein langer String 1234555555559");
	initialEntries.push_back("Ein langer String 123455555555910");
	initialEntries.push_back("Ein langer String 123455555555911");
	initialEntries.push_back("Ein langer String 1234555555559111");
	initialEntries.push_back("Ein langer String 1234555555559113");
	initialEntries.push_back("Ein langer String 1234555555559114");
	initialEntries.push_back("Ein langer String 12345555555591158");
	initialEntries.push_back("Ein langer String 123455555555911589");
	initialEntries.push_back("Ein langer String 12345555555591158910");
	initialEntries.push_back("Ein langer String 1234555555559115891011");
	initialEntries.push_back("Ein langer String 1234555555559115891012");

	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555910");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559111");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559113");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559114");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891011");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910124");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589101246");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012467");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 12345555555591158910124678");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 123455555555911589101246789");
	initialEntries.push_back("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, Ein langer String 1234555555559115891012467890");

	std::shared_ptr<const HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<4>, uint32_t>> node
		= testStringValuesForSingleNode<HOTSingleThreadedNode<hot::commons::MultiMaskPartialKeyMapping<4>, uint32_t>>(initialEntries);

	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(0u), 138);
	BOOST_REQUIRE_EQUAL(node->getLeastSignificantDiscriminativeBitForEntry(1u), 242);
}

BOOST_AUTO_TEST_CASE(testCollectEntryDepth) {
	std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> twoEntriesNode = testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>({ 40ul, 2000000ul });
	const std::array<uint8_t, 32> &entryDepths = twoEntriesNode->getEntryDepths();
	BOOST_REQUIRE_EQUAL(entryDepths[0], 1);
	BOOST_REQUIRE_EQUAL(entryDepths[1], 1);
	for(int i=2; i < 32; ++i) {
		BOOST_REQUIRE_EQUAL(entryDepths[i], 0);
	}


	std::shared_ptr<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t> const> fiveEntryNode
		= testIntSingleNode<HOTSingleThreadedNode<hot::commons::SingleMaskPartialKeyMapping, uint8_t>>({ 1, 4, 12, 13, 14 });

	const std::array<uint8_t, 32> &entryDepthsForFiveEntriesNode = fiveEntryNode->getEntryDepths();
	BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[0], 2);
	BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[1], 2);
	BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[2], 3);
	BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[3], 3);
	BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[4], 2);
	for(int i=5; i < 32; ++i) {
		BOOST_REQUIRE_EQUAL(entryDepthsForFiveEntriesNode[i], 0);
	}

	//at least two elements
	for(uint64_t i=1u; i < 32u; ++i) {
		std::vector<uint64_t> rawElementsZeroDeepestElement { 0 };
		std::vector<uint64_t> rawElementsZeroHighestElement { 0 };
		for(uint64_t j=1u; j <= i; ++j) {
			rawElementsZeroDeepestElement.push_back((1ul << 31) >> j); //elements have form 100000, 01000000, 0010000..., 0
			rawElementsZeroHighestElement.push_back((INT32_MAX >> j) << j); //elements have form 0, 1000000000, 11000000, 11100000, 1111000000,..... 11111111
		}

		insertIntoSingleNodeWithoutTypeCheck(rawElementsZeroDeepestElement, [&](auto const & node) {
			const std::array<uint8_t, 32> &entryDepthsForZeroDeepestElement = node->getEntryDepths();
			for(size_t j=1; j < node->getNumberEntries() - 1; ++j) {
				BOOST_REQUIRE_EQUAL(entryDepthsForZeroDeepestElement[j], node->getNumberEntries() - j);
			}
			BOOST_REQUIRE_EQUAL(entryDepthsForZeroDeepestElement[0], node->getNumberEntries() - 1);
			//BOOST_REQUIRE_EQUAL(entryDepthsForZeroDeepestElement[node->getNumberEntries() - 1], node->getNumberEntries() - 2);
		});

		insertIntoSingleNodeWithoutTypeCheck(rawElementsZeroHighestElement, [&](auto const & node) {
			const std::array<uint8_t, 32> &entryDepthsForZeroHighestElement = node->getEntryDepths();
			for(size_t j=0; j < node->getNumberEntries() - 1; ++j) {
				BOOST_REQUIRE_EQUAL(entryDepthsForZeroHighestElement[j], j + 1);
			}
			BOOST_REQUIRE_EQUAL(entryDepthsForZeroHighestElement[node->getNumberEntries() - 1], node->getNumberEntries() - 1);
		});
	}
}


BOOST_AUTO_TEST_CASE(testGetParentBiNodeInformationOfEntry) {

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                              57
	 *                           /    \
	 *                          59     \
	 *                         /   \    \
	 *                        61    \    \
	 *	                    /   \    \    \
	 * Value:              34   36   49   101
	 * Index:               0    1    2    3
	 * DiscBitIndex:       61    61   59   57
	 * DiscBitValue:        0    1    1    1
	 * HasDirectNeigbour:   T    T    F    F
	 */
	std::vector<uint64_t> firstTestValues = {
		0b0100010,  //34
		0b0100100,  //36
		0b0110001,  //49
		0b1100101   //101
	};
	insertIntoSingleNodeWithoutTypeCheck(firstTestValues, [&](auto const & node) {
		hot::commons::BiNodeInformation const & biNodeInformation = node->getParentBiNodeInformationOfEntry(0);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 2u);


		hot::commons::BiNodeInformation const & biNodeInformation1 = node->getParentBiNodeInformationOfEntry(1);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 2u);

		hot::commons::BiNodeInformation const & biNodeInformation2 = node->getParentBiNodeInformationOfEntry(2);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 59u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 3u);

		hot::commons::BiNodeInformation const & biNodeInformation3 = node->getParentBiNodeInformationOfEntry(3);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 57u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 4u);
	});

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                            57
	 *                          /    \
	 *                         /     59
	 *                        /     /   \
	 *                       /     /    61
	 *	                    /     /    /  \
	 * Value:              50   108  113 118
	 * Index:               0    1    2    3
	 * DiscBitIndex:       57    59   61   61
	 * DiscBitValue:        0    0    0    1
	 * HasDirectNeigbour:   F    F    T    T
	 */
	std::vector<uint64_t> secondTestValues = {
		0b0110010,  //50
		0b1101100,  //108
		0b1110001,  //113
		0b1110110   //118
	};
	insertIntoSingleNodeWithoutTypeCheck(secondTestValues, [&](auto const & node) {
		hot::commons::BiNodeInformation const &biNodeInformation = node->getParentBiNodeInformationOfEntry(0);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 57u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 4u);


		hot::commons::BiNodeInformation const &biNodeInformation1 = node->getParentBiNodeInformationOfEntry(1);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 59u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 3u);

		hot::commons::BiNodeInformation const &biNodeInformation2 = node->getParentBiNodeInformationOfEntry(2);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 2u);

		hot::commons::BiNodeInformation const &biNodeInformation3 = node->getParentBiNodeInformationOfEntry(3);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 2u);
	});

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                              56
	 *                            /   \
	 *                          58     \
	 *                        /    \    \
	 *                       /      60   \
	 *	                    /     /   \   \
	 * Value:              50   108  113 118
	 * Index:               0    1    2    3
	 * DiscBitIndex:       57    59   61   61
	 * DiscBitValue:        0    0    0    1
	 * HasDirectNeigbour:   F    T    T    F
	 */
	std::vector<uint64_t> thirdTestValues = {
		0b01010010,  //82
		0b01100100,  //100
		0b01101001,  //105
		0b10101110   //174
	};


	insertIntoSingleNodeWithoutTypeCheck(thirdTestValues, [&](auto const & node) {
		hot::commons::BiNodeInformation const &biNodeInformation = node->getParentBiNodeInformationOfEntry(0);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 58u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 3u);


		hot::commons::BiNodeInformation const &biNodeInformation1 = node->getParentBiNodeInformationOfEntry(1);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 60u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 2u);

		hot::commons::BiNodeInformation const &biNodeInformation2 = node->getParentBiNodeInformationOfEntry(2);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 60u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 2u);

		hot::commons::BiNodeInformation const &biNodeInformation3 = node->getParentBiNodeInformationOfEntry(3);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 56u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 4u);
	});
}

BOOST_AUTO_TEST_CASE(testGetDeletionInformation) {

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                              57
	 *                           /    \
	 *                          59     \
	 *                         /   \    \
	 *                        61    \    \
	 *	                    /   \    \    \
	 * Value:              34   36   49   101
	 * Index:               0    1    2    3
	 * DiscBitIndex:       61    61   59   57
	 * DiscBitValue:        0    1    1    1
	 * HasDirectNeigbour:   T    T    F    F
	 */
	std::vector<uint64_t> firstTestValues = {
		0b0100010,  //34
		0b0100100,  //36
		0b0110001,  //49
		0b1100101   //101
	};
	insertIntoSingleNodeWithoutTypeCheck(firstTestValues, [&](auto const & node) {
		HOTSingleThreadedDeletionInformation const & deletionInformation = node->getDeletionInformation(0);
		hot::commons::BiNodeInformation const & biNodeInformation = deletionInformation.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getIndexOfEntryToRemove(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDirectNeighbourIfAvailable(), node->getPointers() + 1);
		BOOST_REQUIRE_EQUAL(deletionInformation.getIndexOfEntryToReplace(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getCompressionMask(), 0b110u);


		HOTSingleThreadedDeletionInformation const & deletionInformation1 = node->getDeletionInformation(1);
		hot::commons::BiNodeInformation const & biNodeInformation1 = deletionInformation1.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToRemove(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDirectNeighbourIfAvailable(), node->getPointers());
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToReplace(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getCompressionMask(), 0b110u);

		HOTSingleThreadedDeletionInformation const & deletionInformation2 = node->getDeletionInformation(2);
		hot::commons::BiNodeInformation const & biNodeInformation2 = deletionInformation2.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 59u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getIndexOfEntryToRemove(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getCompressionMask(), 0b101u);


		HOTSingleThreadedDeletionInformation const & deletionInformation3 = node->getDeletionInformation(3);
		hot::commons::BiNodeInformation const & biNodeInformation3 = deletionInformation3.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 57u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 4u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getIndexOfEntryToRemove(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getCompressionMask(), 0b011u);

	});

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                            57
	 *                          /    \
	 *                         /     59
	 *                        /     /   \
	 *                       /     /    61
	 *	                    /     /    /  \
	 * Value:              50   108  113 118
	 * Index:               0    1    2    3
	 * DiscBitIndex:       57    59   61   61
	 * DiscBitValue:        0    0    0    1
	 * HasDirectNeigbour:   F    F    T    T
	 */
	std::vector<uint64_t> secondTestValues = {
		0b0110010,  //50
		0b1101100,  //108
		0b1110001,  //113
		0b1110110   //118
	};
	insertIntoSingleNodeWithoutTypeCheck(secondTestValues, [&](auto const & node) {
		HOTSingleThreadedDeletionInformation const & deletionInformation = node->getDeletionInformation(0);
		hot::commons::BiNodeInformation const & biNodeInformation = deletionInformation.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 57u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 4u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getIndexOfEntryToRemove(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation.getCompressionMask(), 0b011u);


		HOTSingleThreadedDeletionInformation const & deletionInformation1 = node->getDeletionInformation(1);
		hot::commons::BiNodeInformation const & biNodeInformation1 = deletionInformation1.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 59u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToRemove(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getCompressionMask(), 0b101u);

		HOTSingleThreadedDeletionInformation const & deletionInformation2 = node->getDeletionInformation(2);
		hot::commons::BiNodeInformation const & biNodeInformation2 = deletionInformation2.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getIndexOfEntryToRemove(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDirectNeighbourIfAvailable(), node->getPointers() + 3);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getIndexOfEntryToReplace(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getCompressionMask(), 0b110u);

		HOTSingleThreadedDeletionInformation const & deletionInformation3 = node->getDeletionInformation(3);
		hot::commons::BiNodeInformation const & biNodeInformation3 = deletionInformation3.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 61u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getIndexOfEntryToRemove(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDirectNeighbourIfAvailable(), node->getPointers() + 2);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getIndexOfEntryToReplace(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getCompressionMask(), 0b110u);
	});

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                              56
	 *                            /   \
	 *                          58     \
	 *                        /    \    \
	 *                       /      60   \
	 *	                    /     /   \   \
	 * Value:              50   108  113 118
	 * Index:               0    1    2    3
	 * DiscBitIndex:       57    59   61   61
	 * DiscBitValue:        0    0    0    1
	 * HasDirectNeigbour:   F    T    T    F
	 */
	std::vector<uint64_t> thirdTestValues = {
		0b01010010,  //82
		0b01100100,  //100
		0b01101001,  //105
		0b10101110   //174
	};


	insertIntoSingleNodeWithoutTypeCheck(thirdTestValues, [&](auto const & node) {
		HOTSingleThreadedDeletionInformation const & deletionInformation = node->getDeletionInformation(0);
		hot::commons::BiNodeInformation const & biNodeInformation = deletionInformation.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitIndex, 58u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mDiscriminativeBitMask, 0b010u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.mRight.mNumberEntriesInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation.getTotalNumberEntries(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getIndexOfEntryToRemove(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation.getCompressionMask(), 0b101u);


		HOTSingleThreadedDeletionInformation const & deletionInformation1 = node->getDeletionInformation(1);
		hot::commons::BiNodeInformation const & biNodeInformation1 = deletionInformation1.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 60u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToRemove(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDiscriminativeBitValueForEntry(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDirectNeighbourIfAvailable(), node->getPointers() + 2);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToReplace(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getCompressionMask(), 0b110u);

		HOTSingleThreadedDeletionInformation const & deletionInformation2 = node->getDeletionInformation(2);
		hot::commons::BiNodeInformation const & biNodeInformation2 = deletionInformation2.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitIndex, 60u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mDiscriminativeBitMask, 0b001u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mFirstIndexInRange, 2u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation2.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getIndexOfEntryToRemove(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getDirectNeighbourIfAvailable(), node->getPointers() + 1);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getIndexOfEntryToReplace(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation2.getCompressionMask(), 0b110u);

		HOTSingleThreadedDeletionInformation const & deletionInformation3 = node->getDeletionInformation(3);
		hot::commons::BiNodeInformation const & biNodeInformation3 = deletionInformation3.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitIndex, 56u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mDiscriminativeBitMask, 0b100u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mLeft.mNumberEntriesInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mFirstIndexInRange, 3u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation3.getTotalNumberEntries(), 4u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getIndexOfEntryToRemove(), 3u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation3.hasDirectNeighbour(), false);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getDirectNeighbourIfAvailable(), (HOTSingleThreadedChildPointer*) nullptr);
		BOOST_REQUIRE_EQUAL(deletionInformation3.getCompressionMask(), 0b011u);
	});

	/* The resulting structure of the tree created for the given values will be
	 *
	 *                              59
	 *                            /    \
	 *                           /      \
	 *                         62       62
	 *                        /  \     /  \
	 * Value:              13    14   21  22
	 * Index:               0    1    2    3
	 * DiscBitIndex:       62    62   62   62
	 * DiscBitValue:        0    1    0    1
	 * HasDirectNeigbour:   T    T    T    T
	 */
	std::vector<uint64_t> fourthTestValues = {
		0b00001101,  //13
		0b00001110,  //14
		0b00010101,  //21
		0b00010110   //22
	};


	insertIntoSingleNodeWithoutTypeCheck(fourthTestValues, [&](auto const & node) {
		HOTSingleThreadedDeletionInformation const & deletionInformation1 = node->getDeletionInformation(1);
		hot::commons::BiNodeInformation const & biNodeInformation1 = deletionInformation1.getAffectedBiNode();
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitIndex, 62u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mDiscriminativeBitMask, 0b01u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mFirstIndexInRange, 0u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mLeft.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mFirstIndexInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.mRight.mNumberEntriesInRange, 1u);
		BOOST_REQUIRE_EQUAL(biNodeInformation1.getTotalNumberEntries(), 2u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToRemove(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDiscriminativeBitValueForEntry(), 1u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.hasDirectNeighbour(), true);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getDirectNeighbourIfAvailable(), node->getPointers());
		BOOST_REQUIRE_EQUAL(deletionInformation1.getIndexOfEntryToReplace(), 0u);
		BOOST_REQUIRE_EQUAL(deletionInformation1.getCompressionMask(), 0b11u);

		BOOST_REQUIRE_EQUAL(node->getDeletionInformation(0).getAffectedBiNode().mDiscriminativeBitMask, 0b01u);
		BOOST_REQUIRE_EQUAL(node->getDeletionInformation(2).getAffectedBiNode().mDiscriminativeBitMask, 0b01u);
		BOOST_REQUIRE_EQUAL(node->getDeletionInformation(3).getAffectedBiNode().mDiscriminativeBitMask, 0b01u);
	});
}

BOOST_AUTO_TEST_SUITE_END()

}}
