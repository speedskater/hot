//
//  Created by Robert Binna on 23.12.14.
//
//

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <array>
#include <thread>
#include <mutex>

#include <bitset>
#include <set>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <hot/rowex/HOTRowex.hpp>

#include <hot/testhelpers/PartialKeyMappingTestHelper.hpp>
#include <hot/testhelpers/SampleTriples.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/PairPointerKeyExtractor.hpp>
#include <idx/contenthelpers/KeyComparator.hpp>
#include <idx/contenthelpers/OptionalValue.hpp>

#include <idx/utils/8ByteDatFileIO.hpp>
#include <idx/utils/RandomRangeGenerator.hpp>

#include "hot/rowex/ConcurrentTestHelper.hpp"
#include "hot/rowex/TreeTestHelper.hpp"
#include "hot/rowex/StringTestData.hpp"

namespace hot { namespace rowex {

using HOTRowexUint64 = hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
using CStringTrieType = hot::rowex::HOTRowex<const char*, idx::contenthelpers::IdentityKeyExtractor>;

template<typename ValueType>
std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> insertWithoutCheck(std::vector<ValueType> const &valuesToInsert) {
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> cobTrie = std::make_shared<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();
	for (size_t i = 0u; i < valuesToInsert.size(); ++i) {
		cobTrie->insert(valuesToInsert[i]);
	}

	return cobTrie;
	//std::cout << "Checked Integrity" << std::endl;
}

template<typename ValueType> std::thread createInsertThread(std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> cobTrie, size_t threadId, size_t numberThreads, std::vector<ValueType> const * valuesToInsert);

std::thread createCounterBasedInsertThread(std::shared_ptr<hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>> cobTrie, size_t threadId, size_t numberValues);

template<typename ValueType>
std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> testParallel(
	std::vector<ValueType> const &valuesToInsert) {
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> cobTrie = std::make_shared<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();
	std::shared_ptr<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>> unsynchronizedHOT = std::make_shared<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();

	for (size_t i = 0u; i < valuesToInsert.size(); ++i) {
		unsynchronizedHOT->insert(valuesToInsert[i]);
	}

	const unsigned numberThreads = 8; //std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	for(size_t threadId = 0; threadId < numberThreads; ++threadId) {
		threads.push_back(createInsertThread(cobTrie, threadId, numberThreads, &valuesToInsert));
	}
	for(std::thread & thread : threads) {
		thread.join();
	}

	checkConsistency(cobTrie, unsynchronizedHOT, valuesToInsert);

