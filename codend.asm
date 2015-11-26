; RANDDRV.SYS random number driver for RNG-810 or similar hardware
; Copyright (C) 1994 Paul Elliot
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;Paul.Elliott@Hrnowl.LoneStar.Org
;
;Paul Elliott
;
;3986 South Gessner #224 Houston TX 77063
;use ideal mode instead of masm mode
%pagesize
IDEAL
%title	"end of resident part of _TEXT segment"
%pagesize	55,132
;show code actually assembled by fancy assembler directives!
%MACS

;must use compact because the stack is far in a driver therefore
;local data is far.
ASMMODEL EQU COMPACT


;os/2 now requires 386.
P386
;enable local symbols
locals

;
; define all sequences
macro	DefineSegment	segmentName,alignmentType,classType
segment	segmentName	alignmentType public classType
ends	segmentName	
endm
; make sure the data segment is the first one in the .sys module
;
;driver requires first group be data, second code
;driver header must be first.
group	DGROUP	_HEADER,_RDATA,_INIT_,_INITEND_,_EXIT_,_EXITEND_,_RBSS,_RBSSEND,_BSS,_BSSEND,_DATA
;
	DefineSegment	_HEADER,para,'DATA'
	DefineSegment	_DATA,para,'DATA'
	DefineSegment	_RDATA,para,'DATA'
	DefineSegment	_INIT_,word,'INITDATA'
	DefineSegment	_INITEND_,byte,'INITDATA'
	DefineSegment	_EXIT_,word,'EXITDATA'
	DefineSegment	_EXITEND_,byte,'EXITDATA'
	DefineSegment	_RBSS,word,'DATA'
	DefineSegment	_RBSSEND,byte,'DATA'
	DefineSegment	_BSS,word,'BSS'
	DefineSegment	_BSSEND,byte,'BSSEND'
;
group CGROUP	_TEXT,_ITEXT
;
	DefineSegment	_TEXT,word,'CODE'
	DefineSegment	_ITEXT,word,'CODE'


; This code defines the end of the resident portion
; of the _TEXT segment.
; defines the memory limit _CodeEnd

segment _TEXT
public	_CodeEnd
label _CodeEnd byte
ends _TEXT 
	end 	   				;
