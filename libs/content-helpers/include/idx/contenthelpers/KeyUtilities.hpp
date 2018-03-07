#ifndef __IDX__CONTENTHELPERS__KEY_UTILITIES__HPP__
#define __IDX__CONTENTHELPERS__KEY_UTILITIES__HPP__

#include <array>
#include <cstdint>
#include <cstring>

namespace idx { namespace contenthelpers {

template<typename KeyType> inline auto toBigEndianByteOrder(KeyType const & key) {
	return key;
};

template<> __attribute__((always_inline)) inline auto toBigEndianByteOrder<uint64_t>(uint64_t const & key) {
	return __bswap_64(key);
};

template<> inline auto toBigEndianByteOrder<uint32_t>(uint32_t const & key) {
	return __bswap_32(key);
};

template<> inline auto toBigEndianByteOrder<uint16_t>(uint16_t const & key) {
	return __bswap_16(key);
};

template<typename KeyType> constexpr inline __attribute__((always_inline)) size_t getMaxKeyLength() {
	return sizeof(KeyType);
}

constexpr size_t MAX_STRING_KEY_LENGTH = 255;
template<> constexpr inline size_t getMaxKeyLength<char const *>() {
	return MAX_STRING_KEY_LENGTH;
}

template<typename KeyType> inline size_t getKeyLength(KeyType const & key) {
	return getMaxKeyLength<KeyType>();
}

template<> inline size_t getKeyLength<char const *>(char const * const & key) {
	return std::min<size_t>(strlen(key) + 1u, MAX_STRING_KEY_LENGTH);
}

template<typename KeyType> inline __attribute__((always_inline)) auto toFixSizedKey(KeyType const & key) {
	return key;
}

template<> inline auto toFixSizedKey(char const * const & key) {
	std::array<uint8_t, getMaxKeyLength<char const *>()> fixedSizeKey;
	strncpy(reinterpret_cast<char*>(fixedSizeKey.data()), key, MAX_STRING_KEY_LENGTH);
	return fixedSizeKey;
}

//Be aware that this pointer is only valid as long as keyType is valid!!
template<typename KeyType> __attribute__((always_inline)) inline uint8_t const * interpretAsByteArray(KeyType const & keyType) {
	return reinterpret_cast<uint8_t const *>(&keyType);
}

template<> inline uint8_t const* interpretAsByteArray<const char*>(const char * const & cStringKey) {
	return reinterpret_cast<uint8_t const *>(cStringKey);
}

}}

#endif