	return cobTrie;
}


template<typename ValueType> std::thread createInsertThread(std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> cobTrie, size_t threadId, unsigned numberThreads, std::vector<ValueType> const * valuesToInsert) {
	return std::thread([=]{
		size_t totalNumberValuesToInsert = valuesToInsert->size();
		for(size_t i = threadId; i < totalNumberValuesToInsert; i += numberThreads)
		{
			//std::cout << "i" << i << std::endl;
			cobTrie->insert((*valuesToInsert)[i]);
		}
		//std::cout << "thread with id " << threadId << " finished" << std::endl;
	});

}

std::thread createCounterBasedInsertThread(std::shared_ptr<hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>> cobTrie, size_t threadId, size_t numberValues, std::vector<uint64_t>* insertedValues) {
	return std::thread([=]{
		for(size_t i = 0; i < numberValues; ++i)
		{
			uint64_t timerBasedValue = (rdtsc() << 6) + threadId;
			insertedValues->push_back(timerBasedValue);
			//std::cout << "i" << i << std::endl;
			cobTrie->insert(timerBasedValue);
		}
		//std::cout << "thread with id " << threadId << " finished" << std::endl;
	});
}

template<typename ValueType>
std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> testSerial(
	std::vector<ValueType> const &valuesToInsert) {
	using HotType = hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>;
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> cobTrie = std::make_shared<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();
	std::shared_ptr<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>> unsynchronizedHOT = std::make_shared<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();

	using KeyType = typename hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>::KeyType;

	//std::cout << "Starting insert" << std::endl;
	for (size_t i = 0u; i < valuesToInsert.size(); ++i) {
		KeyType key = valuesToInsert[i];
		//int failingValue = -1;
		//int failingValue = 24614;
		//int failingValue = 30;

		/*if(i == failingValue) {
			typename KeyHelper::KeyType failingRawValue = KeyHelper::extract(valuesToInsert[failingValue]);
			std::cout << "vorher" << std::endl;

			hot::rowex::ChildPointer previousNodePointer = cobTrie.getNodeAtPath({ });
			hot::rowex::SuccessiveSIMDCobTrieNode<uint8_t> const *previousNode
				= (const hot::rowex::SuccessiveSIMDCobTrieNode<uint8_t> *) previousNodePointer.getNode();
			uint8_t usedSearchMask = previousNode->extractBytes(failingRawValue.data());
			std::cout << "Search Mask for failing vlaue :: ";
			std::bitset<sizeof(usedSearchMask) * 8> usedSearchMaskBits(usedSearchMask);
			std::cout << usedSearchMaskBits << " with mapping ";
			hot::rowex::printMaskWithMapping(usedSearchMask, previousNode->getExtractionMaskToEntriesMasksMapping());
			std::cout << std::endl;
			printUint64NodeChildPointer(previousNodePointer);

			//Print node for path 1/0/16/20 cobTrie befor and after insert
			std::cout << "inserting failingValue " << valuesToInsert[failingValue] << std::endl;
		}*/
		if (i == (valuesToInsert.size() - 1)) {
			BOOST_REQUIRE_MESSAGE(cobTrie->find(key) == cobTrie->end(),
								  "element has not been inserted yet, hence it must no be contained in the trie");
		}

		//size_t previousNumberReturnToPool = HOTRowexNodeBase::getNumberReturnToPool();

		//if(i >= 1048575) {
		//	std::cout << "i :: " << i << std::endl;
		//}

		cobTrie->insert(valuesToInsert[i]);
		unsynchronizedHOT->insert(valuesToInsert[i]);

		/*size_t intermediateNumberReturnToPool = HOTRowexNodeBase::getNumberReturnToPool();

		unsynchronizedHOT->insert(valuesToInsert[i]);

		size_t finalNumberReturnToPool = HOTRowexNodeBase::getNumberReturnToPool();

		BOOST_REQUIRE_EQUAL(intermediateNumberReturnToPool - previousNumberReturnToPool,
							finalNumberReturnToPool - intermediateNumberReturnToPool);*/

		//std::cout << "i :: " << i << " height :: " << cobTrie->mRoot.getHeight() << std::endl;


		//BOOST_REQUIRE_EQUAL(cobTrie->mRoot.getHeight(), unsynchronizedHOT->mRoot.getHeight());

		if (false) {
			std::cout << "i :: " << i << valuesToInsert[i] << std::endl;
			bool isValid = isSubTreeValid<ValueType, idx::contenthelpers::IdentityKeyExtractor>(&cobTrie->mRoot);
			BOOST_REQUIRE(isValid);
			//BOOST_REQUIRE_EQUAL(cobTrie->mRoot.getHeight(), unsynchronizedHOT->mRoot.getHeight());

			//std::cout << "i :: " << i << std::endl;
			std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> temporarySet(
				valuesToInsert.begin(), valuesToInsert.begin() + i + 1);
			std::vector<ValueType> sortedValues(temporarySet.begin(), temporarySet.end());

			size_t numberValues = sortedValues.size();

			for (size_t j = 0u; j < numberValues; ++j) {
				idx::contenthelpers::OptionalValue<ValueType> result = cobTrie->lookup(sortedValues[j]);

				bool found = result.mIsValid && (result.mValue == sortedValues[j]);
				if (found == false) {
					std::cout << "j :: " << j << " => " << found << std::endl;
				}
				BOOST_REQUIRE(found);
			}


			BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie->begin(), cobTrie->end(), sortedValues.begin(), sortedValues.end());

			size_t k = 0u;
			for (typename HotType::const_iterator it = cobTrie->begin(); it != cobTrie->end(); ++it) {
				BOOST_REQUIRE_EQUAL(*it, sortedValues[k]);
				const typename HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>::const_iterator &searchResult = cobTrie->find(
					sortedValues[k]);
				BOOST_REQUIRE_EQUAL(*searchResult, sortedValues[k]);
				++k;
			}

			for (size_t j = 1u; j < numberValues; ++j) {
				KeyType rawSearchValue = sortedValues[j];

				BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie->find(rawSearchValue), cobTrie->end(), sortedValues.begin() + j,
												sortedValues.end());
				BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie->lower_bound(rawSearchValue), cobTrie->end(),
												sortedValues.begin() + j, sortedValues.end());
				BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie->upper_bound(rawSearchValue), cobTrie->end(),
												sortedValues.begin() + j + 1, sortedValues.end());
				BOOST_REQUIRE(
					cobTrie->scan(rawSearchValue, numberValues - j - 1).compliesWith({true, *sortedValues.rbegin()}));
			}
		}

		/*if(i == failingValue) {
			typename Uint64KeyHelper::KeyType failingRawValue = Uint64KeyHelper::extract(valuesToInsert[failingValue]);
			std::cout << "nachher" << std::endl;
			hot::rowex::ChildPointer newNodePointer = cobTrie.getNodeAtPath({ });
			hot::rowex::SuccessiveSIMDCobTrieNode<uint8_t> const *newNode
				= (const hot::rowex::SuccessiveSIMDCobTrieNode<uint8_t> *) newNodePointer.getNode();
			uint8_t usedSearchMask = newNode->extractBytes(failingRawValue.data());
			std::cout << "Search Mask for failing vlaue :: ";
			std::bitset<sizeof(usedSearchMask) * 8> usedSearchMaskBits(usedSearchMask);
			std::cout << usedSearchMaskBits << " with mapping ";
			hot::rowex::printMaskWithMapping(usedSearchMask, newNode->getExtractionMaskToEntriesMasksMapping());
			std::cout << std::endl;

			printUint64NodeChildPointer(newNodePointer);
			std::cout << "printed node" << std::endl;
			sleep(5);
			//Print node for path 1/0/16/20 cobTrie befor and after insert
		}*/

		/*if(i == failingValue) {
			std::cout << "i :: " << i << "searching values" << std::endl;
			for (int j = 0; j <= i; ++j) {
				typename Uint64KeyHelper::KeyType rawSearchValue = Uint64KeyHelper::extract(valuesToInsert[j]);
				uint64_t foundValue = Uint64KeyHelper::extractValue(cobTrie.lookup(rawSearchValue.data()));
				//BOOST_REQUIRE_EQUAL(foundValue, valuesToInsert[j]);
				found = found && foundValue == valuesToInsert[j];
				if (!found) {
					std::cout << "j :: " << j << " => " << found << std::endl;
				}
				BOOST_REQUIRE(found);
			}

			std::cout << "Starting to check integrity" << std::endl;
			BOOST_REQUIRE(cobTrie.mRoot.isSubTreeValid<Uint64KeyHelper>());
			std::cout << "Checked Integrity" << std::endl;

			BOOST_REQUIRE(found);
		}*/

	}

	checkConsistency(cobTrie, unsynchronizedHOT, valuesToInsert);
	return cobTrie;
}

