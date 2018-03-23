project(hot-rowex-test)

set(NAMESPACE hot/rowex)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

if(USEAVX2)

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/HOTRowexNodeTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/HOTRowexTest.cpp)
set(test-HEADERS ${test-HEADERS} ${SRC_NAMESPACE}/StringTestData.cpp)

set(test-HEADERS ${test-HEADERS} ${HDR_NAMESPACE}/ConcurrentTestHelper.hpp)
set(test-HEADERS ${test-HEADERS} ${HDR_NAMESPACE}/TreeTestHelper.hpp)
set(test-HEADERS ${test-HEADERS} ${HDR_NAMESPACE}/StringTestData.hpp)

endif()


add_executable(hot-rowex-test ${test-SOURCES}  ${test-HEADERS})
target_include_directories(hot-rowex-test  PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(hot-rowex-test content-helpers-lib utils-lib hot-single-threaded-lib hot-rowex-lib hot-test-helpers-lib hot-commons-lib   Boost::unit_test_framework TBB::tbb)

if(COVERAGE)
SETUP_TARGET_FOR_COVERAGE(
        hot-rowex-test-coverage  # Name for custom target.
        hot-rowex-test    # Name of the test driver executable that runs the tests.
        # NOTE! This should always have a ZERO as exit code
        # otherwise the coverage generation will not complete.
        coverage            # Name of output directory.
)
endif()