//
// Copyright (c) Sirius Software 1992.
// All rights reserved.
// But the README.DOC file from Sirius Software says:
// This software is hereby placed in the public domain.
/*
 RANDDRV.SYS random number driver for RNG-810 random # or similar.
 Copyright (C) 1994 Paul Elliot

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Paul.Elliott@Hrnowl.LoneStar.Org

Paul Elliott

3986 South Gessner #224 Houston TX 77063

*/
//
//
// file_name = randdrv.h
//
// Notes
//		- created June 1, 1993 Greg Smith
//		- various defines and external declarations used by SampDrv
//


#if !defined( RANDDRV_H )

#define RANDDRV_H

// misc types and defines

#define TRUE 	1
#define FALSE   0

/* RPstatus bit values */

#define RPERR   0x8000              /*  error occurred, err in RPstatus    */
#define RPDEV   0x4000              /*  error code defined by driver       */
#define RPBUSY  0x0200              /*  device is busy                     */
#define RPDONE  0x0100              /*  driver done with request packet    */

/* error codes returned in RPstatus */

#define ERROR_WRITE_PROTECT         0x0000
#define ERROR_BAD_UNIT              0x0001
#define ERROR_NOT_READY             0x0002
#define ERROR_BAD_COMMAND           0x0003
#define ERROR_CRC                   0x0004
#define ERROR_BAD_LENGTH            0x0005    
#define ERROR_SEEK                  0x0006
#define ERROR_NOT_DOS_DISK          0x0007
#define ERROR_SECTOR_NOT_FOUND      0x0008
#define ERROR_OUT_OF_PAPER          0x0009
#define ERROR_WRITE_FAULT           0x000A
#define ERROR_READ_FAULT            0x000B
#define ERROR_GEN_FAILURE           0x000C
#define ERROR_DISK_CHANGE           0x000D
#define ERROR_WRONG_DISK            0x000F
#define ERROR_UNCERTAIN_MEDIA       0x0010
#define ERROR_CHAR_CALL_INTERRUPTED 0x0011
#define ERROR_NO_MONITOR_SUPPORT    0x0012
#define ERROR_INVALID_PARAMETER     0x0013
#define ERROR_DEVICE_IN_USE         0x0014

/*  driver device attributes word */

#define DAW_CHR    0x8000           /* 1=char, 0=block                     */
#define DAW_IDC    0x4000           /* 1=IDC available in this DD          */
#define DAW_IBM    0x2000           /* 1=non-IBM block format              */
#define DAW_SHR    0x1000           /* 1=supports shared device access     */
#define DAW_OPN    0x0800           /* 1=open/close, or removable media    */
#define DAW_LEVEL1 0x0080           /* level 1                             */
#define DAW_LEVEL2 0x0100           /* level 2 DosDevIOCtl2                */
#define DAW_LEVEL3 0x0180           /* level 3 bit strip                   */
#define DAW_GIO    0x0040           /* 1=generic IOCtl supported           */
#define DAW_CLK    0x0008           /* 1=CLOCK device                      */
#define DAW_NUL    0x0004           /* 1=NUL device                        */
#define DAW_SCR    0x0002           /* 1=STDOUT (screen)                   */
#define DAW_KBD    0x0001           /* 1=STDIN  (keyboard)                 */

/* capabilities bit strip */

#define CBS_SHD    0x0001           /* 1=shutdown/DevIOCtl2                */
#define CBS_HMEM   0x0002           /* hign memory map for adapters        */
#define CBS_PP     0x0004           /* supports parallel ports             */



typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
typedef int Boolean;

inline word MSB( word x ){
	return x >> 8;
}

inline word LSB( word x ){
	return x & 0xff;
}

inline word MSW( dword x ){
	return ( x >> 16 ) & 0xffff;
}

inline word LSW( dword x ){
	return x & 0xffff;
}

#define dim(x)  ( sizeof(x)/sizeof(x[0]) )

/****************************************************************************/
// 
// OS/2 device driver request packet structure
//
/****************************************************************************/

typedef struct {
	byte requestPacketLength;	   						// the size of the request packet
	byte blockDeviceUnitNumber;							//
	byte commandCode;			   						// type of command for the request packet
	word status;				   						// the status returned to the kernel
	byte reserved1[4];			   						//
	dword qLinkage;										//
	union {												// now the variable part of the request packet
		struct {										//
			byte reserved2;								//
			dword devHlpEntry;	 						//
			char far *initArgs;								//
		} initEntry;									//
		struct {										//
			byte logicalUnits;							//
			word csLength;								//
			word dsLength;								//
			void far *bpbAddress;							//
		} initReturn;									//
		struct {										//
			byte mediaDescriptor;						//
			dword transferAddress;						// physical address of the transfer buffer
			word bytesToTransfer;						//
			dword startingSectorNumber;					//
		} transfer;										//
		byte nonDestructiveReadChar;					//
		struct {										//
			byte categoryCode;							//
			byte functionCode;							//
			void far *parameterBuffer;						//
			void far *dataBuffer;							//
		} ioctl;										//
	};													//
} RequestPacket;										//
typedef RequestPacket far * PRequestPacket;
/****************************************************************************/
// 
// OS/2 device driver header structure
//
/****************************************************************************/

typedef struct DeviceDriverHeaderStruct {
	struct DeviceDriverHeaderStruct far *next; 	// pointer to next header, -1 if this is the end of the chain
	word deviceAttributeWord;								// attributes of the device
	void (* near cdecl strategyEntryPoint)( void );	// offset of the strategy routine
	void (* near cdecl idcEntryPoint)( void );		// offset of the IDC routine
	char name[8];												// name of device
	char reserved[8];											// reserved
} DeviceDriverHeader;

typedef enum {
	init, mediaCheck, buildBPB, reserved1, read, nonDestructiveRead, inputStatus,
	inputFlush, write, writeWithVerify, outputStatus, outputFlush, reserved2,
	deviceOpen, deviceClose, removableMedia, genericIOCTL, resetMedia,
	getLogicalDrive, setLogicalDrive, deinstall, reserved3, partitionableFixedDisk,
	getFixedDiskUnitMap, reserved4, reserved5, reserved6
} CommandCodes;

//
// OS/2 16 bit API defines, if you have access to the 16 bit headers then
// remove this code. If you don't have the 16 bit headers and library and 
// require other functions then place the function import definition in
// the .def file and add the prototype here
//

#define OS2stdout 1

#define API far pascal 


// external declarations, these are defined in the c0.asm file but are
// needed in the device driver module.

extern "C" {

extern byte memoryEnd;
extern void CodeEnd();
extern void Entry1(void);
void near Strategy1( RequestPacket far *requestPacket );
unsigned API DosWrite( unsigned handle , void far *data, int dataLength, unsigned far * returnStatus );

}

#endif

// table of request handler routines used to dispatch request calls.
typedef word near (* near DispatchFunctionType)( RequestPacket far *requestPacket );


/* end of file */
