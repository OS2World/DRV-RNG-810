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
;
;
%pagesize
;use ideal mode instead of masm mode
IDEAL
%title	"assembly language interface for os/2 random number generator driver"
%pagesize	55,132
;show code actually assembled by fancy assembler directives!
%MACS

;must use compact because the stack is far in a driver therefore
;local data is far.
ASMMODEL EQU COMPACT
;smart code generation
smart


;os/2 now requires 386.
P386
;enable local symbols
locals
%pagesize
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
group CGROUP	_TEXT,_ITEXT
;
	DefineSegment	_TEXT,word,'CODE'
	DefineSegment	_ITEXT,word,'CODE'
	DefineSegment	_HEADER,para,'DATA'
	DefineSegment	_RDATA,para,'DATA'
	DefineSegment	_DATA,para,'DATA'
	DefineSegment	_INIT_,word,'INITDATA'
	DefineSegment	_INITEND_,byte,'INITDATA'
	DefineSegment	_EXIT_,word,'EXITDATA'
	DefineSegment	_EXITEND_,byte,'EXITDATA'
	DefineSegment	_RBSS,word,'DATA'
	DefineSegment	_RBSSEND,byte,'DATA'
	DefineSegment	_BSS,word,'BSS'
	DefineSegment	_BSSEND,byte,'BSSEND'
%pagesize
;
;
;
; use 16 bit addressing in os/2 driver
; use c language for interface to c,c++ code
	model USE16 ASMMODEL,C
	ASSUME CS:CGROUP
include "driver.inc"
SEGMENT _ITEXT
extrn	NOLANGUAGE _Entry1:far
ENDS	_ITEXT

; define the header for the device driver.
segment _HEADER
Public	Header
label	Header	far
	DD	-1			;NEXT
	DW	DAW_OPN OR DAW_CHR OR DAW_LEVEL1 OR DAW_GIO OR DAW_SHR
					;device attribute word

; this word defines the entry point for the driver.
Public	Entry
label	Entry	near
; initial entry point is _Entry1
; when Entry1 runs at first call, it will change this pointer
; to point to Entry2, therafter Entry2 will be called instead!
	DW	offset CGROUP:_Entry1		;strategy entry point
	DW	0			;idc entry point
	DB	8 DUP ("$RANDOM$")	;name exactly 8 bytes
	DB	8 DUP (0)		;RESERVED
ends	_HEADER	
%pagesize
;
;define constants needed to call back the device helper services.
ProcBlock        =      4      ;  4    Block on event
ProcRun          =      5      ;  5    Unblock process

SetTimer         =     29      ; 1D    set timer r=est handler
ResetTimer       =     30      ; 1E    unset timer request handler

CPhysToVirt       =     21      ; 15    convert physical address to virtual
VirtToPhys       =     22      ; 16    convert virtual address to physical
PhysToUVirt      =     23      ; 17    convert physical to LDT
%pagesize
;DATASEG
segment _RDATA
;define data shared in common with randdrv.cpp
;call back dev helper address
extrn	_devHlp:DWORD

;the number of bytes we are waiting for
extrn	waiting_for:word
;the number of unused random bytes stored in our buffer
extrn	unused_in_buffer:word
; the address of the start of our buffer (offset)
extrn	buf_start:word
; the address of the end of our buffer (offset)
extrn	buf_end:word
;The offset address of the next byte to be read into our buffer
extrn	current_input:word
; The port address of the random number gneerator
extrn	port:word
ENDS _RDATA
%pagesize
CODESEG	 
;define our public functions
;turn timer interrupts on
public	TimerOn
; turn timer interrrupts off
public	TimerOff
; cause current process to block for event
public  block
; map a physical address to a virtual
public	PhysToVirt


;call the device driver help routines to cause this request/thread to block
proc	block	near
	;time to block this request
	arg	@@time:dword
	;flag to pass to helper routine
	arg	@@dflag:word
	;we use these registers
	uses	bx,cx,dx,si,di
	;load registers for ID
	mov	bx,SEG Header
	mov	ax,OFFSET Header
	;load registers for time to block
	mov	cx,[word @@time]
	mov	di,[word @@time+2]
	;load registers for flags indicating kind of block
	mov	dh,[byte @@dflag]
	;procedure block routine
	mov	dl,ProcBlock
	;call back device helper routines.
	call	[_devHlp] NOLANGUAGE
	;if carry some error
	jc	short @@10
	xor	ax,ax	;zero indicates normal unblock, woke by a process
	;done
