//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <idx/benchmark/BenchmarkEventTypeId.hpp>
#include <idx/benchmark/BenchmarkEvent.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventTest)

BOOST_AUTO_TEST_CASE(testUpdateEventConstruction) {
	BenchmarkEvent<BenchmarkEventTypeId::UpdateEventTypeId, uint64_t, idx::contenthelpers::IdentityKeyExtractor> updateEvent;

	updateEvent.getData().mNewValue = 42ul;

	BOOST_REQUIRE_EQUAL(updateEvent.getData().mNewValue, 42ul);
}

BOOST_AUTO_TEST_SUITE_END()

}}