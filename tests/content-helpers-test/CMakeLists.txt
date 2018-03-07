project(content-helpers-test)

if(COVERAGE)
    include(CodeCoverage)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
endif()

set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost COMPONENTS unit_test_framework test_exec_monitor REQUIRED)

set(NAMESPACE spider/contenthelpers)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

include_directories(${Boost_INCLUDE_DIR} include)
link_directories ( ${Boost_LIBRARY_DIRS} )

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ContentEqualsTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/IdentityKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/OptionalValueTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/PairKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/PairPointerKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/CStringComparatorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/KeyComparatorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ValueToKeyTypeMapperTest.cpp)


add_executable(content-helpers-test ${test-SOURCES} ${test-HEADERS})
target_link_libraries(content-helpers-test content-helpers-lib ${Boost_LIBRARIES} boost_unit_test_framework boost_test_exec_monitor)

if(COVERAGE)
    SETUP_TARGET_FOR_COVERAGE(
            content-helpers-test-coverage  # Name for custom target.
            content-helpers-test    # Name of the test driver executable that runs the tests.
            # NOTE! This should always have a ZERO as exit code
            # otherwise the coverage generation will not complete.
            coverage            # Name of output directory.
    )
endif()