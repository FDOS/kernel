#!/bin/sh

set -e

if [ -z "${BUILD_DIR}" ] ; then
  BUILD_DIR=$(pwd)
fi
echo BUILD_DIR is \"${BUILD_DIR}\"

# Output directory
rm -rf _output
mkdir _output

# GCC
mkdir _output/gcc
git clean -x -d -f -e _output -e _watcom -e ow-snapshot.tar.gz
make -C country clean
make all COMPILER=gcc
mv -n bin/KGC*.map bin/KGC*.sys _output/gcc/.
mv -n bin/country.sys _output/gcc/.
# GCC share
(cd share && make clobber && env COMPILER=gcc ./build.sh)
mv -n share/share.com _output/gcc/.
mv -n share/share.map _output/gcc/.

# Watcom
if [ ! -d _watcom ] ; then
  [ -f ow-snapshot.tar.gz ] || wget --quiet https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.gz

  mkdir _watcom
  (cd _watcom && tar -xf ../ow-snapshot.tar.gz)
fi

export PATH=$BUILD_DIR/bin:$PATH:$BUILD_DIR/_watcom/binl64
export WATCOM=$BUILD_DIR/_watcom

mkdir _output/wc
git clean -x -d -f -e _output -e _watcom -e ow-snapshot.tar.gz
make -C country clean
make all COMPILER=owlinux
mv -n bin/KWC*.map bin/KWC*.sys _output/wc/.
mv -n bin/country.sys _output/wc/.

echo done