@@99:	ret
@@10:	jz	short @@20
	;if c set and z is set then timeout code
	mov	ax,2	;return interrrupted flag
	jmp	short @@99
@@20:	mov	ax,1	;timed out flat
	jmp	short @@99

endp	block

%pagesize
;this routine is called each for timer interrupt =18.2 times/second
;this routine reads bytes from the random number generator.
;this is neccessary because the calnet rng810 random number
; generator does not return randomly independant numbers if
; you read it faster than 40usec.
; 18.2 times per second is way to slow making this driver
; to slow making this driver extreamly slower than the requirements
; of the hardware. I do not see any way of getting faster timer
; resolution.
proc	NOLANGUAGE _TIM_HNDLR FAR
	;save all registers
	pusha
	push	si di es
	;is our buffer full?
	cmp	[unused_in_buffer],0
	; if full skip getting a new random byte.
	jz	short @@100
	;make es point to ds=our data segment
	mov	ax,ds
	mov	es,ax
	; offset to read random byte to
	mov	di,[current_input]
	;port to read
	mov	dx,[port]
	cld				;go forward direction
	;read one new byte.
	insb
	;have we reached the end of the buffer
	cmp	di,[buf_end]
	;if yes skip
	jne	short @@10
	;if at end wrap around to the begginig of the buffer
	mov	di,[buf_start]
	;store new location to store next byte.
@@10:	mov	[current_input],di

	;decrease number of free unused bytes in buffer.
	dec	[unused_in_buffer]
;;;	jz	short @@20

	; decrement the number of chars we are waiting for
	dec	[waiting_for]
	;if characters are available now unblock
	jne	short @@100

@@20:	mov	[waiting_for],0
	;set	id in registers to run blocked processes.
	mov	bx,SEG Header
	mov	ax,OFFSET Header
	;indicate run process request to device helper
	mov	dl,ProcRun
	;activate device helper call back function.
	call	[_devHlp] NOLANGUAGE

	;pop saved all registers and return
@@100:
	pop	es di si
	popa
@@200:
	ret
endp	_TIM_HNDLR

%pagesize
;run timer interrupt routine.
proc	TimerOn	near
	uses	bx,cx,dx,si,di
	;offset of timer routine
	mov	ax,offset _TIM_HNDLR
	;indicate set timer request
	mov	dl,SetTimer
	; call back device helper routine
	call	[_devHlp] NOLANGUAGE
	ret
endp	TimerOn
proc	TimerOff near
	uses	bx,cx,dx,si,di
	;offset of timer routine
	mov	ax,offset _TIM_HNDLR
	; indicate reset timer request
	mov	dl,ResetTimer
	; call back device helper routine
	call	[_devHlp] NOLANGUAGE
	ret
endp	TimerOff

;map physical address to a virtual address
proc	PhysToVirt	near
	;physical address of buffer
	arg	@@phyaddr:dword
	;length of buffer
	arg	@@length:word
	;address ad which to return virtual address
	arg	@@retaddr:dword
	;registers that we use
	uses	bx,cx,dx,si,di,es

	;indicate physical address in registers
	mov	bx,[word @@phyaddr]
	mov	ax,[word @@phyaddr+2]
	;indicate length of buffer
	mov	cx,[@@length]
	;indicate return address in es:di
	mov	dh,1
	;indicate map physical to virtual address
	mov	dl,CPhysToVirt
	;call back device helper routine.
	call	[_devHlp] NOLANGUAGE

	;save address segment in dx
	mov	dx,es

	;make es:si point to place to return address to caller
	les	si,[@@retaddr]

	;return offset to caller
	mov	[word es:si],di
	;return segment to caller.
	mov	[word es:si+2],dx

	ret				;done
endp	PhysToVirt
	end 	   				;

