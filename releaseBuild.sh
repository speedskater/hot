#!/bin/sh
CURRENT_DIRECTORY=$(pwd)
SOURCE_ROOT_DIRECTORY=$(realpath $( dirname ${BASH_SOURCE[0]} ))
rm -rf $SOURCE_ROOT_DIRECTORY/release-build
mkdir $SOURCE_ROOT_DIRECTORY/release-build
cd $SOURCE_ROOT_DIRECTORY/release-build
cmake ../ -DCMAKE_BUILD_TYPE=Release "$@"
make
