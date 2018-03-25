//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <idx/benchmark/BenchmarkEventReaderException.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(BenchmarkEventReaderExceptionTest)

BOOST_AUTO_TEST_CASE(testWhat) {
	idx::benchmark::BenchmarkEventReaderException exception("Read illegal benchmark event type", "REAID asfsdaf asdfsaf", 10, "BechmarkEventReadi", 1234);
	BOOST_REQUIRE_EQUAL(exception.what(), "Read illegal benchmark event type on line 10 \"REAID asfsdaf asdfsaf\" at BechmarkEventReadi:1234");
}

BOOST_AUTO_TEST_SUITE_END()

}}