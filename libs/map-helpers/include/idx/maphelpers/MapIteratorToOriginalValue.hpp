#ifndef __IDX__MAPHELPERS__MAP_ITERATOR_TO_ORIGINAL_VALUE__HPP__
#define __IDX__MAPHELPERS__MAP_ITERATOR_TO_ORIGINAL_VALUE__HPP__

#include <utility>

#include <idx/contenthelpers/IdentityKeyExtractor.hpp>
#include <idx/contenthelpers/PairKeyExtractor.hpp>
#include <idx/contenthelpers/PairPointerKeyExtractor.hpp>
#include <idx/maphelpers/MapValueExtractor.hpp>

#include <idx/contenthelpers/KeyComparator.hpp>

namespace idx { namespace maphelpers {

template<template<typename...> typename MapType, typename ValueType,
	template<typename> typename KeyExtractorType>
struct MapIteratorToOriginalValue {
	static KeyExtractorType<ValueType> toKey;
	static MapValueExtractor<ValueType, KeyExtractorType> toValue;

	using MapKeyType = decltype(toKey(std::declval<ValueType>()));
	using MapValueType = decltype(toValue(std::declval<ValueType>()));
	using IteratorType = typename MapType<MapKeyType, MapValueType, typename idx::contenthelpers::KeyComparator<MapKeyType>::type>::const_iterator;

	MapValueType operator()(IteratorType const & iterator) {
		return iterator->second;
	}
};

template<template<typename...> typename MapType, typename ValueType> struct MapIteratorToOriginalValue<MapType, ValueType, idx::contenthelpers::IdentityKeyExtractor> {
	static MapValueExtractor<ValueType, idx::contenthelpers::IdentityKeyExtractor> toValue;
	using MapValueType = decltype(toValue(std::declval<ValueType>()));

	using IteratorType = typename MapType<ValueType, MapValueType, typename idx::contenthelpers::KeyComparator<ValueType>::type>::const_iterator;

	ValueType operator()(IteratorType const & iterator) {
		return iterator->second;
	}
};

template<template<typename...> typename MapType, typename ValueType> struct MapIteratorToOriginalValue<MapType, ValueType, idx::contenthelpers::PairKeyExtractor> {
	static idx::contenthelpers::PairKeyExtractor<ValueType> toKey;
	static MapValueExtractor<ValueType, idx::contenthelpers::PairKeyExtractor> toValue;

	using MapKeyType = decltype(toKey(std::declval<ValueType>()));
	using MapValueType = decltype(toValue(std::declval<ValueType>()));
	using IteratorType = typename MapType<MapKeyType, MapValueType, typename idx::contenthelpers::KeyComparator<MapKeyType>::type>::const_iterator;

	auto operator()(IteratorType const & iterator) {
		return *iterator;
	}
};

}}

#endif