#just in case somebody likes make better than build...
all:
	build

bin\kwc8616.sys:
	build -r wc 86 fat16

bin\kwc8632.sys:
	build -r wc 86 fat32

#use as follows: wmake -ms zip VERSION=2029
zip_src:
	cd ..\..
	zip -9 -r -k source/ke$(VERSION)src.zip source/ke$(VERSION) -i@source/ke$(VERSION)/filelist
	cd source\ke$(VERSION)

BINLIST1 = doc bin/kernel.sys bin/sys.com bin/command.com bin/config.sys
BINLIST2 = bin/autoexec.bat bin/command.com bin/install.bat

zipfat16: bin\kwc8616.sys
	mkdir doc
	copy docs\*.txt doc
	copy docs\*.cvs doc
	copy docs\copying doc
	copy docs\*.lsm doc
	del doc\build.txt
	del doc\lfnapi.txt
	copy bin\kwc8616.sys bin\kernel.sys
	zip -9 -r -k ../ke$(VERSION)_16.zip $(BINLIST1)
	zip -9 -r -k ../ke$(VERSION)_16.zip $(BINLIST2)
	utils\rmfiles doc\*.txt doc\*.cvs doc\*.lsm doc\copying
	rmdir doc

zipfat32: bin\kwc8632.sys
	mkdir doc
	copy docs\*.txt doc
	copy docs\*.cvs doc
	copy docs\copying doc
	copy docs\*.lsm doc
	del doc\build.txt
	del doc\lfnapi.txt
	copy bin\kwc8632.sys bin\kernel.sys
	zip -9 -r -k ../ke$(VERSION)_32.zip $(BINLIST1)
	zip -9 -r -k ../ke$(VERSION)_32.zip $(BINLIST2)
	utils\rmfiles doc\*.txt doc\*.cvs doc\*.lsm doc\copying
	rmdir doc

zip: zip_src zipfat16 zipfat32

