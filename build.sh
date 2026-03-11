#!/bin/bash

set -e # Means if any command fails it will exit

parallel=true
build_parallel="-DBUILD_PARALLEL="

print_help() {
	echo "Build accepts the following arguments
-p [true/false]                           Builds parallel or serial
"
}

while getopts p:h flag
do
    case "${flag}" in
        p) parallel=${OPTARG};;
        h) print_help ; exit 0;;
    esac
done

cmake -H. -G Ninja -Bbuild-metal-Debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_SANITIZED=1 -DBUILD_PARALLEL=$parallel -DENABLE_RTTI=1 &&
cmake --build build-metal-Debug --config Debug -j16 && ./build-metal-Debug/spv_test 2