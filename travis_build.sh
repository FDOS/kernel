#!/bin/sh

set -e

if [ -z "${TRAVIS_BUILD_DIR}" ] ; then
  TRAVIS_BUILD_DIR=$(pwd)
fi
echo TRAVIS_BUILD_DIR is \"${TRAVIS_BUILD_DIR}\"

# Output directory
rm -rf _output
mkdir _output

# GCC
git clean -x -d -f -e _output -e _watcom -e ow-snapshot.tar.gz
make all COMPILER=gcc
mv -i bin/KGC*.map bin/KGC*.sys _output/.

# Watcom
if [ ! -d _watcom ] ; then
  [ -f ow-snapshot.tar.gz ] || wget https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.gz

  mkdir _watcom
  (cd _watcom && tar -xf ../ow-snapshot.tar.gz)
fi

export PATH=$TRAVIS_BUILD_DIR/bin:$PATH:$TRAVIS_BUILD_DIR/_watcom/binl64
export WATCOM=$TRAVIS_BUILD_DIR/_watcom

git clean -x -d -f -e _output -e _watcom -e ow-snapshot.tar.gz
make all COMPILER=owlinux
mv -i bin/KWC*.map bin/KWC*.sys _output/.
