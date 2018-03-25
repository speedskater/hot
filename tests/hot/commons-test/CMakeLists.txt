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

endif()

add_boost_test(EXECUTABLE hot-commons-test SOURCES ${test-SOURCES} ${test-HEADERS})
target_include_directories(hot-commons-test  PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(hot-commons-test content-helpers-lib utils-lib hot-commons-lib hot-test-helpers-lib pthread TBB::tbb)