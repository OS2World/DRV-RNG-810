/*
*
* Copyright (c) Sirius Software 1992.
* All rights reserved.
* But the README.DOC from Sirius Software file says:
* This software is hereby placed in the public domain.
*
*/
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

/*
*  file_name = randdrv.cpp
*
*   Notes :
*		- created June 1, 1993 Greg Smith
*		- sample OS/2 device driver. This is very brain dead and it just
*		installs itself and any other call returns errors. Generic IOCTL 
*		is supported and is used to get the device driver version string.
*/

//#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
#include <mem.h>
#include <dos.h>
#include "randdrv.h"
#include "rand.h"

#define majorVersion 	1
#define minorVersion	00
#define revision		'a'

// 
// This MUST be the first data declaration that allocates space in the
// data segment
//
#if 0
// There is nothing in the c or c++ language, that requires
// that the first data item in the source file
// will be the first element in memory!
// This happens (by chance ) to work, but
// may not in future versions of the compiler/
// Therefore I am commenting this declaration out
// and declaring header in assembly lauague. PE
DeviceDriverHeader Header = {
	(struct DeviceDriverHeaderStruct far *)-1L,
	DAW_OPN | DAW_CHR | DAW_LEVEL1 |DAW_GIO | DAW_SHR,
	//0x80c0,	// character device driver, function level 001, Generic IOCTL supported
	// sharable as well
	Entry1,			// entry point to call driver, defined in c0.asm
	0,
	// name of device
	{ 'R', 'A', 'N', 'D', '0', '0', '0', '1' }
};
#else
extern DeviceDriverHeader Header;
#endif

// request handler routines.
extern word near Init( RequestPacket far *requestPacket );
//word near GenericIOCTL( RequestPacket far *requestPacket );
word near Error( RequestPacket far *requestPacket );
word near Done( RequestPacket far *requestPacket );
word near Open( RequestPacket far *requestPacket );
word near Close( RequestPacket far *requestPacket );
word near Deinstall( RequestPacket far *requestPacket );
word near Read( RequestPacket far *requestPacket );
word near ReadNoWait( RequestPacket far *requestPacket );
word near InputStatus( RequestPacket far *requestPacket );


DispatchFunctionType dispatchTable[] = {
	Init,		 			// 0 = Initialise
	Error,					// 1 = Media Check
	Error,					// 2 = Build Bios Parameter Block
	Error,					// 3 = reserved
	Read,					// 4 = Read
	ReadNoWait,					// 5 = Non Destructive Read
	InputStatus,					// 6 = Input Status
	Error,					// 7 = Input Flush
	Error,					// 8 = Write
	Error,					// 9 = Write with verify
	Error,					// 10 = output status
	Error,					// 11 = output flush
	Error,					// 12 = reserved
	Open,					// 13 = open
	Close,					// 14 = close
	Error,					// 15 = Removable Media
//	GenericIOCTL,			// 16 = Generic IOCTL
	Error,			// 16 = Generic IOCTL
	Error,					// 17 = Reset Media
	Error,					// 18 = Get Logical Drive
	Error,					// 19 = Set Logical Drive
	Deinstall,				// 20 = Deinstall
	Error,					// 21 = reserved
	Error,					// 22 = Partitionable Fixed Disk
	Error,					// 23= Get Fixed Disk Unit Map
	Error,					// 24 = reserved
	Error,					// 25 = reserved
	Error					// 26 = reserved
};

// address indicates call back address used to call back device
// helper routines.
dword near _devHlp;

// status to return.
const word done = 0x0100;						//
const word error = 0x8000;						// status return bit masks
const word generalDeviceError =		0x0c;		//
const word unknownCommand = 		0x03;		// status error codes

// port to access to read random data.
unsigned int near port;

// size of random number buffers
#define BUF_SIZE 512
// buferr to hold random numbers comming form timer routine
char near buf[BUF_SIZE];
// start of above buffer
char near * near  buf_start=buf;
// one byte past above buffer
char near * near buf_end=buf+BUF_SIZE;
// address of next character to read into the buffer
char near * near current_input=buf;
// addres of next character in buffer to give to application.
char near * near current_output=buf;

