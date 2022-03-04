#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

build_android_abi() {
    ABI=$1
    NDK=/Users/lewisou/Library/Android/sdk/ndk-bundle
    MINSDKVERSION=23

    mkdir -p $SCRIPT_DIR/../build/android/$ABI
    cd $SCRIPT_DIR/../build/android/$ABI

    cmake \
        -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$ABI \
        -DANDROID_PLATFORM=android-$MINSDKVERSION \
        -DPLATFORM=android \
        $SCRIPT_DIR/..

    make
}

build_android() {
    abis=(arm64-v8a armeabi-v7a x86 x86_64)
    for abi in ${abis[@]}
    do
        echo build $abi
        build_android_abi $abi
    done
}

build_ios() {
    BUILD_DIR=$SCRIPT_DIR/../build/ios/

    mkdir -p $BUILD_DIR
    cd $BUILD_DIR

    cmake ../.. -GXcode -DCMAKE_SYSTEM_NAME=iOS -DPLATFORM=ios
}

build_android
build_ios
