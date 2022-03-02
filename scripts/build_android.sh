#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

build_abi() {
    ABI=$1
    NDK=/Users/lewisou/Library/Android/sdk/ndk-bundle
    MINSDKVERSION=23

    mkdir -p $SCRIPT_DIR/../build/android/$ABI
    cd $SCRIPT_DIR/../build/android/$ABI

    cmake \
        -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$ABI \
        -DANDROID_PLATFORM=android-$MINSDKVERSION \
        $SCRIPT_DIR/..

    make
}

abis=(arm64-v8a armeabi-v7a x86 x86_64)
for abi in ${abis[@]}
do
    echo build $abi
    build_abi $abi
done