#
# Linux cross compilation only!
# config file that is included in the main makefile for configuration
#

#****************************************************************
#  NOTICE!  If you edit you must rename this file to config.mak!*
#****************************************************************

#**********************************************************************
#- define NASM executable
#**********************************************************************
XNASM=nasm

#**********************************************************************
#- where is the BASE dir of your compiler(s) ??
#**********************************************************************
						
# if WATCOM maybe you need to set your WATCOM environment variables 
# and path
ifndef WATCOM
  WATCOM=$(HOME)/watcom
  PATH:=$(WATCOM)/binl:$(PATH)
endif

#**********************************************************************
# where is UPX and which options to use?
#**********************************************************************
XUPX=upx --8086 --best

# or use 
#unexport XUPX
# without the # if you don't want to use it

#**********************************************************************
# (optionally) which linker to use:
# (otherwise will be determined automatically)

# WATCOM Link
#XLINK=wlink

#*********************************************************************
# optionally define your MAKE type here, if not then
# it will be automatically determined, pick one of them
# use MS nmake if you want to compile with MSCL
#*********************************************************************

# Watcom MAKE in MS mode
#MAKE=wmake -ms -h

#*********************************************************************
# select your default target: required CPU and what FAT system to support
#*********************************************************************

XCPU=86
# XCPU=186
# XCPU=386

XFAT=16
# XFAT=32

# Give extra compiler DEFINE flags here
# such as -DDEBUG : extra DEBUG output
#         -DDOSEMU : printf output goes to dosemu log
# set ALLCFLAGS=-DDEBUG
