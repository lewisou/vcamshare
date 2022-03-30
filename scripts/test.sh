#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
cd $SCRIPT_DIR

./build_share_local.sh

if [ $? -eq 0 ]
then
    cd $SCRIPT_DIR/../build/local
    ./test
fi
