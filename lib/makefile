#
# makefile for libm.lib
#
# $Id$
#

# $Log$
# Revision 1.2  2001/11/13 23:36:45  bartoldeman
# Kernel 2025a final changes.
#
# Revision 1.1  2001/11/04 20:10:15  bartoldeman
# Added new makefile names, utils sources, kconfig.h
#
# Revision 1.5  2001/06/03 14:16:18  bartoldeman
# BUFFERS tuning and misc bug fixes/cleanups (2024c).
#
# Revision 1.4  2001/03/19 04:50:56  bartoldeman
# See history.txt for overview: put kernel 2022beo1 into CVS
#
# Revision 1.4  2001/03/08 21:15:00  bartoldeman
# Fix from Martin Stromberg to make this fully configurable from config.mak
#
# Revision 1.3  2000/05/25 20:56:22  jimtabor
# Fixed project history
#
# Revision 1.2  2000/05/11 03:57:10  jimtabor
# Clean up and Release
#
# Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
# The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
# MS-DOS.  Distributed under the GNU GPL.
#
# Revision 1.6  1999/09/14 17:32:20  jprice
# no message
#
# Revision 1.5  1999/09/13 20:13:15  jprice
# Added !if so we can use TC2 or TC3 to compile.
#
# Revision 1.4  1999/08/25 03:19:22  jprice
# ror4 patches to allow TC 2.01 compile.
#
# Revision 1.3  1999/04/23 03:45:18  jprice
# Improved by jprice
#

!include "..\mkfiles\generic.mak"


libm.lib:	$(CLIB)
	-$(RM) libm.lib
	$(LIBUTIL) $(CLIB) $(MATH_EXTRACT) $(LIBTERM)
	$(LIBUTIL) libm $(MATH_INSERT)     $(LIBTERM)
	-$(RM) *.OBJ


clobber:        clean
	-$(RM) libm.lib status.me

clean:
	-$(RM) *.obj *.bak
