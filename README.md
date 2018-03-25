# What is HOT
HOT stands for Height Optimized Trie.
It is an order preserving index structure which facilitates a dynamic span to reduce the overall tree height.
This makes it highly suitable for general purpose workloads where the distribution of the data is not known in advance, and
non uniformly distributed data can be expected

Its structure and algorithmic details can be found in:

HOT: A Height Optimized Trie Index for Main-Memory Database Systems
by Binna, Zangerle, Pichl, Specht and Leis

# Content

This implementation of HOT is written in C++14 and can be built with the cmake build system.
It is a cleaned up version of the code used in the original HOT publication.
This libraries contains two different implementations of HOT.
    * A single threaded version supporting insert, lookup, scan and deletion operations.
    * A concurrent version using a ROWEX (Read-Optimized Write EXclusion) synchronization strategy. The concurrent implementation currently onyl supports insert, lookup and scan operatoins

Additionally this library contains tools for creating different variations of these two index structures.
Contained are tools for integers and c-strings. But using the provided helper structures custom variations can be created.

Further more this library contains a simple benchmarking framework for index structures to build benchmarking applications for strings and integers.
Based on this benchmarking framework 4 benchmark applications are provided with this library which can be used to evaluate the peformance of the single and the concurrent HOT index.

# Getting started

1. Clone the repository int a subdirectory labeld 'hot' by calling

``` git clone https://github.com/speedskater/hot.git hot```

2. Changing the working directory to the currently checked out repository

```cd hot```

3. Initialize the submodules with
```git submodule update --init --recursive ```

4. Running the provided tests suits

To run the unit test suit execute:

```./runTests.sh```

The result of each individual test is written to stdout
After running the unit tests the coverage report can be found in the subfolder `coverageReport`

5. Build all provided benchmarks and tests in release mode execute:

HOT uses CMake as build system, and can therefore be build like any standard CMake project.
However, for the sake of simplicity we provide a the script `./releaseBuild.sh` which creates the directory release-build sets the build mode to release and
builds all included binaries.

It you want to build the provided benchmarks with support for measuring CPU performance counters, either specify the property "USE_COUNTERS" to "ON" in your CMake build
or pass `-DUSE_COUNTERS=ON` to `./releaseBuild.sh`


9. Running the benchmark applications

To run the benchmark applications set the current working directory to the `release-build` folder.
Choose the desired benchmark binary.

For string benchmarks the binaries are:
    * Concurrent HOT: ```./apps/benchmarks/strings/hot-rowex-string-benchmark/hot-rowex-string-benchmark```
    * Single threaded HOT ```./apps/benchmarks/strings/hot-single-threaded-string-benchmark/hot-single-threaded-string-benchmark```

For integer benchmarks the binaries are:
    *  Concurrent HOT: ```./apps/benchmarks/integer/hot-rowex-integer-benchmark/hot-rowex-integer-benchmark```
    *  Single threaded HOT ```./apps/benchmarks/integer/hot-single-threaded-integer-benchmark/hot-single-threaded-integer-benchmark```

The parameters which are required to run the benchmarks are listed by the binaries usage dialogs which can be invoke like this:

    ```<benchmark-binary> -help```

For instance to benchmark the performance of single threaded HOT for 50 mio randomly distributed integers, with random lookup invoke the corresponding
benchmark application as follows.

    ```./apps/benchmarks/integer/hot-single-threaded-integer-benchmark/hot-single-threaded-integer-benchmark -insert=random -size=50000000 -insertModifier=random -lookup=random```

# Requirements

To compile and use this library the following requirements must be met:
 * x86-64 CPU supporting at least the AVX-2 and BMI-2 instruction sets (Haswell and newer)
 * A C++14 compliant compiler
 * CMake in version 2.8 or newer

# Integrating HOT into your own project

HOT is designed as a header only library. In case of the single threaded version no external dependencies except the source code contained in this project are required.
For the concurrent implementation the intel thread building blocks as well as a fast allocator like tcmalloc is required.

To integrate HOT into your project you can either copy the following source directories right into your project:
    - libs/idx/content-helper (helper functions to encode tuple identifiers, convert keys to their binary representation and so on.)
    - libs/hot/commons (code shared by the concurrent and the single threaded version of HOT)
    - libs/hot/rowex (the concurrent implementation of HOT)
    - libs/hot/single-threaded (the single threaded version of HOT)

or directly include the HOT root CMakeLists.txt into your own project.

# Limitations

The provided implementations currently have the following limitations:

    * Deletion operations are not supported by the concurrent implementation of HOT
    * The length of the keys are restricted to 255 bytes
    * The maximum length of the supported tuple identifiers and therefore the keys which can directly be embedded into the indexes are restricted to 63 bits.