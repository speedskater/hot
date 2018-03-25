//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <idx/contenthelpers/ContentEquals.hpp>

namespace idx { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(ContentEqualsTest)

BOOST_AUTO_TEST_CASE(testEqualityCheckString) {
	BOOST_REQUIRE(contenthelpers::contentEquals("A", "B") == false);
	BOOST_REQUIRE(contenthelpers::contentEquals("B", "A") == false);
	BOOST_REQUIRE(contenthelpers::contentEquals("A", "A") == true);
}

BOOST_AUTO_TEST_CASE(testEqualityCheckPointer) {
	void* pointer1 = reinterpret_cast<void*>(1ul);
	void* sameAddressAsPointer1 = reinterpret_cast<void*>(1ul);
	void* pointer2 = reinterpret_cast<void*>(2ul);

	BOOST_REQUIRE(contenthelpers::contentEquals(pointer1, sameAddressAsPointer1) == true);
	BOOST_REQUIRE(contenthelpers::contentEquals(pointer1, pointer2) == false);
	BOOST_REQUIRE(contenthelpers::contentEquals(sameAddressAsPointer1, pointer2) == false);
}

BOOST_AUTO_TEST_SUITE_END()

}}