add_executable(hot-single-threaded-integer-benchmark "src/main.cpp")
target_link_libraries(hot-single-threaded-integer-benchmark hot-single-threaded-lib benchmark-helpers-lib content-helpers-lib)
