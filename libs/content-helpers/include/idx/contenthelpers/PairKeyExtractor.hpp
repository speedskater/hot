#ifndef __IDX__CONTENTHELPERS__PAIR_KEY_EXTRACTOR__
#define __IDX__CONTENTHELPERS__PAIR_KEY_EXTRACTOR__

namespace idx { namespace contenthelpers {

template<typename PairLikeType>
struct PairKeyExtractor {
	using KeyType = decltype(std::declval<PairLikeType>().first);

	inline KeyType operator()(PairLikeType const &value) const {
		return value.first;
	}
};

} }

#endif