BOOST_AUTO_TEST_SUITE(HOTRowexTest)


BOOST_AUTO_TEST_CASE(testSequentialValuesWithSplitTwoLevel) {
	std::vector<uint64_t> valuesToInsert;
	int numberEntries = 32 * 32;

	for (int i = 0; i < numberEntries; ++i) {
		valuesToInsert.push_back(i);
	}

	std::shared_ptr<HOTRowexUint64> trie = testSerial(valuesToInsert);
	BOOST_REQUIRE_EQUAL(trie->mRoot.getHeight(), 2);

	for(int i = 0; i < 100; ++i) {
		//std::cout << "======================= RUN :: " << i << std::endl << std::flush;
		BOOST_CHECK_EQUAL(testParallel(valuesToInsert)->mRoot.getHeight(), 2);
	}
}

constexpr size_t mixedWorkloadTestSize = 100000u;

BOOST_AUTO_TEST_CASE(testMixedWorkloadLookupAndInsertOfTriples) {
	std::vector<uint64_t> triples = hot::testhelpers::getSampleTriples();
	size_t numberTriples = triples.size();
	executeMixedWorkloadLookupScanAndInsertTest<uint64_t, VectorBasedValueGenerator<uint64_t>>(numberTriples, triples);
}

BOOST_AUTO_TEST_CASE(testMixedWorkloadLookupAndInsertOfStrings) {
	std::vector<const char*> const & longStrings = getLongCStrings();
	size_t numberValues = longStrings.size();
	executeMixedWorkloadLookupScanAndInsertTest<const char*, VectorBasedValueGenerator<const char*>>(numberValues, longStrings);
}

