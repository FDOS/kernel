#!/bin/sh

set -e

sudo add-apt-repository -y ppa:tkchia/build-ia16
sudo add-apt-repository -y ppa:dosemu2/ppa
sudo apt update

# for cross building
sudo apt install gcc-ia16-elf libi86-ia16-elf nasm upx qemu-system-x86 mtools util-linux bash

# for building with DOS using an emulator
sudo apt install dosemu2 dos2unix
#    Perhaps later we should build using Freecom from published package

mkdir -p _downloads
cd _downloads

HERE=$(pwd)

#IBIBLIO_PATH='http://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.2/repos'
IBIBLIO_PATH='https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/1.3'

BASE=${IBIBLIO_PATH}/base

#    get FreeDOS kernel
[ -f kernel.zip ] || wget --no-verbose ${BASE}/kernel.zip

#    get FreeCOM
[ -f freecom.zip ] || wget --no-verbose ${BASE}/freecom.zip

DEVEL=${IBIBLIO_PATH}/devel

#    get gnumake for DOS
[ -f djgpp_mk.zip ] || wget --no-verbose ${DEVEL}/djgpp_mk.zip

#    get nasm for DOS
[ -f nasm.zip ] || wget --no-verbose ${DEVEL}/nasm.zip

#    get upx for DOS
[ -f upx.zip ] || wget --no-verbose ${DEVEL}/upx.zip

#    grab ia16-gcc from ibiblio.org
#[ -f i16gcc.zip ] || wget --no-verbose ${DEVEL}/i16gcc.zip
#[ -f i16newli.zip ] || wget --no-verbose ${DEVEL}/i16newli.zip
#[ -f i16butil.zip ] || wget --no-verbose ${DEVEL}/i16butil.zip
#[ -f i16lbi86.zip ] || wget --no-verbose ${DEVEL}/i16lbi86.zip

#   get watcom for DOS
[ -f watcomc.zip ] || wget --no-verbose ${DEVEL}/watcomc.zip

mkdir -p ${HOME}/.dosemu/drive_c
cd ${HOME}/.dosemu/drive_c && (

  mkdir -p bin

  # Boot files
  unzip -L -q ${HERE}/kernel.zip
  cp -p bin/kernl386.sys ./kernel.sys
  unzip -L -q ${HERE}/freecom.zip
  cp -p bin/command.com ./command.com
  cp -p /usr/share/dosemu/dosemu2-cmds-0.3/c/fdconfig.sys .

  # Development files
  unzip -L -q ${HERE}/djgpp_mk.zip
  cp -p devel/djgpp/bin/make.exe bin/.
  unzip -L -q ${HERE}/upx.zip
  cp -p devel/upx/upx.exe bin/.
  echo PATH to make and upx binaries is 'c:/bin'

  unzip -L -q ${HERE}/nasm.zip
  echo PATH to nasm binary is 'c:/devel/nasm'

#  unzip -L -q ${HERE}/i16gcc.zip
#  unzip -L -q ${HERE}/i16newli.zip
#  unzip -L -q ${HERE}/i16butil.zip
#  unzip -L -q ${HERE}/i16lbi86.zip
#  echo PATH to ia16 binaries is 'c:/devel/i16gnu/bin'

  unzip -L -q ${HERE}/watcomc.zip
  echo PATH to watcom binaries is 'c:/devel/watcomc/binw'
)
