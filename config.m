#
# config.m - FreeDOS Kernel build configuration
#
# Copy this file to "config.mak" before building then edit it
# for your development environment and desired default options. 
#
# It only has local preferences / overrides.  Leave options blank 
# or commented out to use the defaults.
#
# Options can also be overridden on the command line, e.g.
#   make COMPILER=gcc XCPU=386 XFAT=16
#

# =====================================================================
# Compiler selection
# =====================================================================

# Target compiler (used for building DOS artifacts: kernel, sys, ...)
#   watcom            Open Watcom C/C++   [default]
#   gcc  (gcc-ia16)   ia16-elf-gcc        [*]
#   borland           Borland C++  (bcc)
#   msc  (msvc)       Microsoft C  (cl)   [*]
# Leave blank to default to watcom.
# [*] unless HOST is DOS, requires host compatible compiler for
#     building utilities needed during build process, i.e. system
#     gcc if gcc-ia16 used and msvc [win32 or win64] for msc.
COMPILER=

# Use Turbo C (tcc) instead of Borland C (bcc) when COMPILER=borland.
# Set to 1 for Turbo C, leave blank for Borland C++.
USE_TURBO=

# =====================================================================
# Target configuration
# =====================================================================

# Target CPU: 86, 186, 286 or 386.   Default is 86
XCPU=

# FAT filesystem support: 16 or 32.  Default is 32
## FAT12 & FAT16 always enabled, 32 enables also FAT32 support
## which adds some MSDOS 7.x structural changes
XFAT=

# Kernel load segment (hex).         Default is 0x60
## this should not be changed unless your specific system has
## an incompatiblity with this address; this will effect general
## compatibility with any programs that use hard coded addresses
## but the kernel itself has no issues as long as has enough
## free memory above load address for relocation and allocations.
LOADSEG=

# =====================================================================
# Tools
# =====================================================================

# NASM assembler executable.         Default is nasm
## Note that using protected mode DJGPP version of nasm may cause
## crashes on Windows NT/2k/XP if called by other protected mode 
## tools such as Borland's make
XNASM=

# UPX kernel compression.  Set to the full UPX command to enable, e.g.
#   XUPX=upx --8086 --best
#   XUPX=upx --lzma
# Leave blank / commented out to disable UPX.
XUPX=

# =====================================================================
# Compiler installation paths
# =====================================================================
# Only set these if the tools are not already on your PATH.

# --- Open Watcom ---  (default: ~/watcom on Linux, C:/WATCOM otherwise)
#WATCOM=/home/user/watcom
#WATCOM=C:/WATCOM

# --- GCC ia16-elf-gcc ---  (leave blank if ia16-elf-gcc is on PATH)
# Note: system gcc (for build tools only) assumed to be in path as gcc
#GCC_BASE=/usr/local/ia16-elf

# --- Borland C++ (COMPILER=borland, USE_TURBO blank) ---
#BC_BASE=C:/BC5
#BC_BASE=d:/bc30
#BC_BASE=d:/borlandc

# --- Turbo C (COMPILER=borland, USE_TURBO=1) ---
#TC_BASE=C:/TC
## Turbo C/C++ 3.0
#TC_BASE=d:/tc30
## Turbo C++ 1
#TC_BASE=d:\tc101
## Turbo C 2
#TC_BASE=d:/tc201

# --- Microsoft C (COMPILER=msc) ---
#MSC_BASE=C:/MSC
#MSC_BASE=d:/C600

# --- Microsoft Visual C/C++ (for build tools only) ---
#MSVC_BASE=C:/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/


# =====================================================================
# (optionally) which linker to use: (defaults determined automatically)
# =====================================================================

# --- Turbo Link ---
## WARNING TLINK needs to be in your PATH!
#XLINK=tlink32
#XLINK=tlink /m/c/s/l
#XLINK=$(TC_BASE)/tlink /m/c
#XLINK=$(BC_BASE)/bin/tlink /m/c

# --- Microsoft Link ---
#XLINK=d:\qb\link /ma
#XLINK=$(MSC_BASE)\bin\link /ONERROR:NOEXE /ma /nologo

# --- WATCOM Link (wlinker is a batch file calling ms2wlink and wlink)
#XLINK=..\utils\wlinker /ma /nologo

# --- Optlink from Digital Mars ---
#XLINK=link


# =====================================================================
# Extra flags (advanced)
# =====================================================================

# Extra defines added to every C compilation, e.g.

## dis/enable WIN extras (1=enabled, 0=disable)
#WIN=0

## enable LFN API support
#LFNAPI=1

## enable printf function always available (for debugging)
#PRINTF=1

## extra DEBUG output (or add DEBUG=1 on command line)
#DEBUG=1

## DEBUG output on serial port (COM=# e.g. COM=2 for COM2)
#COM=2

## printf out goes to dosemu log
#DOSEMU=1
