#ifndef __HOT__CONCURRENT_TEST_HELPER__
#define __HOT__CONCURRENT_TEST_HELPER__

#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <utility>
#include <sstream>

#include <tbb/tbb_allocator.h> // zero_allocator defined here
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/task_arena.h>
#include <tbb/task_group.h>

#include <idx/contenthelpers/ContentEquals.hpp>
#include <idx/contenthelpers/KeyComparator.hpp>

#include <hot/rowex/HOTRowex.hpp>
#include <hot/singlethreaded/HOTSingleThreaded.hpp>

#include "hot/rowex/TreeTestHelper.hpp"

namespace hot { namespace rowex {

template<typename ValueType>
void checkConsistency(
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> const &cobTrie,
	std::shared_ptr<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>> const &unsynchronizedHOT,
	std::vector<ValueType> const &valuesToInsert
) {
	using KeyType = typename hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>::KeyType;
	//BOOST_REQUIRE_EQUAL(HOTRowexNodeBase::getNumberActiveLocks(), 0u);
	bool heightsMatch = (cobTrie->mRoot.isEmpty() && unsynchronizedHOT->mRoot.isUnused()) || (cobTrie->mRoot.getHeight() == unsynchronizedHOT->mRoot.getHeight());
	if (!heightsMatch) {
		std::cout << "Heights do not match!" << std::endl;
	}
	BOOST_CHECK_EQUAL(cobTrie->mRoot.getHeight(), unsynchronizedHOT->mRoot.getHeight());


	bool found = true;

	//works only because key == value
	std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> temporarySet(
		valuesToInsert.begin(), valuesToInsert.end());
	std::vector<ValueType> sortedValues(temporarySet.begin(), temporarySet.end());

	size_t numberValues = sortedValues.size();

	for (size_t j = 0u; j < numberValues; ++j) {
		idx::contenthelpers::OptionalValue<ValueType> result = cobTrie->lookup(sortedValues[j]);
		bool foundEntry = result.mIsValid && (result.mValue == sortedValues[j]);

		found = found & foundEntry;
		if (!foundEntry) {
			std::cout << "j :: " << j << " searched <" << sortedValues[j] << "> but found <" << result.mValue << ">"
					  << std::endl;
		}
	}

	if (!found) {
		std::cout << "Values are: ";
		for (ValueType const &value : *cobTrie) {
			std::cout << value << ", ";
		}
		std::cout << std::endl;
	}

	if (numberValues < 1000) {
		for (size_t j = 0u; j < numberValues; ++j) {
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
	} else {
		for (size_t j = 0u; j < numberValues; j += (numberValues / 100)) {
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

	//std::cout << "Starting to check integrity" << std::endl;

	BOOST_REQUIRE_EQUAL_COLLECTIONS(cobTrie->begin(), cobTrie->end(), sortedValues.begin(), sortedValues.end());

	bool subtreeValid = isSubTreeValid<ValueType, idx::contenthelpers::IdentityKeyExtractor>(&(cobTrie->mRoot));
	BOOST_REQUIRE(subtreeValid);
	BOOST_REQUIRE(found);
}

uint64_t rdtsc() {
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return static_cast<uint64_t>(lo) | (static_cast<uint64_t>(hi) << 32);
}

struct ThreadIdHashCompare {
	static std::hash<std::thread::id> hasher;

	static size_t hash(const std::thread::id &threadId) {
		return hasher(threadId);
	}

	//! True if strings are equal
	static bool equal(const std::thread::id &x, const std::thread::id &y) {
		return x == y;
	}
};

std::hash<std::thread::id> ThreadIdHashCompare::hasher;

class TimerBasedIncreasingIdGenerator {

public:
	uint64_t generateValue(size_t /* index */, size_t shortThreadId) const {
		assert(shortThreadId < (1 << 6));
		uint64_t increasingId = ((rdtsc() << 6)  + shortThreadId ) & static_cast<uint64_t>(INT64_MAX);
		return increasingId + shortThreadId;
	}
};

class TimerBasedDecreasingIdGenerator {

private:
	TimerBasedIncreasingIdGenerator mIncreasingGenerator;

public:
	uint64_t generateValue(size_t index, size_t shortThreadId) const {
		uint64_t increasingId = mIncreasingGenerator.generateValue(index, shortThreadId);
		uint64_t decreasingId = INT64_MAX - increasingId;
		return decreasingId;
	}
};

template<typename ValueType>
class VectorBasedValueGenerator {

private:
	std::vector<ValueType> const mValues;

public:
	template<typename ProvidedValues> VectorBasedValueGenerator(ProvidedValues && values)
		: mValues(std::forward<ProvidedValues>(values)) {

	}

	ValueType generateValue(size_t index, size_t /* shortThreadId */) const {
		return mValues[index];
	}
};

template<typename ValueType> using AlreadyInsertedValuesType = tbb::concurrent_vector<std::atomic<std::vector<ValueType>*>, tbb::zero_allocator<std::atomic<std::vector<ValueType>*>>>;

template<typename ValueType, typename ValueGenerator>
struct Inserter {
	std::shared_ptr<AlreadyInsertedValuesType<ValueType>> mAlreadyInsertedRanges;
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> mHot;
	std::atomic<uint16_t> mNextThreadId;
	tbb::concurrent_hash_map<std::thread::id, uint16_t, ThreadIdHashCompare> mThreadIdMap;
	using thread_id_accessor = typename tbb::concurrent_hash_map<std::thread::id, uint16_t, ThreadIdHashCompare>::accessor;
	std::atomic<size_t> mNumberSuccessfulInserts;
	std::atomic<size_t> mNumberFailed;
	std::unique_ptr<ValueGenerator> mValueGenerator;

	Inserter(
		std::shared_ptr<AlreadyInsertedValuesType<ValueType>> alreadyInsertedRanges,
		std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> hot,
		std::unique_ptr<ValueGenerator> &&valueGenerator
	)
		: mAlreadyInsertedRanges(alreadyInsertedRanges), mHot(hot), mNextThreadId(0u), mThreadIdMap(),
		  mNumberSuccessfulInserts(0), mNumberFailed(0), mValueGenerator(std::forward<std::unique_ptr<ValueGenerator>>(valueGenerator)) {
	}

	void operator()(const tbb::blocked_range<size_t> &range) {
		std::vector<ValueType>* insertedValues = new std::vector<ValueType>();
		insertedValues->reserve(range.size());
		thread_id_accessor accessor;
		std::thread::id const &currentThreadId = std::this_thread::get_id();
		if (!mThreadIdMap.find(accessor, currentThreadId)) {
			mThreadIdMap.insert(accessor, currentThreadId);
			accessor->second = mNextThreadId.fetch_add(1u);
		}
		uint16_t shortThreadId = accessor->second;
		size_t numberSuccessful = 0;
		for (size_t i = range.begin(); i != range.end(); ++i) {
			ValueType valueToInsert = mValueGenerator->generateValue(i, static_cast<size_t>(shortThreadId));
			insertedValues->push_back(valueToInsert);
			numberSuccessful += mHot->insert(valueToInsert);
		}
		mNumberSuccessfulInserts.fetch_add(numberSuccessful);
		mNumberFailed.fetch_add(range.size() - numberSuccessful);
		mAlreadyInsertedRanges->emplace_back(insertedValues);
	}


	size_t getNumberOfSuccesfulInserts() {
		return mNumberSuccessfulInserts.load(std::memory_order_acquire);
	}


	size_t getNumberOfFailedInserts() {
		return mNumberFailed.load(std::memory_order_acquire);
	}
};

template<typename ValueType> struct Scanner {
	std::shared_ptr<AlreadyInsertedValuesType<ValueType>> mAlreadyInsertedRanges;
	tbb::concurrent_vector<std::string> mFailures;
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> mHot;
	std::atomic<bool> mShouldAbort;
	std::atomic<bool> mExecutedSuccessful;
	std::atomic<size_t> mNumberScanOperationsSuccesfullyCompleted;


	Scanner(std::shared_ptr<AlreadyInsertedValuesType<ValueType>> alreadyInsertedRanges,
		std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> hot
	)
		: mAlreadyInsertedRanges(alreadyInsertedRanges), mHot(hot), mShouldAbort(false), mExecutedSuccessful(true), mNumberScanOperationsSuccesfullyCompleted(0u)
	{
	}

	void operator()() {
		typename idx::contenthelpers::KeyComparator<ValueType>::type lessThan;

		while(!mShouldAbort.load(std::memory_order_acquire) && mAlreadyInsertedRanges->size() == 0) {
			_mm_pause();
		}
		while(!mShouldAbort.load(std::memory_order_acquire)) {
			size_t numberValuesInAlreadyInsertedRange = 0u;
			std::set<ValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type> sampleValues;
			for (typename AlreadyInsertedValuesType<ValueType>::iterator it = mAlreadyInsertedRanges->begin(); it != mAlreadyInsertedRanges->end(); ++it) {
				std::vector<ValueType>* vector = it->load(std::memory_order_acquire);
				if(vector != nullptr) { //otherwise the element is not constructed completely
					numberValuesInAlreadyInsertedRange += vector->size();
					sampleValues.insert((*vector)[0]);
				}
			}
			size_t iteratedValues = 0u;
			size_t numberMatched = 0u;
			ValueType previousValue;
			for (ValueType const &value : *mHot) {
				if(iteratedValues > 0) {

					if(idx::contenthelpers::contentEquals(value, previousValue) || lessThan(value, previousValue)) {
						std::ostringstream out;
						out << "Wrong bound (previous value: " << previousValue << " currentValue " << value << ")";
						mFailures.push_back(out.str());
					}
				}
				++iteratedValues;
				if (sampleValues.find(value) != sampleValues.end()) {
					++numberMatched;
				}
				previousValue = value;
			}

			for (typename AlreadyInsertedValuesType<ValueType>::iterator it = mAlreadyInsertedRanges->begin(); it != mAlreadyInsertedRanges->end(); ++it) {
				std::vector<ValueType>* vector = it->load(std::memory_order_acquire);
				if(vector != nullptr) { //otherwise the element is not constructed completely
					ValueType searchKey = (*vector)[0];
					if (mHot->find(searchKey) == mHot->end()) {
						std::ostringstream out;
						out << "mHot->find(" << searchKey << ") != mHot->end() failed";
						mFailures.push_back(out.str());
					}
				}
			}

			std::vector<ValueType>* firstRange = (*mAlreadyInsertedRanges)[0].load(std::memory_order_acquire);
			if(firstRange != nullptr) {
				size_t mNumberValuesInRangeScan = 0ul;
				ValueType const &firstInsertedValue = firstRange->at(0);

				for (auto it = mHot->find(firstInsertedValue); it != mHot->end(); ++it) {
					ValueType const &value = *it;
					if (mNumberValuesInRangeScan > 0) {
						if (idx::contenthelpers::contentEquals(value, previousValue) ||
							lessThan(value, previousValue)) {
							std::ostringstream out;
							out << "Wrong bound (previous value: " << previousValue << " currentValue " << value << ")";
							mFailures.push_back(out.str());
						}
					}
					++mNumberValuesInRangeScan;
					previousValue = value;
				}
			}

			if(numberMatched != sampleValues.size()) {
				std::ostringstream out;
				out << "numberMatched != sampleValues.size() (" << numberMatched << " != " << sampleValues.size() << ")";
				mFailures.push_back(out.str());
			}
			if(iteratedValues < numberValuesInAlreadyInsertedRange) {
				std::ostringstream out;
				out << "iteratedValues < numberValuesInAlreadyInsertedRange failed (" << iteratedValues << "<" << numberValuesInAlreadyInsertedRange << ")";
				mFailures.push_back(out.str());
			}

			mNumberScanOperationsSuccesfullyCompleted.fetch_add(1u);
		}
	}

	void abortAllScanners() {
		mShouldAbort.store(true, std::memory_order_release);
	}

	size_t getNumberSuccessfulScanOperations() {
		return mNumberScanOperationsSuccesfullyCompleted.load(std::memory_order_acquire);
	}

};

template<typename ValueType>
struct Searcher {
	std::shared_ptr<AlreadyInsertedValuesType<ValueType>> mAlreadyInsertedRanges;
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> mHot;

	std::atomic<size_t> mNumberSuccesfulLookups;
	std::atomic<size_t> mNumberFailedLookups;
	std::atomic<bool> mShouldAbort;

	std::atomic<size_t> mTotalNumberOfExecutedLookups;

	Searcher(
		std::shared_ptr<AlreadyInsertedValuesType<ValueType>> alreadyInsertedRanges,
		std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> hot
	) : mAlreadyInsertedRanges(alreadyInsertedRanges), mHot(hot), mNumberSuccesfulLookups(0u), mNumberFailedLookups(0u),
		mShouldAbort(false), mTotalNumberOfExecutedLookups(0u) {
	}

	void operator()() {
		while (!mShouldAbort.load(std::memory_order_acquire)) {
			for (typename AlreadyInsertedValuesType<ValueType>::iterator it = mAlreadyInsertedRanges->begin(); it != mAlreadyInsertedRanges->end(); ++it) {
				std::vector<ValueType>* partOfInsertedValues = it->load(std::memory_order_acquire);
				if(partOfInsertedValues != nullptr) { //otherwise the current element is not initialized

					size_t numberSuccesfulInserted = 0u;
					for (ValueType const &insertedValue : *partOfInsertedValues) {
						numberSuccesfulInserted += mHot->lookup(insertedValue).mIsValid;
					}
					mNumberSuccesfulLookups.fetch_add(numberSuccesfulInserted);
					mNumberFailedLookups.fetch_add(partOfInsertedValues->size() - numberSuccesfulInserted);
					mTotalNumberOfExecutedLookups.fetch_add(partOfInsertedValues->size());
				}
				_mm_pause();
				if (mShouldAbort.load(std::memory_order_acquire)) {
					return;
				}
			}
		}
	}

	size_t getNumberOfSuccesfulLookups() {
		return mNumberSuccesfulLookups.load(std::memory_order_acquire);
	}


	size_t getNumberOfFailedLookups() {
		return mNumberFailedLookups.load(std::memory_order_acquire);
	}

	size_t getTotalNumberOfExecutedLookups() {
		return mTotalNumberOfExecutedLookups.load(std::memory_order_acquire);
	}

	void abortAllSearchers() {
		mShouldAbort.store(true, std::memory_order_release);
	}
};

template<typename ValueType, typename ValueGenerator, typename... Args> void executeMixedWorkloadLookupScanAndInsertTest(
	size_t numberOfValuesToInsert, Args &&... args) {
	std::shared_ptr<AlreadyInsertedValuesType<ValueType>> alreadyInsertedRanges(
		new AlreadyInsertedValuesType<ValueType>(),
		[](AlreadyInsertedValuesType<ValueType>* alreadyInserted) {
			for(std::atomic<std::vector<ValueType>*> & insertedRange : *alreadyInserted) {
				delete insertedRange.load(std::memory_order_acquire);
			}
			delete alreadyInserted;
		}
	);
	std::shared_ptr<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>> hot
		= std::make_shared<hot::rowex::HOTRowex<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();
	Inserter<ValueType, ValueGenerator> inserter(alreadyInsertedRanges, hot, std::make_unique<ValueGenerator>(std::forward<Args>(args)...));
	Searcher<ValueType> searcher { alreadyInsertedRanges, hot };
	Scanner<ValueType> scanner { alreadyInsertedRanges, hot };
	size_t numberValuesToInsert = numberOfValuesToInsert;

	size_t numberThreadsToUse = std::max<size_t>(static_cast<size_t>(std::thread::hardware_concurrency()/2.0), 1ul);

	std::vector<std::thread> searchThreads;
	for(size_t i=0; i < numberThreadsToUse; ++i) {
		searchThreads.push_back(std::thread([&searcher] {
			searcher();
		}));
	}
	std::vector<std::thread> scanThreads;
	for(size_t i=0; i < numberThreadsToUse; ++i) {
		scanThreads.push_back(std::thread([&scanner] {
			scanner();
		}));
	}

	tbb::task_group insertGroup;
	tbb::task_arena limitedInsertThreads(std::max<size_t>(2, numberThreadsToUse), 1);

	limitedInsertThreads.execute([&]{ // Use at most 2 threads for this job.
		insertGroup.run([&]{ // run in task group
			tbb::parallel_for(tbb::blocked_range<size_t>( 0,  numberValuesToInsert, 100), [&](const tbb::blocked_range<size_t>& range) {
				try {
					inserter(range);
				} catch(std::exception const & e) {
					std::cout << "Exception on insert " << e.what() << std::endl;
				} catch(...) {
					std::cout << "Unknown exception" << std::endl;
				}
			});
		});
	});

	insertGroup.wait();
	searcher.abortAllSearchers();
	scanner.abortAllScanners();

	std::for_each(searchThreads.begin(), searchThreads.end(), [](std::thread & thread) -> void {
		thread.join();
	});

	std::for_each(scanThreads.begin(), scanThreads.end(), [](std::thread & thread) -> void {
		thread.join();
	});

	std::cout << "Executed " << inserter.getNumberOfSuccesfulInserts() << " succesful inserts ( " << inserter.getNumberOfFailedInserts() << " failed)" << std::endl;
	std::cout << "Executed " << searcher.getNumberOfSuccesfulLookups() << " succesful lookups (" << searcher.getNumberOfFailedLookups() << " failed)" << std::endl;
	std::cout << "Executed " << scanner.getNumberSuccessfulScanOperations() << " scans" << std::endl;

	for(std::string const & failure : scanner.mFailures) {
		std::cerr << "Failure in scan: " << failure << std::endl;
	}

	std::vector<ValueType> allInsertedValues;
	std::shared_ptr<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>> unsynchronizedHOT = std::make_shared<hot::singlethreaded::HOTSingleThreaded<ValueType, idx::contenthelpers::IdentityKeyExtractor>>();

	for (std::atomic<std::vector<ValueType>*> & partOfInsertedValuesHolder : (*alreadyInsertedRanges)) {
		std::vector<ValueType>* partOfInsertedValues = partOfInsertedValuesHolder.load(std::memory_order_acquire);
		if(partOfInsertedValues != nullptr) {
			for (ValueType const &insertedValue : (*partOfInsertedValues)) {
				allInsertedValues.push_back(insertedValue);
				unsynchronizedHOT->insert(insertedValue);
			}
		}
	}

	checkConsistency(hot, unsynchronizedHOT, allInsertedValues);
}

} }

#endif