BOOST_AUTO_TEST_CASE(testMixedWorkloadLookupAndInsertOfRandomValues) {
	executeMixedWorkloadLookupScanAndInsertTest<uint64_t, VectorBasedValueGenerator<uint64_t>>(mixedWorkloadTestSize,
																							   getRandomNumbers(
																								   mixedWorkloadTestSize).second);
}

BOOST_AUTO_TEST_CASE(testMixedWorkloadLookupAndInsertOfTimerBasedIncreasingValues) {
	executeMixedWorkloadLookupScanAndInsertTest<uint64_t, TimerBasedIncreasingIdGenerator>(mixedWorkloadTestSize);
}

BOOST_AUTO_TEST_CASE(testMixedWorkloadLookupAndInsertOfTimerBasedDecreasingValues) {
	executeMixedWorkloadLookupScanAndInsertTest<uint64_t, TimerBasedDecreasingIdGenerator>(mixedWorkloadTestSize);
}



BOOST_AUTO_TEST_CASE(testParallelAggressiveCounterBasedSequential) {
	for(size_t i=0; i < 5; ++i) {
		size_t numberEntriesToInsertPerThread = 10000;

		std::shared_ptr<hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>> cobTrie = std::make_shared<hot::rowex::HOTRowex<uint64_t, idx::contenthelpers::IdentityKeyExtractor>>();
		std::shared_ptr<hot::singlethreaded::HOTSingleThreaded<uint64_t, idx::contenthelpers::IdentityKeyExtractor>> unsynchronizedHOT = std::make_shared<hot::singlethreaded::HOTSingleThreaded<uint64_t, idx::contenthelpers::IdentityKeyExtractor>>();

		constexpr size_t numberThreads = 8; //std::thread::hardware_concurrency();
		std::array<std::vector<uint64_t>, numberThreads> allInsertedValues;

		std::vector<std::thread> threads;
		for (size_t threadId = 0; threadId < numberThreads; ++threadId) {
			threads.push_back(createCounterBasedInsertThread(cobTrie, threadId, numberEntriesToInsertPerThread, allInsertedValues.data() + threadId));
		}

		for (size_t threadId = 0; threadId < numberThreads; ++threadId) {
			threads[threadId].join();
		}

		std::vector<uint64_t> insertedValues;

		for(std::vector<uint64_t> insertedValuesForASingleThread: allInsertedValues) {
			for (uint64_t const & insertedValue : insertedValuesForASingleThread) {
				insertedValues.push_back(insertedValue);
				BOOST_REQUIRE(unsynchronizedHOT->insert(insertedValue));
			}
		}

		checkConsistency(cobTrie, unsynchronizedHOT, insertedValues);
	}
}



BOOST_AUTO_TEST_CASE(testSequentialValuesWithSplitThreeLevel) {
	std::vector<uint64_t> valuesToInsert;


	int numberEntries = 32 * 32 * 32;

	for (int i = 0; i < numberEntries; ++i) {
		valuesToInsert.push_back(i);
	}

	testSerial(valuesToInsert);
	for(int i = 0; i < 10; ++i) {
		BOOST_REQUIRE_EQUAL(testParallel(valuesToInsert)->mRoot.getHeight(), 3);
	}
}

