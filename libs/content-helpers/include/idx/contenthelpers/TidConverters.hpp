#ifndef __IDX__CONTENTHELPERS__TID_CONVERTERS__HPP__
#define __IDX__CONTENTHELPERS__TID_CONVERTERS__HPP__

#include <cstdint>

namespace idx { namespace contenthelpers {


template<typename ValueType>
class TidToValueConverter {
public:
	__attribute__((always_inline)) inline ValueType operator()(intptr_t tid) {
		tid &= INTPTR_MAX;
		return *reinterpret_cast<ValueType *>(&tid);
	}
};

template<typename ValueType>
class TidToValueConverter<ValueType *> {
public:
	__attribute__((always_inline)) inline ValueType *operator()(intptr_t tid) {
		return reinterpret_cast<ValueType *>(tid);
	}
};

template<typename ValueType>
class ValueToTidConverter {
public:
	__attribute__((always_inline)) inline intptr_t operator()(ValueType value) {
		return *reinterpret_cast<intptr_t *>(&value);
	}
};

template<typename ValueType>
class ValueToTidConverter<ValueType *> {
public:
	__attribute__((always_inline)) inline intptr_t operator()(ValueType *value) {
		return reinterpret_cast<intptr_t>(value);
	}
};

template<typename ValueType>
__attribute__((always_inline)) inline ValueType tidToValue(intptr_t tid) {
	TidToValueConverter<ValueType> convert;
	return convert(tid);
}

template<typename ValueType>
__attribute__((always_inline)) inline intptr_t valueToTid(ValueType value) {
	ValueToTidConverter<ValueType> convert;
	return convert(value);
}

} }

#endif