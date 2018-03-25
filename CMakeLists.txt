cmake_minimum_required(VERSION 2.8)

PROJECT(HOT_ROOT)

option(USEAVX2 "USEAVX2" ON)
option(USETRAVIS "USETRAVIS" OFF)
option(USEAVX512 "USEAVX512" OFF)
option(USE_COUNTERS "USE_COUNTERS" OFF)
option(COVERAGE "COVERAGE" OFF)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bilke-cmake-modules/")

if(USETRAVIS)
	SET(CMAKE_CXX_FLAGS "-std=c++14 -march=haswell -Wno-deprecated-declarations -Wall -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -faligned-new") #-fstack-check -fsanitize=undefined ${CMAKE_CXX_FLAGS}
elseif(USEAVX2)
	SET(CMAKE_CXX_FLAGS "-std=c++14 -march=native -mavx -mavx2 -mbmi2 -mlzcnt -Wno-deprecated-declarations -Wall -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -faligned-new") #-fstack-check -fsanitize=undefined ${CMAKE_CXX_FLAGS}
	SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
else()
	SET(CMAKE_CXX_FLAGS "-std=c++14 -march=native -Wno-deprecated-declarations -Wall ${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
endif()

if(USEAVX512)
	SET(CMAKE_CXX_FLAGS "-mavx512f -mavx512vl -mavx512bw -mavx512dq -mavx512cd ${CMAKE_CXX_FLAGS}")
	add_definitions(-DUSE_AVX512)
endif()

if(USE_COUNTERS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_COUNTERS=1")
endif(USE_COUNTERS)

set(TC_MALLOC_NAMES tcmalloc tcmalloc_minimal libtcmalloc_minimal.so.4.3.0 )

find_library(TC_MALLOC_LIBRARY
	NAMES ${TC_MALLOC_NAMES}
	PATHS /usr/lib/x86_64-linux-gnu /usr/lib64 /lib /usr/lib /usr/local/lib /opt/local/lib
	NO_DEFAULT_PATH
)

SET(TBB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party/tbb")
include(${TBB_ROOT}/cmake/TBBBuild.cmake)
tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS)
find_package(TBB REQUIRED)

SET(Hana_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third-party/boost-hana")

IF(COVERAGE)
	#call with -DCOVERAGE=1
	set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage")

	include(CodeCoverage)
	message("Code Coverage Activated")
ENDIF(COVERAGE)

ENABLE_TESTING()

ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})

add_subdirectory(third-party/boost-cmake)

ADD_SUBDIRECTORY(apps)
ADD_SUBDIRECTORY(libs)
ADD_SUBDIRECTORY(tests)
