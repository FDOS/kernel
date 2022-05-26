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

cd test
if ! ./test.sh ../_output/gcc/KGC${KVER}.sys diskgc bootgc 'boot gcc: '
then
  echo GCC boot test failed
  exit 2
fi
if ! ./test.sh ../_output/wc/KWC${KVER}.sys diskwc bootwc 'boot wc: '
then
  echo OpenWatcom boot test failed
  exit 2
fi
cd ..
exit 0
