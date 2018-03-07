#ifndef __IDX__CONTENTHELPERS__IDENTITY_KEY_EXTRACTOR__
#define __IDX__CONTENTHELPERS__IDENTITY_KEY_EXTRACTOR__

namespace idx { namespace contenthelpers {

template<typename ValueType>
struct IdentityKeyExtractor {
	typedef ValueType KeyType;

	inline KeyType operator()(ValueType const &value) const {
		return value;
	}
};

} }

#endif