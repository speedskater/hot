//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <idx/benchmark/BenchmarkEventTypeId.hpp>
#include <idx/benchmark/BenchmarkEventTypeIdToEventTypeMapping.hpp>
#include <idx/benchmark/InsertEvent.hpp>
#include <idx/benchmark/LookupEvent.hpp>
#include <idx/benchmark/ScanEvent.hpp>
#include <idx/benchmark/UpdateEvent.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventTypeIdToEventTypeMappingTest)

BOOST_AUTO_TEST_CASE(testUpdateMapping) {
	using BenchmarkEventType = typename BenchmarkEventTypeIdToEventType<uint64_t, idx::contenthelpers::IdentityKeyExtractor, BenchmarkEventTypeId::UpdateEventTypeId>::EventType;

	BOOST_REQUIRE(typeid(BenchmarkEventType) == typeid(UpdateEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>));
}

BOOST_AUTO_TEST_CASE(testLookupMapping) {
	using BenchmarkEventType = typename BenchmarkEventTypeIdToEventType<uint64_t, idx::contenthelpers::IdentityKeyExtractor, BenchmarkEventTypeId::LookupEventTypeId>::EventType;

	BOOST_REQUIRE(typeid(BenchmarkEventType) == typeid(LookupEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>));
}

BOOST_AUTO_TEST_CASE(testInsertMapping) {
	using BenchmarkEventType = typename BenchmarkEventTypeIdToEventType<uint64_t, idx::contenthelpers::IdentityKeyExtractor, BenchmarkEventTypeId::InsertEventTypeId>::EventType;

	BOOST_REQUIRE(typeid(BenchmarkEventType) == typeid(InsertEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>));
}

BOOST_AUTO_TEST_CASE(testScanMapping) {
	using BenchmarkEventType = typename BenchmarkEventTypeIdToEventType<uint64_t, idx::contenthelpers::IdentityKeyExtractor, BenchmarkEventTypeId::ScanEventTypeId>::EventType;

	BOOST_REQUIRE(typeid(BenchmarkEventType) == typeid(ScanEvent<uint64_t, idx::contenthelpers::IdentityKeyExtractor>));
}

BOOST_AUTO_TEST_SUITE_END()

}}