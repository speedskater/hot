#!/bin/bash
CURRENT_DIRECTORY=$(pwd)
SOURCE_ROOT_DIRECTORY=$(realpath $( dirname ${BASH_SOURCE[0]} ))
rm -rf $SOURCE_ROOT_DIRECTORY/coverage-build
mkdir $SOURCE_ROOT_DIRECTORY/coverage-build
cd $SOURCE_ROOT_DIRECTORY/coverage-build
cmake ../ -DCOVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cd tests
make
testReturnCode=$(make test)
mkdir $SOURCE_ROOT_DIRECTORY/coverageReport
cd $SOURCE_ROOT_DIRECTORY/coverageReport
lcov -o overall_coverage.info -c -d $SOURCE_ROOT_DIRECTORY/coverage-build
lcov --extract overall_coverage.info "${SOURCE_ROOT_DIRECTORY}/libs/**/*" --output-file liberaries_coverage.info
genhtml -o . liberaries_coverage.info 
cd $CURRENT_DIRECTORY
exit testReturnCode
