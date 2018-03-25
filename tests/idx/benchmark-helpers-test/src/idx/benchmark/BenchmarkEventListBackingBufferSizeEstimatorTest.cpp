//
//  Created by Robert Binna
//

#include <utility>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/PairKeyExtractor.hpp>

#include <idx/benchmark/BenchmarkEvent.hpp>
#include <idx/benchmark/BenchmarkEventListBackingBufferSizeEstimator.hpp>
#include <idx/benchmark/BenchmarkEventTypeId.hpp>
#include <idx/benchmark/BenchmarkLineReader.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventListBackingBufferSizeEstimatorTest)

template <BenchmarkEventTypeId eventTypeId> using EventType = BenchmarkEvent<eventTypeId, std::pair<const char*, uint64_t>, idx::contenthelpers::PairKeyExtractor>;

BOOST_AUTO_TEST_CASE(estimateSize) {
	BenchmarkLineReader<std::pair<const char*, uint64_t>, idx::contenthelpers::PairKeyExtractor> readLine;
	BenchmarkEventListBackingBuferSizeEstimator sizeEstimator {  };

	sizeEstimator.addEvent(readLine, "INSERT \"fourty four\" 42", 1u);
	size_t firstEventSize = sizeof(EventType<InsertEventTypeId>) + strlen("fourty four") + 1;
	BOOST_REQUIRE_EQUAL(sizeEstimator.getTotalBufferSize(), firstEventSize);
	sizeEstimator.addEvent(readLine, "UPDATE \"fourty three\" 43", 2u);
	size_t updateEventBaseSize = sizeof(EventType<UpdateEventTypeId>);
	size_t secondEventSize = updateEventBaseSize + strlen("fourty three") + 1;
	size_t firstAlignmentOverhead = 4u;
	BOOST_REQUIRE_EQUAL(sizeEstimator.getTotalBufferSize(), firstEventSize + secondEventSize + firstAlignmentOverhead);
	sizeEstimator.addEvent(readLine, "SCAN \"fourty two\" 20", 3u);
	size_t thirdEventBaseSize = sizeof(EventType<ScanEventTypeId>);
	size_t thirdEventSize = thirdEventBaseSize + strlen("fourty two") + 1;
	size_t secondAlignmentOverhead = 3u;
	BOOST_REQUIRE_EQUAL(sizeEstimator.getTotalBufferSize(), firstEventSize + secondEventSize + thirdEventSize + firstAlignmentOverhead + secondAlignmentOverhead);
	sizeEstimator.addEvent(readLine, "READ \"fourty three\"", 4u);

	size_t baseSizes = sizeof(EventType<InsertEventTypeId>) + sizeof(EventType<UpdateEventTypeId>) + sizeof(EventType<ScanEventTypeId>) + sizeof(EventType<LookupEventTypeId>);
	size_t additionalContentSize = (strlen("fourty four") + 1) + (strlen("fourty three") + 1) + (strlen("fourty two") + 1) + (strlen("fourty three") + 1);
	size_t alignmentOffsets = 4 + 3 + 5;

	BOOST_REQUIRE_EQUAL(sizeEstimator.getTotalBufferSize(), baseSizes + additionalContentSize + alignmentOffsets);
}

BOOST_AUTO_TEST_SUITE_END()

}}