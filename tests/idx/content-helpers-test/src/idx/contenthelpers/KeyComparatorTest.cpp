//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <idx/contenthelpers/KeyComparator.hpp>

namespace idx { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(KeyComparatorTest)

BOOST_AUTO_TEST_CASE(testKeyComparatorForIntegralTypes) {
	typename idx::contenthelpers::KeyComparator<uint64_t>::type compare;

	BOOST_REQUIRE(compare(3, 4) == true);
	BOOST_REQUIRE(compare(4, 3) == false);
	BOOST_REQUIRE(compare(10, 10) == false);
}

BOOST_AUTO_TEST_CASE(testKeyComparatorForCString) {
	typename idx::contenthelpers::KeyComparator<const char*>::type compare;

	BOOST_REQUIRE(compare("A", "B") == true);
	BOOST_REQUIRE(compare("B", "A") == false);
	BOOST_REQUIRE(compare("A", "A") == false);
}


BOOST_AUTO_TEST_SUITE_END()

}}