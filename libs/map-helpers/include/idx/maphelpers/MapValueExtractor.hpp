#ifndef __IDX__MAPHELPERS__MAP_VALUE_EXTRACTOR__HPP__
#define __IDX__MAPHELPERS__MAP_VALUE_EXTRACTOR__HPP__

#include <utility>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/PairKeyExtractor.hpp>
#include <idx/contenthelpers/PairPointerKeyExtractor.hpp>

#include <idx/contenthelpers/KeyComparator.hpp>

namespace idx { namespace maphelpers {

template<typename ValueType, template <typename> typename KeyExtractorType> struct MapValueExtractor {
	ValueType operator()(ValueType const &value) {
		return value;
	}
};

template<typename ValueType> struct MapValueExtractor<ValueType, idx::contenthelpers::PairKeyExtractor> {
	auto operator()(ValueType const &value) {
		return value.second;
	}
};

}}

#endif