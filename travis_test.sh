#!/bin/sh

if [ -f kernel/kernel.sys ] ; then
  echo Kernel has been built
  exit 0
fi

exit 1
