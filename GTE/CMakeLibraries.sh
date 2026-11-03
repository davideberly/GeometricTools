#!/bin/bash
# usage: ./CMakeLibraries.sh BUILD_TYPE LIBRARY_TYPE
# where BUILD_TYPE is in {Debug,Release}
# and where LIBRARY_TYPE is in {Static,Shared}

BUILD_TYPE=$1
LIBRARY_TYPE=$2

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
        cmake --preset gcc_Debug -DBUILD_SHARED_LIBS:BOOL=FALSE -DGTE_BUILD_SAMPLES:BOOL=FALSE
    else
        cmake --preset gcc_Debug -DBUILD_SHARED_LIBS:BOOL=TRUE -DGTE_BUILD_SAMPLES:BOOL=FALSE
    fi
    cmake --build ./build --config Debug --target all --
else
    if [ "${LIBRARY_TYPE}" = "Static" ]; then
        cmake --preset gcc_Release -DBUILD_SHARED_LIBS:BOOL=FALSE -DGTE_BUILD_SAMPLES:BOOL=FALSE
    else
        cmake --preset gcc_Release -DBUILD_SHARED_LIBS:BOOL=TRUE -DGTE_BUILD_SAMPLES:BOOL=FALSE
    fi
    cmake --build ./build --config Release --target all --
fi
