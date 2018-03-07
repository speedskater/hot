#ifndef __IDX__CONTENTHELPERS__OPTIONAL_VALUE__HPP__
#define __IDX__CONTENTHELPERS__OPTIONAL_VALUE__HPP__

namespace idx { namespace contenthelpers {

template<typename ValueType> struct OptionalValue{
	bool mIsValid;
	ValueType mValue;

	inline OptionalValue() : mIsValid(false) {
	}

	inline OptionalValue(bool const & isValid, ValueType const & value) : mIsValid(isValid), mValue(value) {
	}

	inline bool compliesWith(OptionalValue const & expected) const {
		return (this->mIsValid == expected.mIsValid) & (!this->mIsValid || (mValue == expected.mValue));
	}
};

}}

#endif