project(hot-commons-test)

set(NAMESPACE hot/commons)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

if(USEAVX2)

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/AlgorithmsTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/DiscriminativeBitTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/SIMDHelperTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/MultiMaskPartialKeyMappingTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/SingleMaskPartialKeyMappingTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/SparsePartialKeysTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/SparsePartialKeys256Test.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/UsedEntriesMask256Test.cpp)

endif()


add_executable(hot-commons-test ${test-SOURCES}  ${test-HEADERS})
target_include_directories(hot-commons-test  PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(hot-commons-test content-helpers-lib utils-lib hot-commons-lib hot-test-helpers-lib Boost::unit_test_framework pthread TBB::tbb)

if(COVERAGE)
SETUP_TARGET_FOR_COVERAGE(
        hot-commons-test-coverage  # Name for custom target.
        hot-commons-test    # Name of the test driver executable that runs the tests.
        # NOTE! This should always have a ZERO as exit code
        # otherwise the coverage generation will not complete.
        coverage            # Name of output directory.
)
endif()