#
# FreeDOS Kernel - Make build
# Requires: GNU Make 4.0 or newer
#
# Host OS auto-detected: Linux/Unix, Windows (CMD/COMMAND) and DOS.
# Target compilers: Open Watcom (default), GCC-ia16, Borland/Turbo C, MSC.
#
# QUICK START
#   make                     build everything with the defaults
#   make COMPILER=gcc        build with gcc-ia16 instead of Open Watcom
#   make XCPU=386 XFAT=16    build a 386 / FAT16 kernel
#   make debug=1 com=1       enable -DDEBUG and COM-port debug output
#   make help                show configuration + target list
#   make clean               remove intermediates, keep binaries + *.map/*.lst
#   make clobber             remove EVERYTHING generated
#
# EVERYTHING is configured in config.mak (which must exist, but may be
# blank).  Every value can also be overridden on the command line.
#
# =====================================================================

# ---- require GNU Make 4.0+ ------------------------------------------
# Note: currently only tested with 4.3 but should work with any 4.x
# Extract only the first word (e.g., "4.3") to remove port designations
DETECTED_VERSION := $(firstword $(MAKE_VERSION))
# Compare the sorted elements 4.0 and the cleaned version string
ifeq ($(firstword $(sort 4.0 $(DETECTED_VERSION))),4.0)
  #if == 4.0 then 4.0 or newer
else
  $(error Error: GNU Make 4.0 or newer required (found $(MAKE_VERSION)))
endif

# =====================================================================
# User configuration: config.mak is optional, may be blank or not exist
# =====================================================================

-include config.mak

# =====================================================================
# Defaults for anything left blank/unset in config.mak
# =====================================================================
ifeq ($(COMPILER),)
  COMPILER := watcom
endif
ifeq ($(XCPU),)
  XCPU := 86
endif
ifeq ($(XFAT),)
  XFAT := 32
endif
ifeq ($(XNASM),)
  XNASM := nasm
endif
ifeq ($(LOADSEG),)
  LOADSEG := 0x60
endif
ifeq ($(BUILDROOT),)
  BUILDROOT := build
endif

# =====================================================================
# Host platform + shell command set detection
# =====================================================================
# PLATFORM is one of: windows | dos | linux  (override with PLATFORM=...)
ifeq ($(origin PLATFORM),undefined)
  ifeq ($(OS),Windows_NT)
    PLATFORM := windows
  else ifneq ($(or $(DJGPP),$(DJDIR)),)
    PLATFORM := dos
  else ifeq ($(OS),MSDOS)
    PLATFORM := dos
  else ifdef COMSPEC
    PLATFORM := dos
  else
    PLATFORM := linux
  endif
endif

# force 8.3 convention for DOS (don't assume LFN api available
ifeq ($(PLATFORM),dos)
  USE_8DOT3:=1
endif

ifeq ($(filter $(PLATFORM),windows dos),$(PLATFORM))
  # ---- DOS / Windows command.com / cmd.exe family ----
  SEP     := $(strip \)
  RM      := del /q
  RMDIR   := rmdir /s /q
  CP      := copy /y
  MKDIR   := mkdir
  TOUCH   := type nul >
  NULL    := nul
  fixpath  = $(subst /,$(SEP),$1)
else
  # ---- Unix-like family (Linux, *BSD, macOS) ----
  SEP     := /
  RM      := rm -f
  RMDIR   := rm -rf
  CP      := cp -f
  MKDIR   := mkdir -p
  TOUCH   := touch
  NULL    := /dev/null
  fixpath  = $1
endif

# =====================================================================
# Normalise + validate the compiler selection
# =====================================================================
ifeq ($(COMPILER),gcc-ia16)
  COMPILER := gcc
endif
ifeq ($(COMPILER),msvc)
  COMPILER := msc
endif
ifeq ($(COMPILER),turbo)
  COMPILER := borland
  USE_TURBO := 1
endif

ifeq ($(filter $(COMPILER),watcom gcc borland msc),)
  $(error Unknown COMPILER "$(COMPILER)" - must be watcom, gcc, borland or msc)
endif
ifeq ($(filter $(XCPU),86 186 286 386),)
  $(error XCPU must be one of 86 186 286 386 (got "$(XCPU)"))
endif
ifeq ($(filter $(XFAT),16 32),)
  $(error XFAT must be 16 or 32 (got "$(XFAT)"))
endif

# =====================================================================
# Text helpers (needed to build linker response files)
# =====================================================================
EMPTY :=
SP    := $(EMPTY) $(EMPTY)
COMMA := ,

# =====================================================================
# Common option toggles
# Enable from config.mak or the command line, e.g.  make debug=1 com=1
# =====================================================================
# accept either upper- or lower-case spellings for convenience
DEBUG   ?= $(debug)
PRINTF  ?= $(printf)
LFNAPI  ?= $(lfnapi)
WIN     ?= $(win)
COM     ?= $(com)
COMPORT ?= $(comport)

# deprecated as always done now
#LIST    ?= $(list)

ifeq ($(DEBUG),1)
  ALLCFLAGS += -DDEBUG
endif
ifeq ($(PRINTF),1)
  ALLCFLAGS += -DDEBUG_NEED_PRINTF
endif
ifeq ($(LFNAPI),1)
  ALLCFLAGS += -DWITHLFNAPI
endif
ifeq ($(WIN),1)
  ALLCFLAGS += -DWIN31SUPPORT
  NASMFLAGS += -DWIN31SUPPORT
else ifeq ($(WIN),0)
  ALLCFLAGS += -UWIN31SUPPORT
  NASMFLAGS += -UWIN31SUPPORT
endif
# COM-port debug output.  "com=N"
# (0=COM1 1=COM2 2=COM3) selects a specific port.
ifneq ($(strip $(COM)),)
  ALLCFLAGS += -DDEBUG_PRINT_COMPORT=$(COMPORT)
  NASMFLAGS += -DDEBUG_PRINT_COMPORT=$(COMPORT)
endif
# Free-form extra defines, e.g.  make DEFINES="-DDEBUG_FINDFIRST"
ifneq ($(strip $(DEFINES)),)
  ALLCFLAGS += $(DEFINES)
