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
[ -z "$LMACROS_DIR" ] && LMACROS_DIR=lmacros/
[ -z "$LDOSBOOT_DIR" ] && LDOSBOOT_DIR=ldosboot/
[ -z "$BOOTIMG_DIR" ] && BOOTIMG_DIR=bootimg/

[ -z "$QEMU" ] && QEMU=qemu-system-i386
[ -z "$NASM" ] && NASM=nasm
