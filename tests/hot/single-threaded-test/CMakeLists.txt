project(hot-single-threaded-test)

set(NAMESPACE hot/singlethreaded)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

if(USEAVX2)
set(single-threaded-test-SOURCES ${single-threaded-test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(single-threaded-test-SOURCES ${single-threaded-test-SOURCES} ${SRC_NAMESPACE}/MemoryPoolTest.cpp)
set(single-threaded-test-SOURCES ${single-threaded-test-SOURCES} ${SRC_NAMESPACE}/HOTSingleThreadedNodeTest.cpp)
set(single-threaded-test-SOURCES ${single-threaded-test-SOURCES} ${SRC_NAMESPACE}/HOTSingleThreadedTest.cpp)
endif()

add_boost_test(EXECUTABLE hot-single-threaded-test SOURCES ${single-threaded-test-SOURCES})
target_include_directories(hot-single-threaded-test PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(hot-single-threaded-test content-helpers-lib hot-commons-lib utils-lib hot-single-threaded-lib hot-test-helpers-lib)