endif

# =====================================================================
# CPU / FAT feature macros
# =====================================================================
ifeq ($(XCPU),186)
  ALLCFLAGS += -DI186
endif
ifeq ($(XCPU),286)
  ALLCFLAGS += -DI186
endif
ifeq ($(XCPU),386)
  ALLCFLAGS += -DI386
  NASMFLAGS += -DI386
endif
ifeq ($(XFAT),32)
  ALLCFLAGS += -DWITHFAT32
  NASMFLAGS += -DWITHFAT32
endif

# =====================================================================
# Directory layout
# =====================================================================
BIN   := bin
# Note add "| $(BIN)" to dependency list to ensure dir exists
#      but not forcing rebuild if it or files within change timestamps
HDR   := hdr
ifeq ($(USE_8DOT3),1)
  # Define lookup tables (note our values are lowercase!)
  map_compiler_watcom    := w
  ifeq ($(USE_TURBO),1)
    map_compiler_borland := t
  else
    map_compiler_borland := b
  endif
  map_compiler_gcc       := g
  map_compiler_msc       := m
  map_platform_dos       := d
  map_platform_windows   := w
  map_platform_linux     := l
  # convert to single char version for 8.3 mappings
  COMPILER_83 := $(map_compiler_$(COMPILER))
  PLATFORM_83 := $(map_platform_$(PLATFORM))

  BUILD := $(BUILDROOT)/$(COMPILER_83)c$(XCPU)$(XFAT)$(PLATFORM_83)
else
  BUILD := $(BUILDROOT)/$(COMPILER)-x$(XCPU)-f$(XFAT)-$(PLATFORM)
endif

OBJ_UTILS   := $(BUILD)/utils
OBJ_LIB     := $(BUILD)/lib
OBJ_DRIVERS := $(BUILD)/drivers
OBJ_BOOT    := $(BUILD)/boot
OBJ_KERNEL  := $(BUILD)/kernel

# =====================================================================
# Per-compiler configuration blocks
# --------------------------------------------------------------------
# Each unique tool chain defines:
#   KPREFIX            kernel filename prefix (kwc/kgc/kbc/ktc/kms)
#   TARGETOPT          CPU code-generation switch
#   OBJFMT/NASM_DEFINE NASM object format + compiler symbol
#   KERNEL_CFLAGS      C flags for normal (HMA_TEXT) kernel objects
#   KERNEL_INITCFLAGS  C flags for INIT_TEXT kernel objects
#   compile_c          $(call compile_c,SRC,OBJ,EXTRA)
#   compile_c_tiny     $(call compile_c_tiny,SRC,OBJ,EXTRA)   (sys.com)
#   compile_host       $(call compile_host,SRC,EXE)  build-host helper exe
#   make_lib           $(call make_lib,LIB,OBJS)
#   init_patch         $(call init_patch,OBJ)   (INIT-segment fixup)
#   link_com           $(call link_com,OBJS,OUT) tiny-model .COM link
#   LINK_KERNEL_RECIPE canned recipe that links kernel.exe
# =====================================================================

# --------------------------- Open Watcom (default, verified) ---------
ifeq ($(COMPILER),watcom)
  KPREFIX := kwc

  ifeq ($(origin WATCOM),undefined)
    ifeq ($(PLATFORM),linux)
      WATCOM := $(HOME)/watcom
    else
      WATCOM := C:/WATCOM
    endif
  endif
  export WATCOM
  # Help Watcom find its binaries + headers without extra shell setup.
  ifeq ($(PLATFORM),linux)
    export PATH := $(WATCOM)/binl64:$(WATCOM)/binl:$(PATH)
    export INCLUDE := $(WATCOM)/h:$(INCLUDE)
  else ifeq ($(PLATFORM),windows)
    export PATH := $(WATCOM)/binnt;$(PATH)
    export INCLUDE := $(WATCOM)/h;$(WATCOM)/h/nt;$(INCLUDE)
  else
    export PATH := $(WATCOM)/binw;$(PATH)
    export INCLUDE := $(WATCOM)/h;$(INCLUDE)
  endif

  TARGETOPT := -0
  ifeq ($(XCPU),186)
    TARGETOPT := -1
  endif
  ifeq ($(XCPU),286)
    TARGETOPT := -2
  endif
  ifeq ($(XCPU),386)
    TARGETOPT := -3
  endif

  OBJFMT      := obj
  NASM_DEFINE := WATCOM

  # -os favour size  -s no stack checks  -e1 stop after 1 errors  -j signed
  # char  -zl no default-lib refs  -zp1 pack 1  -wx max warn  -we warn=err
  # -zgp -zfp far no gs/fs register usage  -r save/restore seg regs.
  WC_KERNEL_BASE    := -zq $(TARGETOPT) -I$(HDR) $(ALLCFLAGS) \
	               -os -s -e1 -j -zl -zp1 -wx -we -zgp -zfp -r
  KERNEL_CFLAGS     := $(WC_KERNEL_BASE) -ntHMA_TEXT
  KERNEL_INITCFLAGS := $(WC_KERNEL_BASE) -ntINIT_TEXT -gTGROUP -ndI

  # Tiny-model flags for the small DOS helper programs (sys.com).
  GEN_CFLAGS := -zq -zp1 -os -s -we -e3 -wx -bt=DOS
  TINY       := -mt

