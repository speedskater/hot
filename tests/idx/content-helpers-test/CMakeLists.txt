project(content-helpers-test)

include_directories(include)

set(NAMESPACE idx/contenthelpers)
set(SRC_NAMESPACE src/${NAMESPACE})
set(HDR_NAMESPACE include/${NAMESPACE})

set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/TestModule.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ContentEqualsTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/IdentityKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/OptionalValueTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/PairKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/PairPointerKeyExtractorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/CStringComparatorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/KeyComparatorTest.cpp)
set(test-SOURCES ${test-SOURCES} ${SRC_NAMESPACE}/ValueToKeyTypeMapperTest.cpp)

add_boost_test(EXECUTABLE content-helpers-test SOURCES ${test-SOURCES} ${test-HEADERS})
target_link_libraries(content-helpers-test content-helpers-lib)