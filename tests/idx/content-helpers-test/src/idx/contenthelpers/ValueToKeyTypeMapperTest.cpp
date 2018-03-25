//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>
#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/PairKeyExtractor.hpp>
#include <idx/contenthelpers/ValueToKeyTypeMapper.hpp>

namespace idx { namespace contenthelpers {

BOOST_AUTO_TEST_SUITE(ValueToKeyTypeMapperTest)

BOOST_AUTO_TEST_CASE(testValueToKeyTypeMapperForIdentityKeyExtractorAndUint64) {
	using KeyType = typename ValueToKeyTypeMapper<uint64_t, IdentityKeyExtractor>::KeyType;

	BOOST_REQUIRE(typeid(KeyType) == typeid(uint64_t));
}

BOOST_AUTO_TEST_CASE(testValueToKeyTypeMapperForPairKeyExtractorAndStdStringUint32Pair) {
	using KeyType = typename ValueToKeyTypeMapper<std::pair<std::string, uint32_t>, PairKeyExtractor>::KeyType;

	BOOST_REQUIRE(typeid(KeyType) == typeid(std::string));
}

BOOST_AUTO_TEST_SUITE_END()

}}