# -d1 add line# debug info, followed by wdis command to generate lst file
  compile_c      = wcc $3 -fo=$(call fixpath,$2) $(call fixpath,$1) -d1 && wdis -l -s $(call fixpath,$(dir $2))$(notdir $(basename $1)).obj
  # -mt (tiny) is only honoured by wcl, not wcc, so compile via wcl -c.
  compile_c_tiny = wcl -zq -c $(GEN_CFLAGS) $(TINY) $3 -fo=$(call fixpath,$2) $(call fixpath,$1)
  link_com       = wcl $(GEN_CFLAGS) $(TINY) -fe=$(call fixpath,$2) $(call fixpath,$1)
  init_patch     = @echo Watcom: no init patch needed for $1 >$(NULL)
  make_lib       = wlib -q -b -n $(call fixpath,$1) $(addprefix +,$(call fixpath,$2))

  # Host-native helper programs (they run during the build).
  # Note: weird $(patsubst %/,%,$(dir $2)) is to strip / from end of path which get 
  # switched to \ and then in usage becomes "blah\ -blah" so escapes the space between args breaking both
  ifeq ($(PLATFORM),linux)
    compile_host = wcl386 -zq -bcl=linux -I$(HDR) -fo=$(call fixpath,$(dir $2)) -fe=$(call fixpath,$2) $(call fixpath,$1)
  else ifeq ($(PLATFORM),windows)
    compile_host = wcl386 -zq -bcl=nt -I$(HDR) -fo=$(call fixpath,$(dir $2)) -fe=$(call fixpath,$2) $(call fixpath,$1)
  else
    compile_host = wcl $(GEN_CFLAGS) $(TINY) -I$(HDR) -fo=$(call fixpath,$(patsubst %/,%,$(dir $2))) -fe=$(call fixpath,$2) $(call fixpath,$1)
  endif

  # wlink reads a response file (the object list is far too long for a
  # command line).  Regenerated every link with $(file ...).
  LNK_FILE  := $(OBJ_KERNEL)/$(KPREFIX)$(XCPU)$(XFAT).lnk
  WC_KLIBS   = $(OBJ_DRIVERS)/device.lib $(LIB_LIBM) $(KERNEL_SUBLIBS)
  wc_objlist = $(subst $(SP),$(COMMA),$(strip $(KERNEL_OBJS)))
  define LINK_KERNEL_RECIPE
	$(file >$(LNK_FILE),debug all)
	$(file >>$(LNK_FILE),format dos)
	$(file >>$(LNK_FILE),option quiet,map=$(OBJ_KERNEL)/kernel.map,symfile,statics,verbose)
	$(file >>$(LNK_FILE),file $(wc_objlist))
	$(file >>$(LNK_FILE),$(addprefix library ,$(WC_KLIBS)))
	$(file >>$(LNK_FILE),name $(OBJ_KERNEL)/kernel.exe)
	wlink @$(LNK_FILE)
  endef
endif

# --------------------------- GCC ia16 (best-effort, config-only) -----
ifeq ($(COMPILER),gcc)
  KPREFIX := kgc

  ifneq ($(strip $(GCC_BASE)),)
    export PATH := $(GCC_BASE)/bin:$(PATH)
  endif

  TARGETOPT := -march=i8086
  ifeq ($(XCPU),186)
    TARGETOPT := -march=i80186
  endif
  ifeq ($(XCPU),286)
    TARGETOPT := -march=i80286
  endif
  ifeq ($(XCPU),386)
    TARGETOPT := -march=i80286
  endif

  OBJFMT      := elf
  NASM_DEFINE := gcc

  GCC_BASEFLAGS := -I./hdr $(TARGETOPT) -mcmodel=small -fleading-underscore \
	           -fno-common -ffreestanding -fcall-used-es \
	           -mrtd -Wno-pointer-to-int-cast -Wno-attributes -Werror -Os \
	           -fno-strict-aliasing -mfar-function-if-far-return-type \
	           $(ALLCFLAGS)
  KERNEL_CFLAGS     := $(GCC_BASEFLAGS)
  KERNEL_INITCFLAGS := $(GCC_BASEFLAGS)
  GCC_TINY := -march=i8086 -mcmodel=tiny -Os -fno-strict-aliasing \
	      -fcall-used-es -Wno-pointer-to-int-cast
#-fpack-struct (we use #pragma pack() where needed)

  compile_c      = ia16-elf-gcc $3 -c -o $2 $1
  compile_c_tiny = ia16-elf-gcc $(GCC_TINY) $3 -c -o $2 $1
  link_com       = ia16-elf-gcc $(GCC_TINY) -o $2 $1
  make_lib       = ia16-elf-ar crs $1 $2
  compile_host   = gcc -O2 -Wall -I$(HDR) -o $2 $1

  # INIT_TEXT objects share names with runtime helpers; rename them so the
  # two text segments do not collide at link time.
  init_patch = ia16-elf-objcopy \
    --redefine-sym ___umodsi3=_init_umodsi3 \
    --redefine-sym ___udivsi3=_init_udivsi3 \
    --redefine-sym ___ashlsi3=_init_ashlsi3 \
    --redefine-sym ___lshrsi3=_init_lshrsi3 \
    --redefine-sym _printf=_init_printf \
    --redefine-sym _sprintf=_init_sprintf \
    --redefine-sym _execrh=_init_execrh \
    --redefine-sym _memcpy=_init_memcpy \
    --redefine-sym _fmemcpy=_init_fmemcpy \
    --redefine-sym _fmemset=_init_fmemset \
    --redefine-sym _fmemcmp=_init_fmemcmp \
    --redefine-sym _memcmp=_init_memcmp \
    --redefine-sym _memset=_init_memset \
    --redefine-sym _strchr=_init_strchr \
    --redefine-sym _strcpy=_init_strcpy \
    --redefine-sym _fstrcpy=_init_fstrcpy \
    --redefine-sym _strlen=_init_strlen \
    --redefine-sym _fstrlen=_init_fstrlen \
    --redefine-sym _open=_init_DosOpen $1

  WC_KLIBS = $(OBJ_DRIVERS)/device.lib $(LIB_LIBM) $(KERNEL_SUBLIBS)
  define LINK_KERNEL_RECIPE
	ia16-elf-gcc -mcmodel=small -T kernel/kernel.ld -Wl,-Map,$(OBJ_KERNEL)/kernel.map \
	  -nostdlib -o $(OBJ_KERNEL)/kernel.exe $(KERNEL_OBJS) $(WC_KLIBS)
  endef
endif

