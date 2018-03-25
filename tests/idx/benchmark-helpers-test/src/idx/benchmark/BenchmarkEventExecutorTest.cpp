//
//  Created by Robert Binna
//

#include <utility>
#include <map>
#include <memory>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/PairPointerKeyExtractor.hpp>

#include <idx/maphelpers/STLLikeIndex.hpp>

#include <idx/benchmark/BenchmarkEventExecutor.hpp>
#include <idx/benchmark/BenchmarkEventReader.hpp>
#include <idx/benchmark/SpecificEventProcessor.hpp>

#include "idx/benchmark/TempFile.hpp"

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(EventBasedBenchmarkExecutorTest)

BOOST_AUTO_TEST_CASE(testBenchmarkEventsExecutor) {
	using IndexType = idx::maphelpers::STLLikeIndex<std::map, std::pair<uint64_t, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;
	using EventExecutorType = BenchmarkEventExecutor<IndexType, std::pair<uint64_t, uint64_t>*, idx::contenthelpers::PairPointerKeyExtractor>;

	std::shared_ptr<IndexType> index = std::make_shared<IndexType>();
	EventExecutorType processEvent(index);

	std::pair<uint64_t, uint64_t> first = { 1, 123 };
	std::pair<uint64_t, uint64_t> second = { 2, 223 };
	std::pair<uint64_t, uint64_t> third = { 3, 223 };

	BOOST_REQUIRE(processEvent(typename EventExecutorType::SpecificInsertEvent { &first }));
	BOOST_REQUIRE(processEvent(typename EventExecutorType::SpecificInsertEvent { &second }));
	BOOST_REQUIRE(processEvent(typename EventExecutorType::SpecificInsertEvent { &third }));
	BOOST_REQUIRE(processEvent(typename EventExecutorType::SpecificLookupEvent { 2, idx::contenthelpers::OptionalValue<std::pair<uint64_t, uint64_t>*>( true, &second )}));
	BOOST_REQUIRE(processEvent(typename EventExecutorType::SpecificScanEvent { 1, 2, idx::contenthelpers::OptionalValue<std::pair<uint64_t, uint64_t>*>( true, &third )}));
}

BOOST_AUTO_TEST_CASE(testEventsExecutorWithBenchmarkEventList) {
	using IndexType = idx::maphelpers::STLLikeIndex<std::map, uint64_t, idx::contenthelpers::IdentityKeyExtractor>;
	using EventExecutorType = BenchmarkEventExecutor<IndexType, uint64_t, idx::contenthelpers::IdentityKeyExtractor>;

	TempFile tempEventFile {
		"INSERT 1234",
		"INSERT 42",
		"INSERT 32",
		"READ 32",
		"READ 42",
		"SCAN 42 10",
		"UPDATE 42"
	};

	BenchmarkEventReader<uint64_t, idx::contenthelpers::IdentityKeyExtractor> reader(tempEventFile.mFileName);
	SpecificEventProcessor<uint64_t, idx::contenthelpers::IdentityKeyExtractor> processEvent;
	std::shared_ptr<IndexType> index = std::make_shared<IndexType>();
	EventExecutorType executeEvent(index);
	BenchmarkEventList events = reader.readEvents();

	for(BenchmarkBaseEvent & event : events) {
		bool result = processEvent(&event, executeEvent);
		BOOST_REQUIRE(result);
	}
}


BOOST_AUTO_TEST_SUITE_END()

}}