#! /bin/bash

KVER=8632

if [ ! -f _output/gcc/KGC${KVER}.sys ] ; then
  echo GCC built kernel not present
  exit 1
fi

if [ ! -f _output/wc/KWC${KVER}.sys ] ; then
  echo Watcom built kernel not present
  exit 1
fi

echo GCC and Watcom kernels have all been built
find _output -ls
exit 0
