#ifndef __IDX__CONTENTHELPERS__KEY_COMPARATOR__HPP__
#define __IDX__CONTENTHELPERS__KEY_COMPARATOR__HPP__

#include "idx/contenthelpers/CStringComparator.hpp"

namespace idx { namespace contenthelpers {

template<typename V> struct KeyComparator {
	using type = std::less<V>;
};

template<> struct KeyComparator<const char*> {
	using type = idx::contenthelpers::CStringComparator;
};

} }

#endif