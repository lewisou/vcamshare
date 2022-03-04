#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BUILD_DIR=$SCRIPT_DIR/../build/local/

mkdir -p $BUILD_DIR
cd $BUILD_DIR

cmake ../..
make
