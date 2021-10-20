#!/bin/bash
# usage: ./CMakeLibraries.sh BUILD_TYPE LIBRARY_TYPE
# where BUILD_TYPE is in {Debug,Release}
# and where LIBRARY_TYPE is in {Static,Shared}

BUILD_TYPE=$1
LIBRARY_TYPE=$2
GCC=gcc
GXX=g++

if [[ ! "${BUILD_TYPE}" = "Debug" && ! "${BUILD_TYPE}" = "Release" ]]; then
    echo "Invalid build type: ${BUILD_TYPE}, must be in {Debug, Release}"
    exit 1
fi

if [[ ! "${LIBRARY_TYPE}" = "Static" && ! "${LIBRARY_TYPE}" = "Shared" ]]; then
    echo "Invalid library type: ${LIBRARY_TYPE}, must be in {Static, Shared}"
    exit 2
fi

if [ "${BUILD_TYPE}" = "Debug" ]; then
    if [ "${LIBRARY_TYPE}" = "Static" ]; then
        cmake -DBUILD_SAMPLES=ON -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_RELEASE_LIB:BOOL=FALSE -DBUILD_SHARED_LIB:BOOL=FALSE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    else
        cmake -DBUILD_SAMPLES=ON -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_RELEASE_LIB:BOOL=FALSE -DBUILD_SHARED_LIB:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    fi
    cmake --build ./build --config Debug --target all -- -j 10
else
    if [ "${LIBRARY_TYPE}" = "Static" ]; then
        cmake -DBUILD_SAMPLES=ON -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_RELEASE_LIB:BOOL=TRUE -DBUILD_SHARED_LIB:BOOL=FALSE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    else
        cmake -DBUILD_SAMPLES=ON -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_RELEASE_LIB:BOOL=TRUE -DBUILD_SHARED_LIB:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    fi
    cmake --build ./build --config Release --target all -- -j 10
fi
