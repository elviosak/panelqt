#!/bin/bash

mkdir -p build
rm -r build/*
cd build
qmake .. && make && sudo make install