#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"

export ANDROID_SDK_ROOT=/Users/lewisou/Library/Android/sdk
export ANDROID_NDK_ROOT=/Users/lewisou/Library/Android/sdk/ndk/22.0.7026061
# 22.0.7026061

build_android() {
    cd $SCRIPT_DIR/../depts/ffmpeg-kit/

    ./android.sh --lts --speed --no-archive

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/android/armeabi-v7a/
    mkdir -p $TARGET_DIR
    cp ./prebuilt/android-arm-lts/ffmpeg/lib/*.so $TARGET_DIR

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/android/arm64-v8a/
    mkdir -p $TARGET_DIR
    cp ./prebuilt/android-arm64-lts/ffmpeg/lib/*.so $TARGET_DIR

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/android/x86/
    mkdir -p $TARGET_DIR
    cp ./prebuilt/android-x86-lts/ffmpeg/lib/*.so $TARGET_DIR

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/android/x86_64/
    mkdir -p $TARGET_DIR
    cp ./prebuilt/android-x86_64-lts/ffmpeg/lib/*.so $TARGET_DIR

    TARGET_INCLUDE_DIR=$SCRIPT_DIR/../libs/ffmpeg/
    mkdir -p $TARGET_INCLUDE_DIR
    cp -r ./prebuilt/android-arm-lts/ffmpeg/include $TARGET_INCLUDE_DIR
}

build_ios() {
    cd $SCRIPT_DIR/../depts/ffmpeg-kit/

    ./ios.sh --lts --speed

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/ios/
    rm -rf $TARGET_DIR
    mkdir -p $TARGET_DIR
    mv ./prebuilt/bundle-apple-framework-ios-lts/*.framework $TARGET_DIR
}

build_macos() {
    cd $SCRIPT_DIR/../depts/ffmpeg-kit/

    ./macos.sh --lts --speed

    TARGET_DIR=$SCRIPT_DIR/../libs/ffmpeg/macos/
    rm -rf $TARGET_DIR
    mkdir -p $TARGET_DIR/x86_64/
    mkdir -p $TARGET_DIR/arm64/

    cp ./prebuilt/apple-macos-x86_64-lts/ffmpeg/lib/*.a $TARGET_DIR/x86_64/
    cp ./prebuilt/apple-macos-arm64-lts/ffmpeg/lib/*.a $TARGET_DIR/arm64/
}

build_ios
build_android
build_macos