// This is the number of bytes of the buffer that we are currently
// moveing to various applications.
volatile word near pending=0;
// This is the number of bytes in buffer that
// do not contain unused saved random numbers that have not been given
// to a app yet
volatile word near unused_in_buffer=BUF_SIZE;
// number of bytes this request is waiting for..
// when this goes to zero, wake blocked thread.
volatile signed int near waiting_for = 0;


// number of open files.
unsigned short near open_count=0;
/***************************************************************************/


/***************************************************************************/

// main request dispatcher.
void near Strategy1( RequestPacket far *requestPacket ){
	// All OS/2 device I/O passes through here
	if	( requestPacket->commandCode < dim( dispatchTable ) ){
		// call the appropriate packet handler
		requestPacket->status = dispatchTable[ requestPacket->commandCode ]( requestPacket );
	} else {
		// a strange packet was received, handle the error
		requestPacket->status = error | done | unknownCommand;
	}
}

/***************************************************************************/

// open one file, inc open file counter.
word Open( RequestPacket far *requestPacket ){
	if ( ++open_count == 1 ) TimerOn();
	return(done);
}
// remove driver, turn off its timer.
word Deinstall( RequestPacket far *requestPacket ){
	TimerOff();
	return(done);
}
// close file, dec open file counter.
word Close( RequestPacket far *requestPacket ){
	if (--open_count == 0) TimerOff();
	return(done);
}

// read one byte with now wait.
word ReadNoWait( RequestPacket far *requestPacket ){
	// assume buffer not empty
	word busy=0;

	// if no saved random bytes
	if (unused_in_buffer == BUF_SIZE)
	{
		// declare device busy.
		busy= RPBUSY;
	}
	else
	{
		// return frist char in buffer
		requestPacket->nonDestructiveReadChar = *current_output;
	}
	// return ok
	return (RPDONE |busy);
}

// if buffer has characters, OK, else return bussy
word InputStatus( RequestPacket far *requestPacket ){

	return(done  | ( (unused_in_buffer == BUF_SIZE)? RPBUSY : 0 ) );
}

// read request to read the buffer
word Read( RequestPacket far *requestPacket ){

	// virtual address to map
	auto char far* virtaddr;
	// number of bytes remaining to be transfered to application.
	auto word remain = requestPacket->transfer.bytesToTransfer;

	// bytes used.
	auto word used;

	// true if the virtual address is mapped
	// blocking allways undoes mapping.
	auto word mapped = 0;

	// while there are bytes to return...
	while ( remain > 0)
	{
		

	  // disable interupts, get free bytes = bytes in buffer - #of
	  // bytes waiting to be transfer.
	  // keep blocking untill we get some bytes.
	  while ( disable(),(used = (BUF_SIZE -unused_in_buffer) - pending),
		(used  == 0))
	  {
	  	word count = buf_end - current_output;
	  	if ( count > remain) count = remain;
	  	if ( waiting_for <= 0 ) waiting_for = count;
	  	
	  	// we are doing a block, which unmapps buffer,
		// if it had been mapped.
	  	mapped = 0;
	  	// save current status of buffer to check for change.
	  	word orig_unused = unused_in_buffer;
	  	int err;

	  	err = block(41000L,0) & 0xff;

	  	//enable interupts, check for error.
		enable();
	  	mapped = 0;

	  	// return errors if found
	  	if ( ( orig_unused != unused_in_buffer) &&(err != 2) )
		  err=0;

	  	if (err > 0) requestPacket->transfer.bytesToTransfer = 0;
	  	if (err == 2)
	  	{
	  	   return RPDONE|RPERR|ERROR_CHAR_CALL_INTERRUPTED;
	  	}
	  	if ( err == 1)
	  	{
	  	   return RPDONE|RPERR|ERROR_NOT_READY;
	  	}
	  }
	  // we fall from above loop with interrupts disabled.


	  // get bytes to transfer, min of
	  // a) space from here to end of buffer
	  // b) bytes remaining to transfer of those originally requested
	  //	by user.
	  // c) number of bytes in buffer - # that other requests my
	  // 	be transfering.
	  word count = buf_end - current_output;
	  if ( count > remain) count = remain;
	  if ( count > used) count = used;

	  // allocate space in buffer for transfer
	  auto char near * pending_buffer = current_output;
	  // update the place in buffer to use for next requests
	  current_output += count;
	  // but wrapp around to beginning if we are at end.
	  if (current_output == buf_end) current_output = buf_start;

	  // we are now tring to tranfer count bytes, so increment
	  // pending by count.
	  pending += count;

	  // enable the interrups that have been disabled
	  enable();

	  // get offset within user buffer to transfer to
	  auto word offset =
		(requestPacket->transfer.bytesToTransfer - remain);

	  // if there are bytes to transfer
	  if (count > 0) {
	     // if the physical address has not been mapped to
	     // a virtual address, or if that address has been
	     // invalidated by a block request, the convert the
	     // physical address to a virtual address
	     if (!mapped)
	     {
		PhysToVirt(requestPacket->transfer.transferAddress,
			requestPacket->transfer.bytesToTransfer, &virtaddr );
	     	mapped = 1;		// vir address has now bee maped.
	     }
	     // move the data to users buffer!
	     memmove(virtaddr+offset,(char far *)pending_buffer,count);
	  }

	  // decrement amount of bytes to transfer to user
	  remain -= count;

	  // disable interrups
	  disable();
	  // reclassify bytes that were pending as unused bytes

	  // decrement pending
	  pending -= count;
	  // increment unused bytes
	  unused_in_buffer += count;
	  // enable interrupts again
	  enable();
	  
	}		// loop till no more devices to transfer.
	return done;
};