# --------------------------- Borland / Turbo C (best-effort) ---------
ifeq ($(COMPILER),borland)
  ifeq ($(USE_TURBO),1)
    KPREFIX := ktc
    BCC := tcc
    BORLAND_BASE := $(TC_BASE)
  else
    KPREFIX := kbc
    BCC := bcc
    BORLAND_BASE := $(BC_BASE)
  endif
  ifneq ($(strip $(BORLAND_BASE)),)
    export PATH := $(BORLAND_BASE)/bin;$(PATH)
  endif

  TARGETOPT := -1
  ifeq ($(XCPU),386)
    TARGETOPT := -3
  endif

  OBJFMT      := obj
  NASM_DEFINE := borland

  BCC_BASEFLAGS     := -I./hdr $(TARGETOPT) -c -O -Z -d -w-pia $(ALLCFLAGS)
  KERNEL_CFLAGS     := $(BCC_BASEFLAGS) -zCHMA_TEXT
  KERNEL_INITCFLAGS := $(BCC_BASEFLAGS) -zCINIT_TEXT
  BCC_TINY := -mt -c -O -Z -d $(TARGETOPT)

  compile_c      = $(BCC) $3 -o$(call fixpath,$2) $(call fixpath,$1)
  compile_c_tiny = $(BCC) $(BCC_TINY) $3 -o$(call fixpath,$2) $(call fixpath,$1)
  link_com       = $(BCC) $(BCC_TINY) -e$(call fixpath,$2) $(call fixpath,$1)
  make_lib       = tlib $1 $(addprefix +,$(call fixpath,$2))
  compile_host   = $(BCC) -I$(HDR) -e$(call fixpath,$2) $(call fixpath,$1)
  init_patch     = @echo Borland: no init patch needed for $1 >$(NULL)

  WC_KLIBS = $(OBJ_DRIVERS)/device.lib $(LIB_LIBM) $(KERNEL_SUBLIBS)
  define LINK_KERNEL_RECIPE
	tlink /m/c/s/l @$(OBJ_KERNEL)/kernel.rsp
  endef
endif

# --------------------------- Microsoft C -----------------------------
ifeq ($(COMPILER),msc)
  KPREFIX := kms
  ifneq ($(strip $(MSC_BASE)),)
    export PATH := $(MSC_BASE)/bin;$(PATH)
  endif

  TARGETOPT := /G0
  ifeq ($(XCPU),186)
    TARGETOPT := /G1
  endif
  ifeq ($(XCPU),386)
    TARGETOPT := /G3
  endif

  OBJFMT      := obj
  NASM_DEFINE := msc

  MSC_BASEFLAGS     := /I./hdr $(TARGETOPT) /c /Ox /Zp1 /W3 /WX $(ALLCFLAGS)
  KERNEL_CFLAGS     := $(MSC_BASEFLAGS) /NTHMA_TEXT
  KERNEL_INITCFLAGS := $(MSC_BASEFLAGS) /NTINIT_TEXT
  MSC_TINY := /AT /c /Ox /Zp1 /W3 $(TARGETOPT)

  compile_c      = cl $3 /Fo$(call fixpath,$2) $(call fixpath,$1)
  compile_c_tiny = cl $(MSC_TINY) $3 /Fo$(call fixpath,$2) $(call fixpath,$1)
  link_com       = cl $(MSC_TINY) /Fe$(call fixpath,$2) $(call fixpath,$1)
  make_lib       = lib /nologo /out:$(call fixpath,$1) $(call fixpath,$2)
  compile_host   = cl /I$(HDR) /Fe$(call fixpath,$2) $(call fixpath,$1)
  init_patch     = @echo MSC: no init patch needed for $1 >$(NULL)

  WC_KLIBS = $(OBJ_DRIVERS)/device.lib $(LIB_LIBM) $(KERNEL_SUBLIBS)
  define LINK_KERNEL_RECIPE
	link /MAP:$(OBJ_KERNEL)/kernel.map @$(OBJ_KERNEL)/kernel.rsp
  endef
endif

# =====================================================================
# NASM + UPX
# =====================================================================
NASM := $(XNASM)
NASM_BASEFLAGS := -i $(HDR)/ -DXCPU=$(XCPU) $(NASMFLAGS)

# UPX kernel compression (enable by setting XUPX in config.mak, e.g.
# XUPX=upx --8086 --best).  Leave XUPX blank to disable.
ifneq ($(strip $(XUPX)),)
  UPXOPT := -U
else
  UPXOPT :=
endif

# Final kernel filename, e.g. kwc8632  (watcom / 8086 / FAT32).
KTARGET := $(KPREFIX)$(XCPU)$(XFAT)

# =====================================================================
# Generic pattern-rule machinery
# =====================================================================
# Order-only prerequisite directories (created on demand).
ALL_DIRS := $(OBJ_UTILS) $(OBJ_LIB) $(OBJ_DRIVERS) $(OBJ_BOOT) $(OBJ_KERNEL) $(BIN)

$(sort $(ALL_DIRS)):
	$(MKDIR) $(call fixpath,$@)

# Windows and Linux automatically mkdir directories, but DJGPP needs explict rule
$(BUILDROOT):
	$(MKDIR) $(call fixpath,$(BUILDROOT))
	
$(BUILD): | $(BUILDROOT)
	$(MKDIR) $(call fixpath,$(BUILD))
	
#$(OBJ_UTILS): | $(BUILDROOT) $(BUILD)
#	$(MKDIR) $(call fixpath,$(OBJ_UTILS))


# $(call DEFINE_COMPILE_RULES,OBJDIR,SRCDIR,CFLAGS) -> C + ASM patterns.
# NASM is told to search SRCDIR so %include "segs.inc" resolves.
# Always generate a per-object .lst listing (formally optional via LIST option)
define DEFINE_COMPILE_RULES
$(1)/%.obj : $(2)/%.c | $(1)
	$$(call compile_c,$$<,$$@,$(3))

$(1)/%.obj : $(2)/%.asm | $(1)
	$$(NASM) -f $$(OBJFMT) -D$$(NASM_DEFINE) -i $(2)/ $$(NASM_BASEFLAGS) \
	  -l$$(@:.obj=.lst) -o $$@ $$<
endef

