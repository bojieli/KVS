#!/bin/bash

PRJ=../../hashtable;

mkdir $1;
cd $1;
ln -sf $PRJ/compile.sh .;
ln -sf $PRJ/inc .;
mkdir bin;
mkdir src;
cd src;
ln -sf ../$PRJ/src/chan .;
ln -sf ../$PRJ/src/dma .;
ln -sf ../$PRJ/src/hashtable .;
ln -sf ../$PRJ/src/pcie .;
ln -sf ../$PRJ/src/sim .;
ln -sf ../$PRJ/src/slab .;
cp ../$PRJ/src/kernels.cl .;
