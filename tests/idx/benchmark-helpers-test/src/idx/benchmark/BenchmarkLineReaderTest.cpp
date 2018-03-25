//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/PairKeyExtractor.hpp>

#include <idx/benchmark/ContinuosBuffer.hpp>
#include <idx/benchmark/BenchmarkBaseEvent.hpp>
#include <idx/benchmark/BenchmarkLineReader.hpp>
#include <idx/benchmark/BenchmarkEvent.hpp>
#include <idx/benchmark/BenchmarkEventTypeId.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkLineReaderTest)

BOOST_AUTO_TEST_CASE(testReadInsertBenchmarkEvent) {
	using TargetEventType = BenchmarkEvent<BenchmarkEventTypeId::InsertEventTypeId, uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	BenchmarkLineReader<uint64_t, idx::contenthelpers::IdentityKeyExtractor> readEvent;

	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer { backingBuffer, 2048u };
	std::pair<BenchmarkBaseEvent*, ContinuousBuffer> const & insertEvent = readEvent("INSERT 1234567", 0, buffer);

	BOOST_REQUIRE(insertEvent.first->mEventTypeId == BenchmarkEventTypeId::InsertEventTypeId);
	BOOST_REQUIRE(insertEvent.first->mNextEvent == nullptr);

	BOOST_REQUIRE_EQUAL(insertEvent.second.mRemainingBufferSize, 2048u - sizeof(TargetEventType));

	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(insertEvent.first)->getData().mValueToInsert, 1234567u);
}

BOOST_AUTO_TEST_CASE(testReadLookupBenchmarkEvent) {
	using ValueType = std::pair<char const*, uint64_t>;
	using TargetEventType = BenchmarkEvent<BenchmarkEventTypeId::LookupEventTypeId, ValueType, idx::contenthelpers::PairKeyExtractor>;
	BenchmarkLineReader<ValueType, idx::contenthelpers::PairKeyExtractor> readEvent;

		char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer { backingBuffer, 2048u };
	std::pair<BenchmarkBaseEvent*, ContinuousBuffer> const & lookupEvent = readEvent("READ \"forty two\"", 0, buffer);

	BOOST_REQUIRE(lookupEvent.first->mEventTypeId == BenchmarkEventTypeId::LookupEventTypeId);
	BOOST_REQUIRE(lookupEvent.first->mNextEvent == nullptr);

	size_t stringSize = strlen("forty two") + 1;

	BOOST_REQUIRE_EQUAL(lookupEvent.second.mRemainingBufferSize, 2048u - sizeof(TargetEventType) - stringSize);

	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(lookupEvent.first)->getData().mKey, "forty two");
}

BOOST_AUTO_TEST_CASE(testReadScanBenchmarkEvent) {
	using ValueType = std::pair<char const*, char*>;
	using TargetEventType = BenchmarkEvent<BenchmarkEventTypeId::ScanEventTypeId, ValueType, idx::contenthelpers::PairKeyExtractor>;
	BenchmarkLineReader<ValueType, idx::contenthelpers::PairKeyExtractor> readEvent;

	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer { backingBuffer, 2048u };
	std::pair<BenchmarkBaseEvent*, ContinuousBuffer> const & lookupEvent = readEvent("SCAN \"forty four\" 342", 0, buffer);

	BOOST_REQUIRE(lookupEvent.first->mEventTypeId == BenchmarkEventTypeId::ScanEventTypeId);
	BOOST_REQUIRE(lookupEvent.first->mNextEvent == nullptr);

	size_t stringSize = strlen("forty four") + 1;

	BOOST_REQUIRE_EQUAL(lookupEvent.second.mRemainingBufferSize, 2048u - sizeof(TargetEventType) - stringSize);

	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(lookupEvent.first)->getData().mLookupKey, "forty four");
	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(lookupEvent.first)->getData().mNumberValuesToScan, 342u);
}

BOOST_AUTO_TEST_CASE(testReadUpdateBenchmarkEvent) {
	using ValueType = std::pair<char const*, char*>;
	using TargetEventType = BenchmarkEvent<BenchmarkEventTypeId::UpdateEventTypeId, ValueType, idx::contenthelpers::PairKeyExtractor>;
	BenchmarkLineReader<ValueType, idx::contenthelpers::PairKeyExtractor> readEvent;

	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer { backingBuffer, 2048u };
	std::pair<BenchmarkBaseEvent*, ContinuousBuffer> const & updateEvent = readEvent("UPDATE \"forty four\" \"vierundvierzig\"", 0, buffer);

	BOOST_REQUIRE(updateEvent.first->mEventTypeId == BenchmarkEventTypeId::UpdateEventTypeId);
	BOOST_REQUIRE(updateEvent.first->mNextEvent == nullptr);

	size_t keyStringSize = strlen("forty four") + 1;
	size_t valueStringSize = strlen("vierundvierzig") + 1;

	BOOST_REQUIRE_EQUAL(updateEvent.second.mRemainingBufferSize, 2048u - sizeof(TargetEventType) - keyStringSize - valueStringSize);

	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(updateEvent.first)->getData().mNewValue.first, "forty four");
	BOOST_REQUIRE_EQUAL(reinterpret_cast<TargetEventType*>(updateEvent.first)->getData().mNewValue.second, "vierundvierzig");
}


BOOST_AUTO_TEST_CASE(testReadScanBenchmarkEventFailure) {
	using ValueType = std::pair<char const*, char*>;
	BenchmarkLineReader<ValueType, idx::contenthelpers::PairKeyExtractor> readEvent;

	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer { backingBuffer, 2048u };

	try {
		readEvent("SCAN \"forty four\"", 0, buffer);
		BOOST_FAIL("An exception must be thrown because Scan event needs an additional parameter specifying how many entries to scan.");
	} catch(BenchmarkEventReaderException e) {
		//must be thrown
	}
}


BOOST_AUTO_TEST_SUITE_END()

}}