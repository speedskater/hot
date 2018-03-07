#ifndef __IDX__CONTENTHELPERS__VALUE_TO_KEY_TYPE_MAPPER__HPP__
#define __IDX__CONTENTHELPERS__VALUE_TO_KEY_TYPE_MAPPER__HPP__

#include <utility>

namespace idx { namespace contenthelpers {

template<typename ValueType, template <typename> typename KeyExtractor> struct ValueToKeyTypeMapper {
	using KeyType = decltype(std::declval<KeyExtractor<ValueType>>()(std::declval<ValueType>()));
};

}}

#endif
