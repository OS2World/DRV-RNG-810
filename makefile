#
# Copyright (c) Sirius Software 1992.
# All rights reserved.
# But the README.DOC from Sirius Software file says:
# This software is hereby placed in the public domain.

# RANDDRV.SYS random number driver for RNG-810 random # or similar.
# Copyright (C) 1994 Paul Elliot
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#Paul.Elliott@Hrnowl.LoneStar.Org
#
#Paul Elliott
#
#3986 South Gessner #224 Houston TX 77063
#
#
# file_name = c0.asm
#
# Notes:
#	- created June 28, 1992 GWS
#	- modified for BC++ 3.0 for DOS May 2, 1993 GWS
#

#	- modified paul elliottjan 24-94
#	- requires access to runtime source
#	- and the gnu sed program for ms-dos.

# To use you must have borland's runtime source, available from
# borland, and a copy of sed, a publicly available batch editor.


.AUTODEPEND

BASENAME = randdrv

CMODEL=c
ASMMODEL=COMPACT

# path to the compiler
COMPILER_PATH = F:\msdos\borlandc

COMPILER_FLAGS = -c -m$(CMODEL) -2 -I$(COMPILER_PATH)\include -H=$(BASENAME).sym -w-rvl

ASSEMBLER_FLAGS = -ml -s

# compile with BSS segment redirected to RBSS, DATA redirected to RDATA.
# used to compile c++ code that must remain resident after INIT.
RESIDENT= -zD_RBSS -zR_RDATA -zBDATA

#
# if you have access to the 16 bit OS2.LIB add it to the following macro
# and get rid of the import section in the .DEF linker file
#

OS2_LIB = c:\os2\doscalls.lib

.asm.obj :
	tasm $(ASSEMBLER_FLAGS)   $*,$*,$*,$*;

.c.obj :	
	bcc -S $*.c
	-del $*.ast
	ren $*.asm $*.ast
	bcc $*.c

.cpp.obj :
	bcc -S $*.cpp
	-del $*.ast
	ren $*.asm $*.ast
	bcc $*.cpp

target : turboc.cfg $(BASENAME).sys

# delete anything owned by borland, so remainder
# can be released.
propriety:
	-del rinc\rules.asi
	-rmdir rinc
	-del movmem.obj
	-del N_PCMP.OBJ

clean	:
	-del *.xrf
	-del *.sym
	-del *.sed
	-del *.lst
	-del *.ast
	-del *.obj
	-del *.bak
	-del *.dll
	-del *.map
	-del *.err
	-del *.def
	-del rinc\rules.asi
	-rmdir rinc
	-del chksum.dat chksum.pgp randdrv.chk

#clean all but the driver itself!
sclean	:
	-del *.xrf
	-del *.sym
	-del *.sed
	-del *.lst
	-del *.ast
	-del *.obj
	-del *.bak
	-del *.dll
	-del *.map
	-del *.err
	-del *.def
	-del rinc\rules.asi
	-rmdir rinc
	-del chksum.dat chksum.pgp randdrv.chk


$(BASENAME).def : makefile
	echo LIBRARY $(BASENAME) >$*.def
	echo PROTMODE>>$*.def

$(BASENAME).obj :	$(BASENAME).cpp $(BASENAME).H turboc.cfg rand.h
	bcc $(RESIDENT) -S $*.cpp
	-del $*.ast
	ren $*.asm $*.ast
	bcc $(RESIDENT) $*.cpp



turboc.cfg : makefile
	copy &&|
$(COMPILER_FLAGS)
| $<

# 
# The device driver structure requires a header in the data segment.
# To make sure this comes first place the header structure in the
# module immediately after the c0.obj file in the linker line
#
$(BASENAME).sys : c0.obj $(BASENAME).obj rand.obj $(BASENAME).def rand.obj codend.obj startup.obj start.obj movmem.obj N_PCMP.OBJ
# modules before codend are resident.
# modules after codend exist ONLY till INIT completes.
# movmem  is used by code that exists after INIT so must be specially compiled.
	tlink /m/s c0+$(BASENAME)+rand+movmem+N_pcmp+codend+startup+start,$(BASENAME),,$(COMPILER_PATH)\lib\cc.lib $(OS2_LIB),$(BASENAME)/m;
	-del $(BASENAME).sys
	ren $(BASENAME).dll $(BASENAME).sys

# movmem from the RTL must be compiled so that it's segments
# can be placed to remain resident.
movmem.obj :
	bcc -S $(RESIDENT) -I$(COMPILER_PATH)\crtl\rtlinc $(COMPILER_PATH)\crtl\clib\movmem.cas 
	-del $*.ast
	ren $*.asm $*.ast
	bcc $(RESIDENT) -I$(COMPILER_PATH)\crtl\rtlinc $(COMPILER_PATH)\crtl\clib\movmem.cas 

# create a SED control file which changes 'BSS' to 'DATA'
# and _DATA to _RDATA and _BSS to _RBSS
mod.sed:	makefile
	copy &&%
/\([ 	]\)\(segment\|SEGMENT\)\([ 	]\)/s/'BSS'/'DATA'/g
s/\(^\|[ 	,:]\)_BSS/\1_RBSS/g
s/\(^\|[ 	,:]\)_DATA/\1_RDATA/g
% $<
# change 'BSS' to 'DATA'
# change _BSS to _RBSS
# change _DATA to _RDATA


# create a copy of BORLAND'S RULES.ASI with the above changes.
RINC\RULES.ASI:	mod.sed
	-MKDIR	RINC
	sed -fmod.sed $(COMPILER_PATH)\Crtl\rtlinc\rules.asi > $<


# compile N_PCMP.ASM useing modified rules.asi.
# NPCMP.OBJ is used by movmem at runtime!
N_PCMP.OBJ:	rinc\RULES.ASI
	tasm -D__COMPACT__ /mx /s /iRINC $(COMPILER_PATH)\crtl\clib\N_PCMP.ASM,N_PCMP,N_PCMP,N_PCMP;


randdrv.chk:	makefile 
	-mkdir sum
	-del chksum.dat
	for %%x in (*.* test\*.*) do echo %%x >>sum\list.dat
	-echo "unix chksums for each file" >chksum.dat
	chksum -v -- <sum\list.dat >>chksum.dat
	-del sum\list.dat
	-rmdir sum
	pgp -sat +clearsig=on chksum.dat -u "Paul Elliott"
	-del randdrv.chk
	ren chksum.asc randdrv.chk

