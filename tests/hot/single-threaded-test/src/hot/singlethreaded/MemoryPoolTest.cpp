//
//  @author robert.binna@uibk.ac.at
//

#include <iostream>

#include <hot/singlethreaded/MemoryPool.hpp>
#include <boost/test/unit_test.hpp>

namespace hot { namespace singlethreaded {

struct DummyElement {
	uint64_t mMember;
	static size_t sNumberContructorCalls;
	static size_t sNumberDestructorCalls;

	DummyElement() : mMember(0u) {
		++sNumberContructorCalls;
	}

	~DummyElement() {
		++sNumberDestructorCalls;
	}

	static void reset() {
		sNumberContructorCalls = 0;
		sNumberDestructorCalls = 0;
	}
};

class MemoryPoolTestFixture {
public:
	//before test
	MemoryPoolTestFixture() {
		DummyElement::reset();
	}

	//after test
	~MemoryPoolTestFixture() {
		DummyElement::reset();
	}
};

size_t DummyElement::sNumberContructorCalls = 0u;
size_t DummyElement::sNumberDestructorCalls = 0u;

BOOST_FIXTURE_TEST_SUITE(MemoryPoolTest, MemoryPoolTestFixture)

BOOST_AUTO_TEST_CASE(testAllocAndDestruction)
{
	MemoryPool<DummyElement, 10>* pool = new MemoryPool<DummyElement, 10>();

	size_t numberConstructorCalls = 0u;
	for(size_t i=1u; i < 10u; ++i) {
		void* rawMemory = pool->alloc(i);
		numberConstructorCalls+=i;
		BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), i);
		pool->returnToPool(i, rawMemory);
		void* rawMemory2 = pool->alloc(i);
		pool->returnToPool(i, rawMemory2);
		BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), i);
	}
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 0u);
	delete pool;
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), pool->getNumberAllocations());
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 9u);
}

BOOST_AUTO_TEST_CASE(testEviction) {
	MemoryPool<DummyElement, 10, 20, 8>* pool = new MemoryPool<DummyElement, 10, 20, 8>();

	size_t numberConstructorCalls = 0u;
	for(size_t i=1u; i < 10u; ++i) {
		void* rawMemory = pool->alloc(i);
		BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), i);
		pool->returnToPool(i, rawMemory);
		void* rawMemory2 = pool->alloc(i);
		pool->returnToPool(i, rawMemory2);
		BOOST_REQUIRE_EQUAL(rawMemory2, rawMemory);
		BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), i);
	}

	std::array<void*, 20> intermediateMemory;
	intermediateMemory[0] = pool->alloc(4);
	for(size_t i=1u; i < 20u; ++i) {
		intermediateMemory[i] = pool->alloc(4);
		numberConstructorCalls += 4;
	}
	BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), 28u);
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 0u);

	for(size_t i=0u; i < 19u; ++i) {
		pool->returnToPool(4, intermediateMemory[i]);
	}
	BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), 28u);
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 0u);

	pool->returnToPool(4, intermediateMemory[19]);
	BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), 28u);
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 12u);
	delete pool;
	BOOST_REQUIRE_EQUAL(pool->getNumberAllocations(), 28u);
	BOOST_REQUIRE_EQUAL(pool->getNumberFrees(), 28u);
}

BOOST_AUTO_TEST_SUITE_END()

}}

