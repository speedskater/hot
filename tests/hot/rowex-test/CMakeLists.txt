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

add_boost_test(EXECUTABLE hot-rowex-test SOURCES ${test-SOURCES} ${test-HEADERS})
target_include_directories(hot-rowex-test  PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(hot-rowex-test content-helpers-lib utils-lib hot-single-threaded-lib hot-rowex-lib hot-test-helpers-lib hot-commons-lib TBB::tbb)