BOOST_AUTO_TEST_CASE(testSequentialValuesWithSplitThreeLevelsReverse) {
	std::vector<uint64_t> valuesToInsert;

	int numberEntries = 32 * 32 * 32;

	for (int i = numberEntries - 1; i >= 0; --i) {
		valuesToInsert.push_back(i);
	}

	testSerial(valuesToInsert);

	for(int i = 0; i < 2; ++i) {
		BOOST_REQUIRE_EQUAL(testParallel(valuesToInsert)->mRoot.getHeight(), 3);
	}
}

BOOST_AUTO_TEST_CASE(testRandomValues) {
	std::vector<uint64_t> valuesToInsert = getRandomNumbers(100000).second;
	testSerial(valuesToInsert);

	for(int i = 0; i < 2; ++i) {
		testParallel(valuesToInsert);
	}
}

BOOST_AUTO_TEST_CASE(testBoundsInteger) {
	std::set<uint64_t> sortedValues;
	idx::utils::RandomRangeGenerator<uint64_t> rnd{12344567, 0, INT64_MAX};

	unsigned int numberValues = 100000;

	for (size_t i = 0u; i < numberValues; ++i) {
		sortedValues.insert(rnd());
	}

	std::vector<uint64_t> valuesToInsert;
	std::vector<uint64_t> valuesToLeaveOut;

	uint64_t valueBeforeAll;
	uint64_t valueAfterAll;
	size_t index = 0;
	for(auto value : sortedValues) {
		if(index == 0) {
			valueBeforeAll = value;
		} else if(index == (sortedValues.size() - 1)) {
			valueAfterAll = value;
		} else if((index%2) == 1) {
			valuesToInsert.push_back(value);
		} else {
			valuesToLeaveOut.push_back(value);
		}
		++index;
	}

	std::shared_ptr<HOTRowexUint64> trie = testSerial(valuesToInsert);

	std::set<uint64_t> redBlackTree(valuesToInsert.begin(), valuesToInsert.end());

	const typename HOTRowexUint64::const_iterator &lowerBound = trie->lower_bound(valueAfterAll);
		
	BOOST_REQUIRE_MESSAGE(lowerBound == trie->end(), "Lower bound for value which is after all values is the end");
	BOOST_REQUIRE_MESSAGE(trie->upper_bound(valueAfterAll) == trie->end(), "Upper bound for a value which is after all values is the end");

	BOOST_REQUIRE_MESSAGE(*trie->lower_bound(valueBeforeAll) == valuesToInsert[0], "Lower bound for value which is before all values is the first value");
	BOOST_REQUIRE_MESSAGE(*trie->upper_bound(valueBeforeAll) == valuesToInsert[0], "Upper bound for a value which is before all values is the first value");



	BOOST_REQUIRE_EQUAL(*trie->lower_bound(valueBeforeAll), *redBlackTree.lower_bound(valueBeforeAll));
	BOOST_REQUIRE_MESSAGE(redBlackTree.lower_bound(valueAfterAll) == redBlackTree.end(), "Defined behavior for lower bound if value after all is searched");

	BOOST_REQUIRE_EQUAL(*trie->upper_bound(valueBeforeAll), *redBlackTree.upper_bound(valueBeforeAll));
	BOOST_REQUIRE_MESSAGE(redBlackTree.upper_bound(valueAfterAll) == redBlackTree.end(), "Defined behavior for upper bound if value after all is searched");

	for(size_t i=2; i < (valuesToLeaveOut.size() - 2); ++i) {
		BOOST_REQUIRE_EQUAL(*trie->lower_bound(valuesToLeaveOut[i]), *redBlackTree.lower_bound(valuesToLeaveOut[i]));
		BOOST_REQUIRE_EQUAL(*trie->upper_bound(valuesToLeaveOut[i]), *redBlackTree.upper_bound(valuesToLeaveOut[i]));
	}

	BOOST_REQUIRE_EQUAL_COLLECTIONS(
		trie->lower_bound(valuesToLeaveOut[4]), trie->upper_bound(valuesToLeaveOut[valuesToLeaveOut.size() - 2]),
		redBlackTree.lower_bound(valuesToLeaveOut[4]), redBlackTree.upper_bound(valuesToLeaveOut[valuesToLeaveOut.size() - 2])
	);
}



