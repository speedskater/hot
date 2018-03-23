#define BOOST_TEST_DYN_LINK

#include <array>
#include <cstdint>
#include <iostream>

#include <boost/test/unit_test.hpp>
#include <hot/commons/UsedEntriesMask256.hpp>

namespace hot { namespace commons {

BOOST_AUTO_TEST_SUITE(UsedEntriesMask256Test)

uint8_t numberEntriesToPartial(size_t numberEntries) {
	assert(numberEntries < 8);
	uint8_t partialMask = 0u;
	switch(numberEntries) {
		case 0:
			partialMask = 0;
			break;
		case 1:
			partialMask = 0b0000'0001;
			//partialMask = 0b1000'0000;
			break;
		case 2:
			partialMask = 0b0000'0011;
			//partialMask = 0b1100'0000;
			break;
		case 3:
			partialMask = 0b0000'0111;
			//partialMask = 0b1110'0000;
			break;
		case 4:
			partialMask = 0b0000'1111;
			//partialMask = 0b1111'0000;
			break;
		case 5:
			partialMask = 0b0001'1111;
			//partialMask = 0b1111'1000;
			break;
		case 6:
			partialMask = 0b0011'1111;
			//partialMask = 0b1111'1100;
			break;
		case 7:
			partialMask = 0b0111'1111;
			//partialMask = 0b1111'1110;
			break;
		default:
			assert(false);
	}
	return partialMask;
}

BOOST_AUTO_TEST_CASE(testConvertNumbeEntriesToEntriesMask256) {
	for(size_t i=1; i < 256; ++i) {
		std::array<uint8_t, 32> result;
		SIMDHelper<256>::store(UsedEntriesMask256::createForNumberEntries(i), result.data());

		size_t partialMaskByteIndex = i/8;
		for(size_t j=0; j < partialMaskByteIndex; ++j) {
			BOOST_REQUIRE_EQUAL(result[j], 0xFFu);
		}
		if(partialMaskByteIndex < 32) {
			BOOST_REQUIRE_EQUAL(result[partialMaskByteIndex], numberEntriesToPartial(i%8));
		}
		for(size_t j = partialMaskByteIndex + 1; j < 32; ++j) {
			BOOST_REQUIRE_EQUAL(result[j], 0u);
		}
	}
}

void expectEquality(__m256i actualDataRegister, std::array<uint8_t, 32> const & expectedData) {
	std::array<uint8_t, 32> actualRawData;
	SIMDHelper<256>::store(actualDataRegister, actualRawData.data());
	BOOST_REQUIRE_EQUAL_COLLECTIONS(actualRawData.begin(), actualRawData.end(), expectedData.begin(), expectedData.end());
}

void expectEquality(__m256i actualDataRegister, __m256i expectedDataRegister) {
	std::array<uint8_t, 32> actualRawData;
	std::array<uint8_t, 32> expectedRawData;
	SIMDHelper<256>::store(actualDataRegister, actualRawData.data());
	SIMDHelper<256>::store(expectedDataRegister, expectedRawData.data());
	BOOST_REQUIRE_EQUAL_COLLECTIONS(actualRawData.begin(), actualRawData.end(), expectedRawData.begin(), expectedRawData.end());
}

std::array<uint8_t, 32> getZeroedRawArray() {
	std::array<uint8_t, 32> rawArray;
	std::fill(rawArray.begin(), rawArray.end(), 0);
	return rawArray;
};

BOOST_AUTO_TEST_CASE(testCreateSingleEntry) {
	std::array<uint8_t, 32> singleEntryOnIndex0 = {
	//7         0     13      8     23     14    31      24     39     32     47     40     55     48     63     56
		0b0000'0001u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//71       64     79     72     87     80    95      88     103    96     111   104     119   112     127   120
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//135     128     143   136     151   144    159    152     167   160     175   168     183   176     191   184
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//199     192     207   200     215   208    223    216     231   224     239   232     247   240     255   248
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u
	};
	expectEquality(UsedEntriesMask256::createSingleEntryMask(0), singleEntryOnIndex0);

	std::array<uint8_t, 32> singleEntryOnIndex7 = {
	//7         0     13      8     23     14    31      24     39     32     47     40     55     48     63     56
		0b1000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//71       64     79     72     87     80    95      88     103    96     111   104     119   112     127   120
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//135     128     143   136     151   144    159    152     167   160     175   168     183   176     191   184
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//199     192     207   200     215   208    223    216     231   224     239   232     247   240     255   248
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u
	};
	expectEquality(UsedEntriesMask256::createSingleEntryMask(7), singleEntryOnIndex7);

	std::array<uint8_t, 32> singleEntryOnIndex248 = {
	//7         0     13      8     23     14    31      24     39     32     47     40     55     48     63     56
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//71       64     79     72     87     80    95      88     103    96     111   104     119   112     127   120
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//135     128     143   136     151   144    159    152     167   160     175   168     183   176     191   184
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//199     192     207   200     215   208    223    216     231   224     239   232     247   240     255   248
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0001u
	};
	expectEquality(UsedEntriesMask256::createSingleEntryMask(248), singleEntryOnIndex248);

	std::array<uint8_t, 32> singleEntryOnIndex255 = {
	//7         0     13      8     23     14    31      24     39     32     47     40     55     48     63     56
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//71       64     79     72     87     80    95      88     103    96     111   104     119   112     127   120
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//135     128     143   136     151   144    159    152     167   160     175   168     183   176     191   184
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//199     192     207   200     215   208    223    216     231   224     239   232     247   240     255   248
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b1000'0000u
	};
	expectEquality(UsedEntriesMask256::createSingleEntryMask(255), singleEntryOnIndex255);

	std::array<uint8_t, 32> singleEntryOnIndex153 = {
	//7       0       13      8     23     14    31      24     39     32     47     40     55     48     63     56
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//71       64     79     72     87     80    95      88     103    96     111   104     119   112     127   120
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//135   128       143   136     151   144    159    152     167   160     175   168     183   176     191   184
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0010u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u,
	//199   192       207   200     215   208    223    216     231   224     239   232     247   240     255   248
		0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u, 0b0000'0000u
	};
	expectEquality(UsedEntriesMask256::createSingleEntryMask(153), singleEntryOnIndex153);

}

void setSingleEntryBitInRawArray(std::array<uint8_t, 32> & arrayToModify, size_t bitIndex) {
	size_t byteIndex = bitIndex/8;
	size_t byteRelativeIndex = bitIndex%8;
	uint8_t byteRelativeMask = 0b1000'0000u >> (7 - byteRelativeIndex);
	arrayToModify[byteIndex] = arrayToModify[byteIndex] | byteRelativeMask;
}

__m256i getMaskForIndexes(std::initializer_list<uint32_t> indexes) {
	__m256i mask = SIMDHelper<256>::zero();
	for(uint32_t index : indexes) {
		mask = UsedEntriesMask256::setUsedEntry(mask, index);
	}
	return mask;
}

BOOST_AUTO_TEST_CASE(testGetFirstUsedEntry) {
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(getMaskForIndexes({ 0, 5, 120, 137, 255 })), 0u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(getMaskForIndexes({ 5, 120, 137, 255 })), 5u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(getMaskForIndexes({ 5, 7, 120, 137, 255 })), 5u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(getMaskForIndexes({120, 137, 255 })), 120u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(getMaskForIndexes({255 })), 255u);
}

BOOST_AUTO_TEST_CASE(testGetLastUsedEntry) {
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(getMaskForIndexes({ 0, 5, 120, 137, 255 })), 255u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(getMaskForIndexes({ 5, 120, 137, 253, 255 })), 255u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(getMaskForIndexes({ 5, 7, 120, 137 })), 137u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(getMaskForIndexes({ 0, 5, 7 })), 7u);
	BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(getMaskForIndexes({ 0 })), 0u);
}

