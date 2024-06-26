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

cp -r `pwd`/src/include `pwd`/lib