# =====================================================================
# utils/  -  host-native build helper programs
# =====================================================================
UTILS_EXEFLAT  := $(OBJ_UTILS)/exeflat.exe
UTILS_PATCHOBJ := $(OBJ_UTILS)/patchobj.exe
UTILS_UPXENTRY := $(OBJ_UTILS)/upxentry.bin
UTILS_UPXDEVIC := $(OBJ_UTILS)/upxdevic.bin

UTILS_TARGETS := $(UTILS_EXEFLAT) $(UTILS_PATCHOBJ) \
	         $(UTILS_UPXENTRY) $(UTILS_UPXDEVIC)

.PHONY: utils
utils: $(UTILS_TARGETS)

$(UTILS_EXEFLAT): utils/exeflat.c hdr/exe.h | $(BUILDROOT) $(BUILD) $(OBJ_UTILS)
	$(call compile_host,utils/exeflat.c,$@)

$(UTILS_PATCHOBJ): utils/patchobj.c | $(BUILDROOT) $(BUILD) $(OBJ_UTILS)
	$(call compile_host,utils/patchobj.c,$@)

$(UTILS_UPXENTRY): utils/upxentry.asm | $(BUILDROOT) $(BUILD) $(OBJ_UTILS)
	$(NASM) -f bin -i utils/ $(NASM_BASEFLAGS) -o $@ $<

$(UTILS_UPXDEVIC): utils/upxdevic.asm | $(BUILDROOT) $(BUILD) $(OBJ_UTILS)
	$(NASM) -f bin -i utils/ $(NASM_BASEFLAGS) -o $@ $<

# =====================================================================
# lib/  -  libm.lib placeholder
# =====================================================================
# Historically carved out of the C runtime on a DOS host; the supported
# cross builds do not need it, so a zero-length placeholder keeps it as an
# optional, always-present kernel-link input.
LIB_LIBM := $(OBJ_LIB)/libm.lib

.PHONY: libm
libm: $(LIB_LIBM) |  $(BUILDROOT) $(BUILD)

ifeq ($(COMPILER),borland)
  # Math library extraction (for Borland/Turbo C)
  CLIB := $(LIBPATH)/cs.lib
  MATH_OBJS := H_LDIV.OBJ H_LLSH.OBJ H_LURSH.OBJ F_LXMUL.OBJ
  
$(LIBM_LIB): $(CLIB) | $(OBJ_LIB)
	@echo Extracting math library from $(CLIB)
	-$(RM) libm.lib
	-$(RM) $(OBJ_LIB)/H_LDIV.OBJ $(OBJ_LIB)/H_LLSH.OBJ $(OBJ_LIB)/H_LURSH.OBJ $(OBJ_LIB)/F_LXMUL.OBJ
	#$(LIBUTIL) $(CLIB) $(MATH_EXTRACT) $(LIBTERM)
	$(CD) $(OBJ_LIB) && $(TARGET_AR) $(call PATHSEP,$(CLIB)) *H_LDIV *H_LLSH *H_LURSH *F_LXMUL
	#$(COMSPEC) /c for %i in (*.obj) do ..\utils\patchobj CODE=LCODE %i
	$(CD) $(OBJ_LIB) && $(PATCHOBJ) CODE=LCODE H_LDIV.OBJ
	$(CD) $(OBJ_LIB) && $(PATCHOBJ) CODE=LCODE H_LLSH.OBJ
	$(CD) $(OBJ_LIB) && $(PATCHOBJ) CODE=LCODE H_LURSH.OBJ
	$(CD) $(OBJ_LIB) && $(PATCHOBJ) CODE=LCODE F_LXMUL.OBJ
	#$(LIBUTIL) libm $(MATH_INSERT)     $(LIBTERM)
	$(CD) $(OBJ_LIB) && $(TARGET_AR) libm $(LIBPLUS)H_LDIV.OBJ $(LIBPLUS)H_LLSH.OBJ $(LIBPLUS)H_LURSH.OBJ $(LIBPLUS)F_LXMUL.OBJ $(LIBTERM)
	-$(RM) $(OBJ_LIB)/H_LDIV.OBJ $(OBJ_LIB)/H_LLSH.OBJ $(OBJ_LIB)/H_LURSH.OBJ $(OBJ_LIB)/F_LXMUL.OBJ
else
  # Other compilers don't need libm extraction
$(LIB_LIBM): | $(OBJ_LIB)
	@echo Math library not needed for $(COMPILER)
	$(TOUCH) $(call fixpath,$@)
endif


# =====================================================================
# drivers/  -  device.lib
# =====================================================================
DRIVER_NAMES := floppy rdpcclk wrpcclk wratclk
DRIVER_OBJS  := $(addprefix $(OBJ_DRIVERS)/,$(addsuffix .obj,$(DRIVER_NAMES)))
DRIVERS_LIB  := $(OBJ_DRIVERS)/device.lib

$(eval $(call DEFINE_COMPILE_RULES,$(OBJ_DRIVERS),drivers,))

.PHONY: drivers
drivers: $(DRIVERS_LIB) |  $(BUILDROOT) $(BUILD)

$(DRIVERS_LIB): $(DRIVER_OBJS) | $(OBJ_DRIVERS)
	$(RM) $(call fixpath,$@)
	$(call make_lib,$@,$(DRIVER_OBJS))

# =====================================================================
# boot/  -  FAT boot sectors (flat .bin images)
# =====================================================================
BOOT_BINS := $(addprefix $(OBJ_BOOT)/,\
  fat12com.bin fat16com.bin fat32chs.bin fat32lba.bin oemfat12.bin oemfat16.bin)

nasmboot = $(NASM) -f bin -i boot/ $(NASMBOOTFLAGS) $(NASM_BASEFLAGS)

.PHONY: boot
boot: $(BOOT_BINS)

