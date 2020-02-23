#!/bin/sh

KVER=8632

if [ ! -f _output/KGC${KVER}.sys ] ; then
  echo GCC built kernel not present
  exit 1
fi

if [ ! -f _output/KWC${KVER}.sys ] ; then
  echo Watcom built kernel not present
  exit 1
fi

echo GCC and Watcom kernels have all been built
ls -l _output/K*
exit 0
