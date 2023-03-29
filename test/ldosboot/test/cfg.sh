#! /bin/bash

# Usage of the works is permitted provided that this
# instrument is retained with the works, so that any entity
# that uses the works is notified of this instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

# If you want to override configuration without the
# hassle of having to exclude differences in this file
# (cfg.sh) from the SCM, you may provide ovr.sh.
# The meaning of this line allows you to copy cfg.sh
# to serve as a template for ovr.sh without changes.
[ -f ovr.sh ] && [[ "${BASH_SOURCE[0]##*/}" != ovr.sh ]] && . ovr.sh

# As the below only are set if no value is provided
# yet, any value can be overridden from the shell.
[ -z "$LMACROS_DIR" ] && LMACROS_DIR=../../lmacros/
[ -z "$LDOSBOOT_DIR" ] && LDOSBOOT_DIR=../
[ -z "$SCANPTAB_DIR" ] && SCANPTAB_DIR=../../scanptab/
[ -z "$INICHECK_DIR" ] && INICHECK_DIR=../../crc16-t/
[ -z "$BOOTIMG_DIR" ] && BOOTIMG_DIR=../../bootimg/
[ -z "$LDEBUG_DIR" ] && LDEBUG_DIR=../../ldebug/bin/
[ -z "$LDOSMBR_DIR" ] && LDOSMBR_DIR=../../ldosmbr/
[ -z "$INSTSECT_DIR" ] && INSTSECT_DIR=../../instsect/

[ -z "$DOSEMU" ] && DOSEMU=dosemu
[ -z "$QEMU" ] && QEMU=qemu-system-i386
[ -z "$DEFAULT_MACHINE" ] && DEFAULT_MACHINE=dosemu
[ -z "$SENDKEYS" ] && SENDKEYS=sendkeys
[ -z "$BOOT_KERNEL" ] && BOOT_KERNEL=~/.dosemu/drive_c/kernel.sys
[ -z "$BOOT_COMMAND" ] && BOOT_COMMAND=~/.dosemu/drive_c/command.com
[ -z "$BOOT_PROTOCOL" ] && BOOT_PROTOCOL=FREEDOS
[ -z "$BOOT_OPTIONS" ] && BOOT_OPTIONS=" "
[ -z "$MKTMPINC" ] && MKTMPINC=mktmpinc.pl
[ -z "$NASM" ] && NASM=nasm
[ -z "$CHECKSUM" ] && CHECKSUM="${INICHECK_DIR%/}"/iniload/checksum

[ -z "$use_build_inicheck" ] && use_build_inicheck=0
