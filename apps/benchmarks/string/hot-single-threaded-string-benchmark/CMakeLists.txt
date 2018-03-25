add_executable(hot-single-threaded-strings-benchmark "src/main.cpp")
target_link_libraries(hot-single-threaded-strings-benchmark hot-single-threaded-lib benchmark-helpers-lib content-helpers-lib)
