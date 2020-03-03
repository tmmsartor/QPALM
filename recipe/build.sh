#!/bin/bash

export MINICONDA_LIB=${HOME}/.local/opt/miniconda/lib
export MINICONDA_INCLUDE=${HOME}/.local/opt/miniconda/include

curdir=`pwd`

#Build direcetories
mkdir -p build
mkdir -p build/debug
mkdir -p build/lib
mkdir -p build/metis

metisdir=$curdir/build/metis
cd $metisdir

cmake $curdir/suitesparse/metis-5.1.0 -DGKLIB_PATH=$curdir/suitesparse/metis-5.1.0/GKlib -DSHARED=1 && make
cd $curdir
cp build/metis/libmetis/libmetis.so build/lib/

#Build QPALM and tests
cd $curdir

builddir=$curdir/build/debug

cd $builddir

cmake $curdir -DCMAKE_BUILD_TYPE=release -DCOVERAGE=ON
make
