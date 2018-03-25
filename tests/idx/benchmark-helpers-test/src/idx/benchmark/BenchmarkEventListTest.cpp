//
//  Created by Robert Binna
//

#include <utility>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/PairKeyExtractor.hpp>

#include <idx/benchmark/BenchmarkEvent.hpp>
#include <idx/benchmark/BenchmarkEventListBackingBufferSizeEstimator.hpp>
#include <idx/benchmark/BenchmarkEventList.hpp>
#include <idx/benchmark/BenchmarkEventTypeId.hpp>
#include <idx/benchmark/BenchmarkLineReader.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventListBackingBufferSizeEstimatorTest)

template <BenchmarkEventTypeId eventTypeId> using EventType = BenchmarkEvent<eventTypeId, std::pair<const char*, uint64_t>, idx::contenthelpers::PairKeyExtractor>;

BOOST_AUTO_TEST_CASE(createAndIterateBenchmarkEventList) {
	BenchmarkLineReader<std::pair<const char*, uint64_t>, idx::contenthelpers::PairKeyExtractor> readLine;

	std::array<std::string, 4> lines {
		"INSERT \"fourty four\" 42",
		"UPDATE \"fourty three\" 43",
		"SCAN \"fourty two\" 20",
		"READ \"fourty three\""
	};

	for(size_t linesToTest = 1; linesToTest <= lines.size(); ++linesToTest) {
		BenchmarkEventListBackingBuferSizeEstimator sizeEstimator {  };

		//std::cout << "test for " << linesToTest << " lines" << std::endl;
		for (size_t i = 0; i < linesToTest; ++i) {
			sizeEstimator.addEvent(readLine, lines[i], i + 1);
		}
		BenchmarkEventList events{sizeEstimator.getTotalBufferSize()};

		for (size_t i = 0; i < linesToTest; ++i) {
			events.addEvent(readLine, lines[i], i + 1);
		}
		size_t i = 0;
		BenchmarkBaseEvent const *previousEvent;
		BenchmarkEventTypeId eventTypeIds[]{InsertEventTypeId, UpdateEventTypeId, ScanEventTypeId, LookupEventTypeId};
		for (BenchmarkBaseEvent const &event : events) {
			BOOST_REQUIRE_EQUAL(event.mEventTypeId, eventTypeIds[i]);
			if (i > 0u) {
				BOOST_REQUIRE_EQUAL((void *) previousEvent->mNextEvent, (void *) &event);
			}
			previousEvent = &event;
			++i;
		}
		BOOST_REQUIRE_EQUAL(i, linesToTest);
		BOOST_REQUIRE_EQUAL(events.getRemainingBackingBytes(), 0u);

		BOOST_REQUIRE_EQUAL(events.size(),linesToTest);
	}

}

BOOST_AUTO_TEST_CASE(createAndIterateBenchmarkEventListFixed) {
	BenchmarkLineReader<std::pair<const char*, uint64_t>, idx::contenthelpers::PairKeyExtractor> readLine;
	BenchmarkEventListBackingBuferSizeEstimator sizeEstimator {  };

	sizeEstimator.addEvent(readLine, "INSERT \"fourty four\" 42", 1u);
	sizeEstimator.addEvent(readLine, "UPDATE \"fourty three\" 43", 2u);
	sizeEstimator.addEvent(readLine, "SCAN \"fourty two\" 20", 3u);
	sizeEstimator.addEvent(readLine, "READ \"fourty three\"", 4u);

	BenchmarkEventList events { sizeEstimator.getTotalBufferSize() };

	events.addEvent(readLine, "INSERT \"fourty four\" 42", 1u);
	events.addEvent(readLine, "UPDATE \"fourty three\" 43", 2u);
	events.addEvent(readLine, "SCAN \"fourty two\" 20", 3u);
	events.addEvent(readLine, "READ \"fourty three\"", 4u);

	int i=0;
	BenchmarkBaseEvent const * previousEvent;
	BenchmarkEventTypeId eventTypeIds[] { InsertEventTypeId, UpdateEventTypeId, ScanEventTypeId, LookupEventTypeId };
	for(BenchmarkBaseEvent const & event : events) {
		BOOST_REQUIRE_EQUAL(event.mEventTypeId, eventTypeIds[i]);
		if(i > 0) {
			BOOST_REQUIRE_EQUAL((void*) previousEvent->mNextEvent, (void*) &event);
		}
		previousEvent = &event;
		++i;
	}
	BOOST_REQUIRE_EQUAL(i, 4);
	BOOST_REQUIRE_EQUAL(events.getRemainingBackingBytes(), 0u);
}

BOOST_AUTO_TEST_SUITE_END()

}}