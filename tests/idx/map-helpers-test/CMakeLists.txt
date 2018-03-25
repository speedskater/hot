project(map-helpers-test)

set(NAMESPACE idx/maphelpers)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

include_directories(${Boost_INCLUDE_DIR} include)
link_directories ( ${Boost_LIBRARY_DIRS} )

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/MapValueExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/STLLikeIndexTest.cpp)

add_boost_test(EXECUTABLE map-helpers-test SOURCES ${test-SOURCES} ${test-HEADERS})
target_link_libraries(map-helpers-test map-helpers-lib content-helpers-lib)