project(map-helpers-test)

set(NAMESPACE idx/maphelpers)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

include_directories(${Boost_INCLUDE_DIR} include)
link_directories ( ${Boost_LIBRARY_DIRS} )

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/MapValueExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/STLLikeIndexTest.cpp)

add_executable(map-helpers-test ${test-SOURCES} ${test-HEADERS})
target_link_libraries(map-helpers-test map-helpers-lib content-helpers-lib   Boost::unit_test_framework)

if(COVERAGE)
    SETUP_TARGET_FOR_COVERAGE(
            map-helpers-test-coverage  # Name for custom target.
            map-helpers-test-test    # Name of the test driver executable that runs the tests.
            # NOTE! This should always have a ZERO as exit code
            # otherwise the coverage generation will not complete.
            coverage            # Name of output directory.
    )
endif()