$(OBJ_BOOT)/fat12com.bin: boot/boot.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -dISFAT12 -o $@ -l$(OBJ_BOOT)/fat12com.lst boot/boot.asm
$(OBJ_BOOT)/fat16com.bin: boot/boot.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -dISFAT16 -o $@ -l$(OBJ_BOOT)/fat16com.lst boot/boot.asm
$(OBJ_BOOT)/fat32chs.bin: boot/boot32.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -o $@ -l$(OBJ_BOOT)/fat32chs.lst boot/boot32.asm
$(OBJ_BOOT)/fat32lba.bin: boot/boot32lb.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -o $@ -l$(OBJ_BOOT)/fat32lba.lst boot/boot32lb.asm
$(OBJ_BOOT)/oemfat12.bin: boot/oemboot.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -dISFAT12 -o $@ -l$(OBJ_BOOT)/oemfat12.lst boot/oemboot.asm
$(OBJ_BOOT)/oemfat16.bin: boot/oemboot.asm boot/magic.mac | $(OBJ_BOOT)
	$(nasmboot) -dISFAT16 -o $@ -l$(OBJ_BOOT)/oemfat16.lst boot/oemboot.asm

# =====================================================================
# kernel/  -  the FreeDOS kernel itself
# =====================================================================
# Object list (defines link order).
KERNEL_MODS := \
  kernel entry io console serial printer dsk sysclk \
  asmsupt execrh nlssupt procsupt dosidle int2f nls_hc \
  apisupt intr irqstack blockio chario systime error \
  break dosfns fatdir fatfs fattab fcbfns inthndlr \
  ioctl memmgr task newstuff nls network \
  prf misc strings syspack lfnapi iasmsupt memdisk \
  main config initoem inithma dyninit iprf \
  initdisk initclk cpu

KERNEL_OBJS := $(addprefix $(OBJ_KERNEL)/,$(addsuffix .obj,$(KERNEL_MODS)))

# ---- header dependency groups
HDR_FILES := $(addprefix hdr/,\
  portab.h device.h mcb.h pcb.h fat.h fcb.h tail.h dtime.h process.h \
  dcb.h sft.h cds.h exe.h fnode.h dirmatch.h file.h clock.h kbd.h \
  error.h version.h) kernel/dyndata.h
KHEADERS     := $(HDR_FILES) kernel/globals.h kernel/proto.h
KINITHEADERS := $(HDR_FILES) kernel/init-mod.h kernel/init-dat.h

# Normal (HMA_TEXT) compile pattern rules for kernel/*.c and kernel/*.asm.
$(eval $(call DEFINE_COMPILE_RULES,$(OBJ_KERNEL),kernel,$(KERNEL_CFLAGS)))

# C header dependencies (append prerequisites).
KERNEL_C_HMA := blockio break chario dosfns dsk error fatdir fatfs \
	        fattab fcbfns inthndlr ioctl memmgr misc lfnapi \
	        newstuff network nls sysclk syspack systime task
$(foreach m,$(KERNEL_C_HMA),$(eval $(OBJ_KERNEL)/$(m).obj: $(KHEADERS)))
$(OBJ_KERNEL)/prf.obj: hdr/portab.h

# ASM include dependencies.
KSEGS := kernel/segs.inc
KSTK  := hdr/stacks.inc
$(OBJ_KERNEL)/apisupt.obj:  $(KSEGS)
$(OBJ_KERNEL)/asmsupt.obj:  $(KSEGS)
$(OBJ_KERNEL)/console.obj:  $(KSEGS) kernel/io.inc
$(OBJ_KERNEL)/cpu.obj:      $(KSEGS)
$(OBJ_KERNEL)/dosidle.obj:  $(KSEGS)
$(OBJ_KERNEL)/entry.obj:    $(KSEGS) $(KSTK)
$(OBJ_KERNEL)/execrh.obj:   $(KSEGS)
$(OBJ_KERNEL)/int2f.obj:    $(KSEGS) $(KSTK)
$(OBJ_KERNEL)/intr.obj:     $(KSEGS)
$(OBJ_KERNEL)/io.obj:       $(KSEGS) $(KSTK)
$(OBJ_KERNEL)/irqstack.obj: $(KSEGS)
$(OBJ_KERNEL)/kernel.obj:   $(KSEGS) kernel/ludivmul.inc
$(OBJ_KERNEL)/memdisk.obj:  $(KSEGS)
$(OBJ_KERNEL)/nls_hc.obj:   $(KSEGS)
$(OBJ_KERNEL)/nlssupt.obj:  $(KSEGS) $(KSTK)
$(OBJ_KERNEL)/printer.obj:  $(KSEGS) kernel/io.inc
$(OBJ_KERNEL)/procsupt.obj: $(KSEGS) $(KSTK)
$(OBJ_KERNEL)/serial.obj:   $(KSEGS) kernel/io.inc

# ---- INIT_TEXT-segment objects: different C flags + an object patch ---
define DEFINE_INIT_OBJ
$(OBJ_KERNEL)/$(1).obj: kernel/$(1).c $$(KINITHEADERS) | $$(OBJ_KERNEL)
	$$(call compile_c,$$<,$$@,$$(KERNEL_INITCFLAGS))
	$$(call init_patch,$$@)
endef
$(foreach m,config initoem main inithma dyninit initdisk initclk,\
  $(eval $(call DEFINE_INIT_OBJ,$(m))))

# iprf.obj: INIT-segment printf; its source #includes prf.c.
$(OBJ_KERNEL)/iprf.obj: kernel/iprf.c kernel/prf.c hdr/portab.h | $(OBJ_KERNEL)
	$(call compile_c,$<,$@,$(KERNEL_INITCFLAGS))
	$(call init_patch,$@)

# iasmsupt.obj: the INIT-segment copy of asmsupt.asm (built with -D_INIT).
$(OBJ_KERNEL)/iasmsupt.obj: kernel/asmsupt.asm $(KSEGS) | $(OBJ_KERNEL)
	$(NASM) -f $(OBJFMT) -D$(NASM_DEFINE) -D_INIT -i kernel/ \
	  $(NASM_BASEFLAGS) -o $@ $<

