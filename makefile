# IF NOTHING COMPILES, CHECK IF YOUR CVS CHECKOUT USES CORRECT DOS LINEBREAKS

# What you WANT is: EDIT CONFIG.B, COPY CONFIG.B to CONFIG.BAT, RUN BUILD.BAT
# THIS file is provided only for people who have a habit of typing MAKE ALL...

all:
	build

bin\kwc8616.sys:
	build -r wc 86 fat16

bin\kwc8632.sys:
	build -r wc 86 fat32

# use as follows: wmake -ms zip VERSION=2029
zip_src:
	cd ..\..
	zip -9 -r -k source/ke$(VERSION)s.zip source/ke$(VERSION) -i@source/ke$(VERSION)/filelist
	cd source\ke$(VERSION)

BINLIST1 = doc bin/kernel.sys bin/sys.com
# removed - as the 2nd zip -r line to add those to the zip:
# BINLIST2 = bin/config.sys bin/autoexec.bat bin/command.com bin/install.bat

zipfat16: bin\kwc8616.sys
	mkdir doc
	mkdir doc\kernel
	copy docs\*.txt doc\kernel
	copy docs\*.cvs doc\kernel
	copy docs\copying doc\kernel
	copy docs\*.lsm doc\kernel
	del doc\kernel\build.txt
	del doc\kernel\lfnapi.txt
	copy bin\kwc8616.sys bin\kernel.sys
	zip -r -k ../ke$(VERSION)16.zip $(BINLIST)
	utils\rmfiles doc\kernel\*.txt doc\kernel\*.cvs doc\kernel\*.lsm doc\kernel\copying
	rmdir doc\kernel
	rmdir doc

zipfat32: bin\kwc8632.sys
	mkdir doc
	mkdir doc\kernel
	copy docs\*.txt doc\kernel
	copy docs\*.cvs doc\kernel
	copy docs\copying doc\kernel
	copy docs\*.lsm doc\kernel
	del doc\kernel\build.txt
	del doc\kernel\lfnapi.txt
	copy bin\kwc8632.sys bin\kernel.sys
	zip -r -k ../ke$(VERSION)32.zip $(BINLIST)
	utils\rmfiles doc\kernel\*.txt doc\kernel\*.cvs doc\kernel\*.lsm doc\kernel\copying
	rmdir doc\kernel
	rmdir doc

zip: zip_src zipfat16 zipfat32

