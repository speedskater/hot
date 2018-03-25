//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <idx/contenthelpers/CStringComparator.hpp>

namespace idx { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(CStringComparatorTest)

BOOST_AUTO_TEST_CASE(testCompareCStrings) {
	idx::contenthelpers::CStringComparator compareCStrings;

	BOOST_REQUIRE(compareCStrings("A", "B") == true);
	BOOST_REQUIRE(compareCStrings("B", "A") == false);
	BOOST_REQUIRE(compareCStrings("A", "A") == false);
}

BOOST_AUTO_TEST_SUITE_END()

}}