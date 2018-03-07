//
//  Created by Robert Binna
//
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <spider/contenthelpers/CStringComparator.hpp>

namespace spider { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(CStringComparatorTest)

BOOST_AUTO_TEST_CASE(testCompareCStrings) {
	spider::contenthelpers::CStringComparator compareCStrings;

	BOOST_REQUIRE(compareCStrings("A", "B") == true);
	BOOST_REQUIRE(compareCStrings("B", "A") == false);
	BOOST_REQUIRE(compareCStrings("A", "A") == false);
}

BOOST_AUTO_TEST_SUITE_END()

}}