project(hot-test-helpers)

set(NAMESPACE hot/testhelpers)

set(TEST_HELPERS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TEST_HELPERS_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HDR_NAMESPACE ${TEST_HELPERS_INCLUDE_DIRECTORY}/${NAMESPACE})
set(SRC_NAMESPACE ${TEST_HELPERS_SOURCE_DIRECTORY}/${NAMESPACE})

include_directories(include)

if(USEAVX2)

set(test-helper-HEADERS ${test-helper-HEADERS} ${HDR_NAMESPACE}/PartialKeyMappingTestHelper.hpp)
set(test-helper-HEADERS ${test-helper-HEADERS} ${HDR_NAMESPACE}/SampleTriples.hpp)

set(test-helper-SOURCES ${test-helper-SOURCES} ${SRC_NAMESPACE}/PartialKeyMappingTestHelper.cpp)
set(test-helper-SOURCES ${test-helper-SOURCES} ${SRC_NAMESPACE}/SampleTriples.cpp)
endif()

add_library(hot-test-helpers-lib ${test-helper-SOURCES} ${test-helper-HEADERS})
target_include_directories(hot-test-helpers-lib PUBLIC "${TEST_HELPERS_INCLUDE_DIRECTORY}")
target_link_libraries(hot-test-helpers-lib hot-commons-lib  Boost::unit_test_framework)