BOOST_AUTO_TEST_CASE(testCreateSingleEntryMask2) {
	for(size_t i=0; i < 256; ++i) {
		std::array<uint8_t, 32> expectedData = getZeroedRawArray();
		setSingleEntryBitInRawArray(expectedData, i);
		__m256i singleEntryMask = UsedEntriesMask256::createSingleEntryMask(i);
		expectEquality(singleEntryMask, expectedData);
		BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getFirstUsedEntry(singleEntryMask), i);
		BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getLastUsedEntry(singleEntryMask), i);
	}
}

BOOST_AUTO_TEST_CASE(testSetUsedEntry) {
	std::array<uint8_t, 32> expectedData = getZeroedRawArray();
	__m256i currentUsedEntriesMask = SIMDHelper<256>::zero();

	for(size_t i=0; i < 256; ++i) {
		setSingleEntryBitInRawArray(expectedData, i);
		currentUsedEntriesMask = UsedEntriesMask256::setUsedEntry(currentUsedEntriesMask, i);
		expectEquality(currentUsedEntriesMask, expectedData);
		BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getNumberUsedEntries(currentUsedEntriesMask), i + 1);
	}
}

BOOST_AUTO_TEST_CASE(testRemoveUsedEntry) {
	__m256i currentlyUsedEntriesMask = UsedEntriesMask256::createForNumberEntries(256);
	for(int i=255; i >= 0; --i) {
		currentlyUsedEntriesMask = UsedEntriesMask256::removeUsedEntry(currentlyUsedEntriesMask, static_cast<uint32_t>(i));
		if(i == 0) {
			expectEquality(currentlyUsedEntriesMask, SIMDHelper<256>::zero());
			BOOST_REQUIRE(UsedEntriesMask256::isEmpty(currentlyUsedEntriesMask));
		} else {
			expectEquality(currentlyUsedEntriesMask, UsedEntriesMask256::createForNumberEntries(static_cast<uint32_t>(i)));
			BOOST_REQUIRE(!UsedEntriesMask256::isEmpty(currentlyUsedEntriesMask));
		}
		BOOST_REQUIRE_EQUAL(UsedEntriesMask256::getNumberUsedEntries(currentlyUsedEntriesMask), static_cast<uint32_t>(i));
	}

}

//createFor(range(from, to))
//createForListOfEntries({ 1, 2, 3, 4, 6, 7, 8, 9, 10 });
//testFirst
//testLast
//test...

BOOST_AUTO_TEST_SUITE_END()

}}