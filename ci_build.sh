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
git clean -x -d -f -e test -e _output -e _downloads -e _watcom
make -C country clean
make all COMPILER=gcc
mv -n bin/KGC*.map bin/KGC*.sys _output/gcc/.
mv -n bin/country.sys _output/gcc/.
# GCC share
(
  cd share
  git submodule update --init --recursive
  git clean -x -d -f
  env COMPILER=gcc ./build.sh
)
mv -n share/src/share.com _output/gcc/.
mv -n share/src/share.map _output/gcc/.

# Watcom
OWTAR=ow-snapshot.tar.xz
if [ ! -d _watcom ] ; then
  mkdir -p _downloads
  mkdir _watcom
  if [ ! -f _downloads/$OWTAR ] ; then
    (cd _downloads && wget --no-verbose https://github.com/open-watcom/open-watcom-v2/releases/download/Last-CI-build/$OWTAR)
  fi
  (cd _watcom && tar -xf ../_downloads/$OWTAR)
fi

export WATCOM=$BUILD_DIR/_watcom
export PATH=$BUILD_DIR/bin:$PATH:$WATCOM/binl64

mkdir _output/wc
git clean -x -d -f -e test -e _output -e _downloads -e _watcom
make -C country clean
make all COMPILER=owlinux
mv -n bin/KWC*.map bin/KWC*.sys _output/wc/.
mv -n bin/country.sys _output/wc/.

## DOS (GCC)
#mkdir _output/gcc_dos
#git clean -x -d -f -e test -e _output -e _downloads -e _watcom
#{
#  echo set COMPILER=GCC
#  echo set MAKE=make
#  echo set XCPU=386
#  echo set XFAT=32
#  echo set XNASM='C:\\devel\\nasm\\nasm'
#  echo set OLDPATH=%PATH%
#  echo set PATH='C:\\devel\\i16gnu\\bin;C:\\bin;%OLDPATH%'
#} | unix2dos > config.bat

#dosemu -td -q -K . -E "build.bat"

# DOS (Watcom)
mkdir _output/wc_dos
git clean -x -d -f -e test -e _output -e _downloads -e _watcom
{
  echo set COMPILER=WATCOM
  echo set WATCOM='C:\\devel\\watcomc'
  echo set MAKE=wmake /ms
  echo set XCPU=386
  echo set XFAT=32
  echo set XNASM='C:\\devel\\nasm\\nasm'
  echo set XUPX=upx --8086 --best
  echo set OLDPATH=%PATH%
  echo set PATH='%WATCOM%\\binw;C:\\bin;%OLDPATH%'
  echo set DOS4G=QUIET
} | unix2dos > config.bat

dosemu -td -q -K . -E "build.bat"
mv -n bin/KWC*.map bin/KWC*.sys _output/wc_dos/.
mv -n bin/country.sys _output/wc_dos/.


# DOS (Turbo C 2.01)
if [ -d ${HOME}/.dosemu/drive_c/tc201 ] ; then
  mkdir _output/tc_dos
  git clean -x -d -f -e test -e _output -e _downloads -e _watcom
  {
    echo set COMPILER=TC2
    echo set TC2_BASE='C:\\tc201'
    echo set MAKE=make
    echo set XCPU=386
    echo set XFAT=32
    echo set XNASM=nasm
    echo set OLDPATH=%PATH%
    echo set PATH='%TC2_BASE%;C:\\devel\\nasm;C:\\bin;%OLDPATH%'
  } | unix2dos > config.bat

  dosemu -td -q -K . -E "build.bat lfn"
  mv -n bin/KTC*.map bin/KTC*.sys _output/tc_dos/.
  mv -n bin/country.sys _output/tc_dos/.
  # TC share
  (
    cd share
    git submodule update --init --recursive
    git clean -x -d -f
    env COMPILER=tcc2-emu ./build.sh
  )
  mv -n share/src/share.com _output/tc_dos/.
  mv -n share/src/share.map _output/tc_dos/.
fi

echo done
