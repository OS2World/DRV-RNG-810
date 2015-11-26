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

/* This is initialization code required by this specific
io driver */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <mem.h>
#include <dos.h>
#include "randdrv.h"
#include "rand.h"

#define majorVersion 	1
#define minorVersion	00
#define revision		'a'
// address indicates call back address used to call back device
// helper routines.

extern dword near _devHlp;

extern word near Error( RequestPacket far *requestPacket );
extern DispatchFunctionType dispatchTable[] ;

// status to return.
const word done = 0x0100;						//
const word error = 0x8000;						// status return bit masks
const word generalDeviceError =		0x0c;		//
const word unknownCommand = 		0x03;		// status error codes

// port to access to read random data.
extern unsigned int near port;

// size of random number buffers
#define BUF_SIZE 512
// buferr to hold random numbers comming form timer routine
extern char near buf[BUF_SIZE];
// start of above buffer
extern char near * near  buf_start;
// one byte past above buffer
extern char near * near buf_end;
// address of next character to read into the buffer
extern char near * near current_input;
// addres of next character in buffer to give to application.
extern char near * near current_output;

// This is the number of bytes of the buffer that we are currently
// moveing to various applications.
extern volatile word near pending;
// This is the number of bytes in buffer that
// do not contain unused saved random numbers that have not been given
// to a app yet
extern volatile word near unused_in_buffer;
// number of bytes this request is waiting for..
// when this goes to zero, wake blocked thread.
extern volatile signed int near waiting_for ;



// number of open files.
extern unsigned short near open_count;
// This is the number of bytes of the buffer that we are currently
// moveing to various applications.
extern volatile word near pending;


// prints during init call.
int printf( const char *fmt, ... ){
	// printf that is re-entrant and doesn't call any floating point,
	// uses OS/2 DosWrite and has a maximum string length of 255
	// after % expansion

	va_list args;
	static char buf[256];
	int len;
	unsigned int action;
	va_start( args, fmt );
	len = vsprintf( buf, fmt, args );
	DosWrite( OS2stdout, buf, len, &action );
	return len;
}


/***************************************************************************/
// init routine called when driver is initialized.
word Init( RequestPacket far *requestPacket ){
	// Initialise the device driver. We are allowed to use 
	// certain Dosxxx functions only while receiving an ini
	// request packet. The init packet contains the address of the
	// devHlp routine and we return the length of the code and data
	// segments. Typically in assembly drivers the init handling code
	// is jettisoned. Unfortunately if you wish to use things like 
	// printf and other code in the libraries it is a bit harder to
	// do this. It would be possible but the library code would have to
	// be recompiled and the segment names or classes of code that
	// would be removed would have to be changed and the order carefully
	// inspected. 
	
	// save the device helper routine.
	_devHlp = requestPacket->initEntry.devHlpEntry;	// save the devhlp pointer, needed by the devhlp functions

	// save the end of the code segment (offset).
	requestPacket->initReturn.csLength = FP_OFF( &CodeEnd );
	// save the end of the data segment (offset).
	requestPacket->initReturn.dsLength = FP_OFF( &memoryEnd );


	// The above means that data beond CodeEnd and memoryEnd
	// will not exist after this call.

	
	// initialize data shared with
	// rand.asm

	// limits of our buffer
	buf_start=buf;
	buf_end=buf+BUF_SIZE;

	// current buffer input
	current_input=buf;

	// current buffer output
	current_output=buf;

	// number of bytes pending to be transfered to various read requests.
	pending=0;

	// number of bytes in buffer that to not have unused, saved
	// random bytes.
	unused_in_buffer=BUF_SIZE;


	// waiting for is the number of bytes request is waiting
	// for when goes to 0 unblock blocked threads
	waiting_for = 0;
	
	// number of open files
	open_count=0;

	// port address
	port=0x300;

	// read in port address from command line.
	strcpy(buf,requestPacket->initEntry.initArgs);
	sscanf(buf,"%*s%i",(int far *)&port);

	// print message
	printf("\r\nRandom Number Device Driver Version: %d.%02d%c\r\n"
        "RANDDRV version 00, Copyright (C) 1984 Paul Elliott\r\n"
        "RANDDRV comes with ABSOLUTELY NO WARRANTY; for details\r\n"
        "see documentation.  This is free software, and you are welcome\r\n"
        "to redistribute it under certain conditions; see docs for details.\r\n" ,
		majorVersion, minorVersion, revision );

	// start interrrupt timer.
	// TimerOn();  // don't start here. start on open

	// init routine can only be called once
	// goes out of memory when first called.
	// remove the vector to this routine from dispatch table.
	dispatchTable[0] = Error;
	// this routine will cease to exist after this call
	// its memory returned to system.

	return done;
}


