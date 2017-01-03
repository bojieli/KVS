#!/bin/bash

cp src/kernels.cl src/kernels.cpp;
g++ -g -lrt -lboost_system -lboost_thread src/kernels.cpp -o bin/kernels.out;
rm src/kernels.cpp;
