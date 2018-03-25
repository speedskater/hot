//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <idx/benchmark/BenchmarkEventTypeId.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventTypeIdTest)

BOOST_AUTO_TEST_CASE(testEnumValue) {
	BOOST_REQUIRE(static_cast<int>(BenchmarkEventTypeId::UpdateEventTypeId) == 0);
	BOOST_REQUIRE(static_cast<int>(BenchmarkEventTypeId::LookupEventTypeId) == 1);
	BOOST_REQUIRE(static_cast<int>(BenchmarkEventTypeId::ScanEventTypeId) == 2);
	BOOST_REQUIRE(static_cast<int>(BenchmarkEventTypeId::InsertEventTypeId) == 3);
}

BOOST_AUTO_TEST_SUITE_END()

}}