#ifndef __IDX__CONTENTHELPERS__PAIR_POINTER_KEY_EXTRACTOR__
#define __IDX__CONTENTHELPERS__PAIR_POINTER_KEY_EXTRACTOR__

namespace idx { namespace contenthelpers {

template<typename PointerPairLikeType> struct PairPointerKeyExtractor {
	using KeyType = decltype(std::declval<PointerPairLikeType>()->first);

	inline KeyType operator()(PointerPairLikeType const & value) const {
		return value->first;
	}
};

} }

#endif