BOOST_AUTO_TEST_CASE(testTriples) {
	std::vector<uint64_t> valuesToInsert = hot::testhelpers::getSampleTriples();
	testSerial(valuesToInsert);

	for(int i = 0; i < 2; ++i) {
		testParallel(valuesToInsert);
	}
}

BOOST_AUTO_TEST_CASE(testWithLongStringsAndNodeSplit) {
	std::vector<std::string> const &strings = getLongStrings();
	testSerial(hot::testhelpers::stdStringsToCStrings(strings));

	for(int i = 0; i < 10; ++i) {
		testParallel(hot::testhelpers::stdStringsToCStrings(strings));
	}
}

BOOST_AUTO_TEST_CASE(testBoundsWithLongStringsAndIterate) {
	std::vector<std::string> const &strings = getLongStrings();
	std::set<std::string> sortedStrings { strings.begin(), strings.end() };

	std::vector<std::string> valuesToInsert;
	std::vector<std::string> valuesToLeaveOut;

	std::string valueBeforeAll;
	std::string valueAfterAll;
	size_t index = 0;
	for(auto value : sortedStrings) {
		if(index == 0) {
			valueBeforeAll = value;
		} else if(index == (sortedStrings.size() - 1)) {
			valueAfterAll = value;
		} else if((index%2) == 1) {
			valuesToInsert.push_back(value);
		} else {
			valuesToLeaveOut.push_back(value);
		}
		++index;
	}

	std::vector<char const *> cStrings = hot::testhelpers::stdStringsToCStrings(valuesToInsert);
	std::vector<char const *> cStringsToLeaveOut = hot::testhelpers::stdStringsToCStrings(valuesToLeaveOut);
	std::shared_ptr<CStringTrieType> trie = testSerial(cStrings);

	std::set<char const *, typename idx::contenthelpers::KeyComparator<char const *>::type> redBlackTree(cStrings.begin(), cStrings.end());

	const CStringTrieType::const_iterator &lowerBound = trie->lower_bound(valueAfterAll.c_str());

	BOOST_REQUIRE_MESSAGE(lowerBound == trie->end(), "Lower bound for value which is after all values is the end");
	BOOST_REQUIRE_MESSAGE(trie->upper_bound(valueAfterAll.c_str()) == trie->end(), "Upper bound for a value which is after all values is the end");

	BOOST_REQUIRE_MESSAGE(*trie->lower_bound(valueBeforeAll.c_str()) == cStrings[0], "Lower bound for value which is before all values is the first value");
	BOOST_REQUIRE_MESSAGE(*trie->upper_bound(valueBeforeAll.c_str()) == cStrings[0], "Upper bound for a value which is before all values is the first value");

	BOOST_REQUIRE_EQUAL(*trie->lower_bound(valueBeforeAll.c_str()), *redBlackTree.lower_bound(valueBeforeAll.c_str()));
	BOOST_REQUIRE_MESSAGE(redBlackTree.lower_bound(valueAfterAll.c_str()) == redBlackTree.end(), "Defined behavior for lower bound if value after all is searched");

	BOOST_REQUIRE_EQUAL(*trie->upper_bound(valueBeforeAll.c_str()), *redBlackTree.upper_bound(valueBeforeAll.c_str()));
	BOOST_REQUIRE_MESSAGE(redBlackTree.upper_bound(valueAfterAll.c_str()) == redBlackTree.end(), "Defined behavior for upper bound if value after all is searched");

	for(size_t i=2; i < (valuesToLeaveOut.size() - 2); ++i) {
		BOOST_REQUIRE_EQUAL(*trie->lower_bound(cStringsToLeaveOut[i]), *redBlackTree.lower_bound(cStringsToLeaveOut[i]));
		BOOST_REQUIRE_EQUAL(*trie->upper_bound(cStringsToLeaveOut[i]), *redBlackTree.upper_bound(cStringsToLeaveOut[i]));
	}

	BOOST_REQUIRE_EQUAL_COLLECTIONS(
		trie->lower_bound(cStringsToLeaveOut[4]), trie->upper_bound(cStringsToLeaveOut[cStringsToLeaveOut.size() - 2]),
		redBlackTree.lower_bound(cStringsToLeaveOut[4]), redBlackTree.upper_bound(cStringsToLeaveOut[cStringsToLeaveOut.size() - 2])
	);
}

