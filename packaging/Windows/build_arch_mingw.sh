#! /usr/bin/bash

QT_CMAKE=/usr/x86_64-w64-mingw32/static/lib/qt6/bin/qt-cmake
SRC_DIR=../..
BUILD_ROOT=$SRC_DIR/build
BUILD_DIR=$BUILD_ROOT/Mingw-Release

$QT_CMAKE -DCMAKE_BUILD_TYPE="Release" -G Ninja -B $BUILD_DIR $SRC_DIR
cmake --build $BUILD_DIR
strip -s $BUILD_DIR/StateS.exe
