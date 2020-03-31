#!/bin/bash

export CMAKE_BUILD_TYPE=Release

curdir=`pwd`
builddir=$curdir/build/$CMAKE_BUILD_TYPE

# Build direcetories
mkdir -p build
mkdir -p build/$CMAKE_BUILD_TYPE
mkdir -p build/lib


pushd $builddir
cmake $curdir -DCMAKE_BUILD_TYPE=release -DCOVERAGE=ON && make
popd
