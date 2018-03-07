//
//  Created by Robert Binna
//
#include <map>

#include <boost/test/unit_test.hpp>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>

#include <idx/maphelpers/STLLikeIndex.hpp>

namespace idx { namespace maphelpers {

BOOST_AUTO_TEST_SUITE(STLLikeIndexTest)

template<typename ValueType> void checkSearchResult(idx::contenthelpers::OptionalValue<ValueType> const & optional, idx::contenthelpers::OptionalValue<ValueType> const & expectedOptional) {
	BOOST_REQUIRE(optional.compliesWith(expectedOptional));
}

BOOST_AUTO_TEST_CASE(testIdentityValuesMap) {
	idx::maphelpers::STLLikeIndex<std::map, uint64_t, idx::contenthelpers::IdentityKeyExtractor> map;

	BOOST_REQUIRE(map.insert(3u));
	BOOST_REQUIRE(map.insert(7u));
	BOOST_REQUIRE(map.insert(8u));
	BOOST_REQUIRE(map.insert(10u));
	BOOST_REQUIRE(map.insert(16u));
	BOOST_REQUIRE(map.insert(42u));

	BOOST_REQUIRE(!map.insert(3u));

	checkSearchResult(map.lookup(3u), { true, 3u });
	checkSearchResult(map.lookup(4u), { false, 4u });
	checkSearchResult(map.lookup(4u), { false, 3u });

	checkSearchResult(map.scan(7u, 4), { true, 42u });
	checkSearchResult(map.scan(7u, 5), {});
	checkSearchResult(map.scan(7u, 500), {});
}

BOOST_AUTO_TEST_CASE(testPairValuesMap) {
	idx::maphelpers::STLLikeIndex<std::map, std::pair<char const *, uint64_t>, idx::contenthelpers::PairKeyExtractor> map;

	char const * test = "test";

	map.insert({ test, 42u });

	checkSearchResult(map.lookup("test"), { true, { "test", 42u }});
	checkSearchResult(map.lookup("test2"), {});

}

BOOST_AUTO_TEST_CASE(testUpsert) {
	idx::maphelpers::STLLikeIndex<std::map, std::pair<uint64_t, uint64_t>, idx::contenthelpers::PairKeyExtractor> map;

	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(3u, 10u)));
	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(7u, 8u)));
	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(8u, 11u)));
	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(10u, 16u)));
	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(16u, 20u)));
	BOOST_REQUIRE(map.insert(std::pair<uint64_t, uint64_t>(42u, 21u)));

	idx::contenthelpers::OptionalValue<std::pair<uint64_t, uint64_t>> previousValue = map.upsert(std::pair<uint64_t, uint64_t>(7u, 22u));
	BOOST_REQUIRE(previousValue.compliesWith({ true, std::pair<uint64_t, uint64_t>(7u, 8u) }));

	BOOST_REQUIRE(map.lookup(7u).compliesWith({ true, std::pair<uint64_t, uint64_t>(7u, 22u) }));

	idx::contenthelpers::OptionalValue<std::pair<uint64_t, uint64_t>> previousValue2 = map.upsert(std::pair<uint64_t, uint64_t>(9u, 42u));
	BOOST_REQUIRE(map.lookup(9u).compliesWith({ true, std::pair<uint64_t, uint64_t>(9u, 42u) }));

	BOOST_REQUIRE(previousValue2.compliesWith({}));
}

BOOST_AUTO_TEST_SUITE_END()

}}