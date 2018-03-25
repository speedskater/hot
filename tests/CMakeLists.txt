function(add_boost_test)
  set(oneValueArgs EXECUTABLE)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(add_boost_test "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
  set(TEST_SOURCE_FILES ${add_boost_test_SOURCES})
  set(EXECUTABLE_NAME ${add_boost_test_EXECUTABLE})
  foreach(testSourceFile ${TEST_SOURCE_FILES})
    file(READ "${testSourceFile}" SOURCE_FILE_CONTENTS)
    string(REGEX MATCHALL "BOOST_AUTO_TEST_SUITE\\([^)]+\\)"
      TEST_SUITE_LINE ${SOURCE_FILE_CONTENTS}
      )
    if(TEST_SUITE_LINE)
      string(REGEX REPLACE ".*\\(([^)]+)\\).*" "\\1" TEST_SUITE ${TEST_SUITE_LINE})

      string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\([^)]+\\)"
        FOUND_TESTS ${SOURCE_FILE_CONTENTS})


      foreach(HIT ${FOUND_TESTS})
        string(REGEX REPLACE ".*\\(([^)]+)\\).*" "\\1" TEST_NAME ${HIT})
        add_test(NAME "${TEST_SUITE}.${TEST_NAME}"
          COMMAND ${EXECUTABLE_NAME}
          --run_test=${TEST_SUITE}/${TEST_NAME} --catch_system_error=yes)
      endforeach()
    endif()
  endforeach()
  add_executable(${EXECUTABLE_NAME} ${TEST_SOURCE_FILES})
  target_link_libraries(${EXECUTABLE_NAME} Boost::unit_test_framework)
  if(COVERAGE)
    target_link_libraries(${EXECUTABLE_NAME} gcov)
  endif()
endfunction()

ADD_SUBDIRECTORY(idx)
ADD_SUBDIRECTORY(hot)