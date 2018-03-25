//
//  Created by Robert Binna
//

#include <boost/test/unit_test.hpp>

#include <cstdlib>
#include <string>
#include <sstream>
#include <utility>

#include <idx/benchmark/ContinuosBuffer.hpp>

namespace idx { namespace benchmark {

BOOST_AUTO_TEST_SUITE(ContinuosBufferTest)

BOOST_AUTO_TEST_CASE(testConstruction) {
	void* availableMemory = reinterpret_cast<void*>(12345u);
	idx::benchmark::ContinuousBuffer buffer(availableMemory, 168);

	BOOST_REQUIRE_EQUAL(buffer.mRemainingBufferSize, 168u);
	BOOST_REQUIRE_EQUAL(buffer.mRemainingBuffer, availableMemory);
}
	
BOOST_AUTO_TEST_CASE(testAlign) {
	void* availableMemory = reinterpret_cast<void*>(1u);
	idx::benchmark::ContinuousBuffer buffer(availableMemory, 256u);
	const ContinuousBuffer &alignedBufferChar = buffer.template align<char>();
	const ContinuousBuffer &alignedBufferUint32 = buffer.template align<uint32_t>();
	const ContinuousBuffer &alignedBufferUint64 = buffer.template align<uint64_t>();

	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(alignedBufferChar.mRemainingBuffer), 1u);
	BOOST_REQUIRE_EQUAL(alignedBufferChar.mRemainingBufferSize, 256u);
	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(alignedBufferUint32.mRemainingBuffer), 4u);
	BOOST_REQUIRE_EQUAL(alignedBufferUint32.mRemainingBufferSize, 256u - 3u);
	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(alignedBufferUint64.mRemainingBuffer), 8u);
	BOOST_REQUIRE_EQUAL(alignedBufferUint64.mRemainingBufferSize, 256u - 7u);
}

BOOST_AUTO_TEST_CASE(testAlignExpectException) {
	void* availableMemory = reinterpret_cast<void*>(1u);
	idx::benchmark::ContinuousBuffer buffer(availableMemory, 7u);
	const ContinuousBuffer &alignedBufferChar = buffer.template align<char>();
	const ContinuousBuffer &alignedBufferUint32 = buffer.template align<uint32_t>();

	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(alignedBufferChar.mRemainingBuffer), 1u);
	BOOST_REQUIRE_EQUAL(alignedBufferChar.mRemainingBufferSize, 7u);
	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(alignedBufferUint32.mRemainingBuffer), 4u);
	BOOST_REQUIRE_EQUAL(alignedBufferUint32.mRemainingBufferSize, 4u);

	BOOST_REQUIRE_THROW( buffer.template align<uint64_t>(), std::bad_alloc );
}

BOOST_AUTO_TEST_CASE(testAdvance) {
	void* availableMemory = reinterpret_cast<void*>(123446u);
	idx::benchmark::ContinuousBuffer buffer(availableMemory, 2048u);
	idx::benchmark::ContinuousBuffer const & nextBuffer = buffer.advance(194u);

	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(nextBuffer.mRemainingBuffer), 123640u);
	BOOST_REQUIRE_EQUAL(nextBuffer.mRemainingBufferSize, 2048u - 194u);

	idx::benchmark::ContinuousBuffer const & nextBuffer2 = buffer.advance(2048u);

	BOOST_REQUIRE_EQUAL(reinterpret_cast<uintptr_t>(nextBuffer2.mRemainingBuffer), 125494u);
	BOOST_REQUIRE_EQUAL(nextBuffer2.mRemainingBufferSize, 0u);
}

BOOST_AUTO_TEST_CASE(testAdvanceExpectThrowable) {
	void* availableMemory = reinterpret_cast<void*>(123446u);
	idx::benchmark::ContinuousBuffer buffer(availableMemory, 194u);

	BOOST_REQUIRE_THROW( buffer.advance(195u), std::bad_alloc );
}

BOOST_AUTO_TEST_CASE(testAllocate) {
	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer(backingBuffer, 2048u);
	const std::pair<char *, ContinuousBuffer> &singleChar = buffer.allocate<char>();
	const std::pair<uint64_t*, ContinuousBuffer> & singleLongLong = singleChar.second.template allocate<uint64_t>();

	BOOST_REQUIRE(singleChar.first == backingBuffer);
	BOOST_REQUIRE(singleLongLong.first == reinterpret_cast<uint64_t*>(backingBuffer + 8));
	//one byte char and alignment 8 for 8 byte data type
	BOOST_REQUIRE_EQUAL(singleLongLong.second.mRemainingBufferSize, 2048u - 16u);
}

BOOST_AUTO_TEST_CASE(testAllocateExpectThrowable) {
	char backingBuffer[2048];
	idx::benchmark::ContinuousBuffer buffer(backingBuffer, 9u);
	const std::pair<char *, ContinuousBuffer> &singleChar = buffer.allocate<char>();
	BOOST_REQUIRE_THROW(singleChar.second.template allocate<uint64_t>(), std::bad_alloc);
}

BOOST_AUTO_TEST_SUITE_END()

}}