#!/bin/bash
cd "${0%/*}"
rm ../src/generated/*
../thirdparty/protobuf/cmake/build/x86-Release/Release/protoc.exe -I. -I../thirdparty/protobuf/src/ --cpp_out=../src/generated ./*.proto