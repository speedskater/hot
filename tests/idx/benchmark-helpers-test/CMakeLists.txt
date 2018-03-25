project(benchmark-helpers-test)

include_directories(include)

set(NAMESPACE idx/benchmark)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventExecutorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventReaderTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventReaderExceptionTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventListBackingBufferSizeEstimatorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventListTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventTypeIdTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkEventTypeIdToEventTypeMappingTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/BenchmarkLineReaderTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ContentReaderTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ContinuosBufferTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TempFile.cpp)

set(test-HEADERS ${test-HEADERS} ${HDR_NAMESPACE}/TempFile.hpp)

add_boost_test(EXECUTABLE benchmark-helpers-test SOURCES ${test-SOURCES} ${test-HEADERS})
target_link_libraries(benchmark-helpers-test benchmark-helpers-lib)
