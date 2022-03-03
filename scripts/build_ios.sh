#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DIR=$SCRIPT_DIR/../build/ios/

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake ../.. -GXcode -DCMAKE_SYSTEM_NAME=iOS
