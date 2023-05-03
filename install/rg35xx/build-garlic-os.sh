#!/usr/bin/env bash
set -euo pipefail

declare -r BUILD_DIR=build-rg35xx-garlic
declare -r CXXFLAGS="-mcpu=cortex-a9 -mtune=cortex-a9 -march=armv7-a -marm -mfpu=neon -mfloat-abi=softfp -DNO_STACK_TRACE=1 -DGAMEPAD_ENABLED=1"

cd ../../

if [ ${@,,} == "clean" ]; then
    echo Cleaning $BUILD_DIR
    rm -rf $BUILD_DIR
fi

if ! [ -d "$BUILD_DIR" ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR

cmake -DCMAKE_CXX_FLAGS="$CXXFLAGS" ../

re='^[0-9]+$'
if ! [[ $@ =~ $re ]]; then
    make
else
    make -j$@
fi

if [ -d "ROMS" ]; then
    rm -rf ROMS
fi
if [ -f "README.txt" ]; then
    rm README.txt
fi
if [ -f "openxcom-rg35xx-garlic.zip" ]; then
    rm openxcom-rg35xx-garlic.zip
fi
cd ../install/rg35xx/garlicos-skeleton
cp -r ROMS ../../../$BUILD_DIR
cp -r README.txt ../../../$BUILD_DIR
cd ../../../$BUILD_DIR/bin
mv * ../ROMS/PORTS/openxcom
cd ../
zip -r openxcom-rg35xx-garlic.zip ROMS
zip -r openxcom-rg35xx-garlic.zip README.txt

