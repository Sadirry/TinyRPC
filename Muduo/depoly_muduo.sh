#!/bin/bash

set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

if [ ! -d /usr/include/Muduo ]; then 
    mkdir /usr/include/Muduo
fi

cd `pwd`/include

for header in `ls *.h`
do
    cp $header /usr/include/Muduo
done

cd ..
cp `pwd`/lib/libMuduo.so /usr/lib

ldconfig