// error request.
word Error( RequestPacket far *requestPacket ){
	// a request packet that we don't know how to handle has been
	// received, return an error to the kernel
	(void)requestPacket;	// keep the warnings from the compiler down
	return error | done | unknownCommand;
}
// request successfully.
word Done( RequestPacket far *requestPacket ){
	// a request packet that we don't know how to handle has been
	// received, return an error to the kernel
	(void)requestPacket;	// keep the warnings from the compiler down
	return done ;
}
#if 0
/***************************************************************************/
//
// function: VerifyAccess
// input   : pointer to data, length of data, access type required
// return  :  TRUE if access is allowed, False otherwise
// Note    :
//	- this is an example of DevHlp code using BC++ inline assembly to access
//	devHlp functions. 
//
/***************************************************************************/

Boolean VerifyAccess( void far *data, word length, word accessType ){
	// accessType = 0 for read only access
	// accessType = 1 for read/write access
	// returns TRUE if access is allowed, FALSE otherwise
	//
	asm	mov		ax,word ptr data + 2
	asm mov		di,word ptr data
	asm	mov		cx,length
	asm	mov		dh,byte ptr accessType
	asm	mov		dl,0x27					// function code for VerifyAccess
	asm	call	_devHlp
	asm	jc		returnFalse
	return TRUE;
returnFalse:
	return FALSE;
}
/***************************************************************************/

word GenericIOCTL( RequestPacket far *requestPacket ){
	// This is the Generic IOCTL code. Currently only one category and
	// one function type is supported but the structure is done in a
	// way that allows easy modification to handle more categories of
	// functions. 
	word *wp;
	byte *bp;
	switch	( requestPacket->ioctl.categoryCode ) {
		case 0x80 :
			// device specific IOCTL category
			switch ( requestPacket->ioctl.functionCode ){
				case 0x01 :		// version name function
					// read the device driver version, it is returned to
					// the caller as a string.
					char buf[ 80 ];
					sprintf( buf, "%d.%02d%c", majorVersion, minorVersion, revision );
					if	( VerifyAccess( requestPacket->ioctl.dataBuffer, strlen( buf ) + 1, 1 ) ){
						// we have read/write access to the callers memory where
						// the string will be copied ( 1 extra char for the NULL terminator )
						strcpy( (char *)requestPacket->ioctl.dataBuffer, buf );
						return done;
					} else {
						// the memory is not accessible, return an error
						return error | done | generalDeviceError;
					};
				case 0x02 :		// version name function
					// return the number of open files.
					if	( VerifyAccess( requestPacket->ioctl.dataBuffer, sizeof(open_count), 1 ) ){
						// we have read/write access to the callers memory where
						// the string will be copied ( 1 extra char for the NULL terminator )
						memcpy( requestPacket->ioctl.dataBuffer, (char *)&open_count ,
							sizeof(open_count) );
						return done;
					} else {
						// the memory is not accessible, return an error
						return error | done | generalDeviceError;
					};
			}	// end switch( requestPacket->ioctl.functionCode )
	} // end switch	( requestPacket->ioctl.categoryCode )
	return error | done | unknownCommand;
}
#endif
// end of file
