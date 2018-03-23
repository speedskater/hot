project(benchmark-helpers-test)

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

set(test-HEADERS ${test-SOURCES} ${HDR_NAMESPACE}/TempFile.hpp)

add_executable(benchmark-helpers-test ${test-SOURCES} ${test-HEADERS})
target_link_libraries(benchmark-helpers-test benchmark-helpers-lib Boost::unit_test_framework)

if(COVERAGE)
    SETUP_TARGET_FOR_COVERAGE(
            benchmark-helpers-test-coverage  # Name for custom target.
            benchmark-helpers-test    # Name of the test driver executable that runs the tests.
            # NOTE! This should always have a ZERO as exit code
            # otherwise the coverage generation will not complete.
            coverage            # Name of output directory.
    )
endif()