# ---------------------------------------------------------------------
# REUSABLE TEMPLATE: build a static library from a kernel source subdir.
# ---------------------------------------------------------------------
# The kernel is being refactored into topical subdirectories (e.g.
# kernel/filesys/*.c for FAT logic, kernel/dosfns/*.c for the DOS API).
# For each such subdir this template:
#   * compiles every .c / .asm in it (HMA_TEXT flags, same header deps),
#   * archives the objects into $(OBJ_KERNEL)/<name>.lib,
#   * appends that lib to $(KERNEL_SUBLIBS) so it is linked into kernel.exe.
# It is driven by KERNEL_LIB_DIRS and is a harmless no-op until the
# directories actually exist (guarded by $(wildcard)), so you can add a
# new library simply by creating kernel/<name>/ and dropping sources in.
#
#   $(call KERNEL_SUBLIB,<subdir-name>)
# ---------------------------------------------------------------------
KERNEL_SUBLIBS :=
KERNEL_LIB_DIRS ?= filesys dosfns

define KERNEL_SUBLIB
$(1)_SRCDIR := kernel/$(1)
$(1)_OBJDIR := $$(OBJ_KERNEL)/$(1)
$(1)_CSRC   := $$(wildcard kernel/$(1)/*.c)
$(1)_ASRC   := $$(wildcard kernel/$(1)/*.asm)
$(1)_OBJS   := $$(patsubst kernel/$(1)/%.c,$$($(1)_OBJDIR)/%.obj,$$($(1)_CSRC)) \
	       $$(patsubst kernel/$(1)/%.asm,$$($(1)_OBJDIR)/%.obj,$$($(1)_ASRC))
$(1)_LIB    := $$(OBJ_KERNEL)/$(1).lib

$$($(1)_OBJDIR):
	$$(MKDIR) $$(call fixpath,$$@)

$$($(1)_OBJDIR)/%.obj : kernel/$(1)/%.c $$(KHEADERS) | $$($(1)_OBJDIR)
	$$(call compile_c,$$<,$$@,$$(KERNEL_CFLAGS))

$$($(1)_OBJDIR)/%.obj : kernel/$(1)/%.asm | $$($(1)_OBJDIR)
	$$(NASM) -f $$(OBJFMT) -D$$(NASM_DEFINE) -i kernel/$(1)/ -i kernel/ \
	  $$(NASM_BASEFLAGS) -o $$@ $$<

$$($(1)_LIB): $$($(1)_OBJS) | $$(OBJ_KERNEL)
	$$(RM) $$(call fixpath,$$@)
	$$(call make_lib,$$@,$$($(1)_OBJS))

KERNEL_SUBLIBS += $$($(1)_LIB)
endef

# Instantiate the template for every configured subdir that exists.
$(foreach d,$(KERNEL_LIB_DIRS),\
  $(if $(strip $(wildcard kernel/$(d)/*.c) $(wildcard kernel/$(d)/*.asm)),\
    $(eval $(call KERNEL_SUBLIB,$(d)))))

# ---- link kernel.exe, flatten to kernel.sys, publish into bin/ --------
KERNEL_LINK_DEPS := $(DRIVERS_LIB) $(LIB_LIBM) $(KERNEL_SUBLIBS)

$(OBJ_KERNEL)/kernel.exe: $(KERNEL_OBJS) $(KERNEL_LINK_DEPS) | $(OBJ_KERNEL)
	$(LINK_KERNEL_RECIPE)

# exeflat command file (contents vary with UPX config -> regenerated).
$(OBJ_KERNEL)/exeflat.rsp: | $(OBJ_KERNEL)
	$(file >$@,-S0x10)
	$(file >>$@,-S0x78)
	$(file >>$@,-S0x79)
	$(file >>$@,-E$(UTILS_UPXENTRY))
	$(file >>$@,-D$(UTILS_UPXDEVIC))
	$(file >>$@,$(UPXOPT))
	$(file >>$@,$(XUPX))

$(OBJ_KERNEL)/kernel.sys: $(OBJ_KERNEL)/kernel.exe $(UTILS_EXEFLAT) \
	$(UTILS_UPXENTRY) $(UTILS_UPXDEVIC) $(OBJ_KERNEL)/exeflat.rsp
	$(UTILS_EXEFLAT) $(OBJ_KERNEL)/kernel.exe $@ $(LOADSEG) @$(OBJ_KERNEL)/exeflat.rsp

KERNEL_OUT := $(BIN)/$(KTARGET).sys

$(KERNEL_OUT): $(OBJ_KERNEL)/kernel.sys | $(BIN)  $(BUILDROOT) $(BUILD)
	$(CP) $(call fixpath,$<) $(call fixpath,$@)
	$(CP) $(call fixpath,$<) $(call fixpath,$(BIN)/kernel.sys)
	$(CP) $(call fixpath,$(OBJ_KERNEL)/kernel.map) $(call fixpath,$(BIN)/$(KTARGET).map)

.PHONY: kernel
kernel: utils $(KERNEL_OUT)

# =====================================================================
# Submodules (sys, setver, country, share)
# =====================================================================
# Each is a git submodule with its own makefile/build script.  We invoke
# it and hand down EVERY relevant variable so a submodule builds the same
# way whether driven from here or standalone (cd <dir> && make ...).
SUBVARS := COMPILER="$(COMPILER)" XCPU="$(XCPU)" XFAT="$(XFAT)" \
	   XNASM="$(XNASM)" XUPX="$(XUPX)" LOADSEG="$(LOADSEG)" \
	   ALLCFLAGS="$(ALLCFLAGS)" NASMFLAGS="$(NASMFLAGS)" \
	   WATCOM="$(WATCOM)" GCC_BASE="$(GCC_BASE)" BC_BASE="$(BC_BASE)" \
	   TC_BASE="$(TC_BASE)" MSC_BASE="$(MSC_BASE)" USE_TURBO="$(USE_TURBO)" \
	   PLATFORM="$(PLATFORM)" SEP="$(SEP)" DIRSEP="$(SEP)" \
	   RM="$(RM)" RMDIR="$(RMDIR)" CP="$(CP)" MKDIR="$(MKDIR)" \
	   NASM="$(NASM)" OBJFMT="$(OBJFMT)" NASM_DEFINE="$(NASM_DEFINE)" \
	   TARGETOPT="$(TARGETOPT)" KPREFIX="$(KPREFIX)" \
	   KERNEL_CFLAGS="$(KERNEL_CFLAGS)" GEN_CFLAGS="$(GEN_CFLAGS)" \
	   TINY="$(TINY)" \
	   TOPDIR="$(CURDIR)" BOOTDIR="$(CURDIR)/$(OBJ_BOOT)" \
	   KERNELDIR="$(CURDIR)/kernel" BINDIR="$(CURDIR)/$(BIN)"

# sys.com needs the boot sectors + kernel/prf.c, so build boot first.
.PHONY: sys
sys: boot | $(BIN)
	@echo "==> building sys submodule"

#	$(MAKE) -C sys $(SUBVARS)

.PHONY: setver
setver: | $(BIN)
	@echo "==> building setver submodule"

#	$(MAKE) -C setver $(SUBVARS)

# country ships a GNU makefile with a "production" target.
.PHONY: country
country: | $(BIN)
	$(if $(wildcard country/makefile country/Makefile),\
	  @echo "==> building country submodule" && \
	  $(MAKE) -C country $(SUBVARS) production BINDIR="$(CURDIR)/$(BIN)",\
	  @echo "country submodule not initialised - run: git submodule update --init")
	  
# share ships build.sh which keys off $COMPILER.
.PHONY: share
share: | $(BIN)
	$(if $(wildcard share/build.sh),\
	  @echo "==> building share submodule" && \
	  ( cd share && env COMPILER=$(COMPILER) ./build.sh ),\
	  @echo "share submodule not initialised - run: git submodule update --init")

# =====================================================================
# main entry and cleanup targets
# =====================================================================
.DEFAULT_GOAL := all

.PHONY: all
all: kernel sys setver country
	@echo "--------------------------------------------------------------"
	@echo "Build complete: COMPILER=$(COMPILER) XCPU=$(XCPU) XFAT=$(XFAT)"
	@echo "  kernel -> $(BIN)/$(KTARGET).sys (+ $(BIN)/kernel.sys)"
	@echo "  sys    -> $(BIN)/sys.com"
	@echo "  setver -> $(BIN)/setver.sys"

# clean: remove intermediates but KEEP final binaries and every *.map/*.lst.
CLEAN_FILES := $(wildcard \
  $(BUILD)/*/*.obj $(BUILD)/*/*.exe $(BUILD)/*/*.com $(BUILD)/*/*.bin \
  $(BUILD)/*/*.lib $(BUILD)/*/*.h   $(BUILD)/*/*.rsp $(BUILD)/*/*.lnk \
  $(BUILD)/*/*.err $(BUILD)/*/*.bak $(BUILD)/*/kernel.sys \
  $(BUILD)/kernel/*/*.obj)

.PHONY: clean
clean:
	-$(MAKE) -C sys $(SUBVARS) clean 2>$(NULL) || true
	-$(MAKE) -C setver $(SUBVARS) clean 2>$(NULL) || true
	-[ -f country/makefile ] && $(MAKE) -C country $(SUBVARS) clean 2>$(NULL) || true
	$(if $(strip $(CLEAN_FILES)),$(RM) $(call fixpath,$(CLEAN_FILES)),@echo "clean: no intermediates")
	@echo "clean: kept *.map, *.lst and final binaries in $(BIN)/"

# clobber: remove ALL generated files - the whole build tree + bin/ binaries.
CLOBBER_FILES := $(sort $(wildcard \
  $(BIN)/kernel.sys $(BIN)/k*.sys $(BIN)/*.map \
  $(BIN)/sys.com $(BIN)/setver.sys $(BIN)/country.sys $(BIN)/share.com))

.PHONY: clobber
clobber:
	-$(MAKE) -C sys $(SUBVARS) clobber 2>$(NULL) || true
	-$(MAKE) -C setver $(SUBVARS) clobber 2>$(NULL) || true
	$(if $(wildcard country/Makefile),-$(MAKE) -C country $(SUBVARS) clobber 2>$(NULL) || true)
	$(if $(wildcard $(call fixpath,$(BUILDROOT))),-$(RMDIR) $(call fixpath,$(BUILDROOT)) || true)
	$(if $(strip $(CLOBBER_FILES)),-$(RM) $(call fixpath,$(CLOBBER_FILES)),@echo "clobber: no bin/ binaries")
	@echo "clobber: removed build tree and generated binaries"

# =====================================================================
# Help / usage
# =====================================================================
.PHONY: help
help:
	@echo "FreeDOS kernel - unified GNU Make build system (needs Make 4.0+)"
	@echo ""
	@echo "Targets:"
	@echo "  all        build kernel + sys + setver + country (default)"
	@echo "  kernel     build only $(BIN)/$(KTARGET).sys"
	@echo "  sys        build the sys submodule (sys.com)"
	@echo "  setver     build the setver submodule (setver.sys)"
	@echo "  country    build the country submodule"
	@echo "  share      build the share submodule"
	@echo "  utils drivers boot lib   build individual components"
	@echo "  clean      remove intermediates, keep binaries + *.map/*.lst"
	@echo "  clobber    remove ALL generated files"
	@echo ""
	@echo "Configuration (set in config.mak or override on the command line):"
	@echo "  COMPILER = $(COMPILER)   (watcom|gcc|borland|msc; turbo/msvc/gcc-ia16 aliases)"
	@echo "  XCPU     = $(XCPU)       (86|186|286|386)"
	@echo "  XFAT     = $(XFAT)       (16|32)"
	@echo "  PLATFORM = $(PLATFORM)   (auto-detected: windows|dos|linux)"
	@echo "  BUILD    = $(BUILD)"
	@echo ""
	@echo "Common options (e.g. make debug=1 com=1 DEFINES=-DDEBUG_FINDFIRST):"
	@echo "  debug=1     -DDEBUG            printf=1  -DDEBUG_NEED_PRINTF"
	@echo "  lfnapi=1    -DWITHLFNAPI       win=1     -DWIN31SUPPORT"
	@echo "  com=1       COM-port debug     comport=N specific port (0/1/2)"
	@echo "  list=1      per-object .lst    DEFINES=  extra -D flags"
	@echo "  XUPX=...    UPX-compress the kernel (blank = disabled)"
