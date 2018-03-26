#!/bin/bash
CURRENT_DIRECTORY=$(pwd)
SOURCE_ROOT_DIRECTORY=$(readlink -f $( dirname ${BASH_SOURCE[0]} ))
rm -rf $SOURCE_ROOT_DIRECTORY/coverage-build
mkdir $SOURCE_ROOT_DIRECTORY/coverage-build
cd $SOURCE_ROOT_DIRECTORY/coverage-build
cmake ../ -DCOVERAGE=ON -DCMAKE_BUILD_TYPE=Debug "$@"
cd tests
make
make CTEST_OUTPUT_ON_FAILURE=1 test
testReturnCode=$?
mkdir $SOURCE_ROOT_DIRECTORY/coverageReport
cd $SOURCE_ROOT_DIRECTORY/coverageReport
lcov -o overall_coverage.info -c -d $SOURCE_ROOT_DIRECTORY/coverage-build
lcov --extract overall_coverage.info "${SOURCE_ROOT_DIRECTORY}/libs/**/*" --output-file libraries_coverage.info
genhtml -o . libraries_coverage.info
cd $CURRENT_DIRECTORY
exit ${testReturnCode}