BOOST_AUTO_TEST_CASE(testStringPrefixes) {
	std::vector<std::string> strings = { "fernando@terras.com.bt", "fernando@terras.com" };

	testSerial(hot::testhelpers::stdStringsToCStrings(strings));
	testParallel(hot::testhelpers::stdStringsToCStrings(strings));
}

BOOST_AUTO_TEST_CASE(testEmptyIterator) {
	HOTRowexUint64 cobTrie;

	BOOST_REQUIRE(cobTrie.begin() == cobTrie.end());
}

BOOST_AUTO_TEST_CASE(testSingleElementIterator) {
	HOTRowexUint64 cobTrie;
	cobTrie.insert(42u);
	std::array<uint64_t, 1> expectedValues = { 42u };
	BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie.begin(), cobTrie.end(), expectedValues.begin(), expectedValues.end());
}

BOOST_AUTO_TEST_CASE(testFindOnEmptyTrie) {
	HOTRowexUint64 cobTrie;
	BOOST_REQUIRE_MESSAGE(cobTrie.find(42u) == cobTrie.end(), "Find on empty trie must return the end iterator");
}

BOOST_AUTO_TEST_CASE(testFindElementNotInTrie) {
	HOTRowexUint64 cobTrie;

	cobTrie.insert(41u);
	cobTrie.insert(43u);

	BOOST_REQUIRE_MESSAGE(cobTrie.find(40u) == cobTrie.end(), "Cannot lookup element which is not contained.");
	BOOST_REQUIRE_MESSAGE(cobTrie.find(42u) == cobTrie.end(), "Cannot lookup element which is not contained.");
	BOOST_REQUIRE_MESSAGE(cobTrie.find(44u) == cobTrie.end(), "Cannot lookup element which is not contained.");
}

BOOST_AUTO_TEST_CASE(testUpsert) {
	hot::rowex::HOTRowex<std::pair<uint64_t, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor> cobTrie;

	std::vector<std::pair<uint64_t, uint64_t>> initialValues {
		{ 41u, 3u },
		{ 43u, 5u },
		{ 55u, 9u },
		{ 59u, 13u },
		{ 62u, 2u },
		{ 69u, 7u },
		{ 105u, 44u },
		{ 120u, 1200u },
		{ 257u, 33u },
	};

	std::vector<std::pair<uint64_t , uint64_t>*> pointerValues;

	for(std::pair<uint64_t, uint64_t> & value : initialValues) {
		pointerValues.push_back(&value);
		cobTrie.insert(&value);
	}

	std::pair<uint64_t, uint64_t> newValue { 120u, 42u };

	BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie.begin(), cobTrie.end(), pointerValues.begin(), pointerValues.end());
	const idx::contenthelpers::OptionalValue<std::pair<uint64_t, uint64_t> *> &previousValue = cobTrie.upsert(&newValue);

	pointerValues[7] = &newValue;
	BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie.begin(), cobTrie.end(), pointerValues.begin(), pointerValues.end());

	BOOST_REQUIRE(previousValue.compliesWith({ true, &initialValues[7] }));
}


BOOST_AUTO_TEST_SUITE_END()

}}