#! /bin/bash

KVER=8632

if [ ! -f _output/gcc/kgc${KVER}.sys ] ; then
  echo GCC built kernel not present
  exit 1
fi

if [ ! -f _output/wc/kwc${KVER}.sys ] ; then
  echo Watcom built kernel not present
  exit 1
fi

if [ ! -f _output/wc_dos/kwc38632.sys ] ; then
  echo Watcom DOS built kernel not present
  exit 1
fi

if [ ! -f _output/tc_dos/ktc8632.sys ] && [ -d ${HOME}/.dosemu/drive_c/tc201 ] ; then
  echo Turbo C 2.01 built kernel not present
  exit 1
fi

echo Kernels have all been built
find _output -ls

cd test
if ! ./test.sh ../_output/gcc/kgc${KVER}.sys diskgc bootgc 'boot gcc: '
then
  echo GCC boot test failed
  exit 2
fi
if ! ./test.sh ../_output/wc/kwc${KVER}.sys diskwc bootwc 'boot wc: '
then
  echo OpenWatcom boot test failed
  exit 2
fi
if ! ./test.sh ../_output/wc_dos/kwc38632.sys diskwcd bootwcd 'boot wcd: '
then
  echo 'OpenWatcom(DOS) boot test failed'
  exit 2
fi
if [ -d ${HOME}/.dosemu/drive_c/tc201 ] ; then
  if ! ./test.sh ../_output/tc_dos/ktc8632.sys disktcd boottcd 'boot tcd: '
  then
    echo 'Turbo C 2.01 boot test failed'
    exit 2
  fi
fi
cd ..
exit 0
