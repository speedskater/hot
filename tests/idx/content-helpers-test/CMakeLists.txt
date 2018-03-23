project(content-helpers-test)

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

add_executable(content-helpers-test ${test-SOURCES} ${test-HEADERS})
target_link_libraries(content-helpers-test content-helpers-lib  Boost::unit_test_framework)

if(COVERAGE)
    SETUP_TARGET_FOR_COVERAGE(
      content-helpers-test-coverage  # Name for custom target.
      content-helpers-test    # Name of the test driver executable that runs the tests.
      # NOTE! This should always have a ZERO as exit code
      # otherwise the coverage generation will not complete.
      coverage            # Name of output directory.
    )
endif()