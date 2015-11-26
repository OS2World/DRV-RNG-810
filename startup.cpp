/*
*
* Copyright (c) Sirius Software 1992.
* All rights reserved.
* But the README.DOC file from Sirius Software says:
* This software is hereby placed in the public domain.
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
/* this is initialization code that may be used by
any IO driver. Initializes bcc c++ global and static
objects.
*/
/*
*
*  file_name = startup.cpp
*
*   Notes :
*		- created May 9 1993 Greg Smith
*		- startup code for Borland C++ version 3.0
*/

//
// This routine is used to call the startup code for the libraries and
// also call the C++ constructors
//

#include <stdio.h>

typedef unsigned char byte;

typedef struct {
	byte functionCallType;	// 0x00 = near call, 0x01 = far call, anything else ignore
	byte priority;			// 0x00 = highest priority, 0xff = lowest priority
	union {
		void (far * farFunction)(void);
 		void (near * nearFunction)(void);
	};
} StartupEntry;

extern "C" {

void near CallStartup( StartupEntry far *startOfStartup, StartupEntry far *endOfStartup );

}

void near CallStartup( StartupEntry far *startOfStartup, StartupEntry far *endOfStartup ){
	StartupEntry far *s;
	int priority, count;
	for	( priority=255; priority >= 0; priority-- ){
		s = startOfStartup;
		count = endOfStartup - s;
		while	(count--){
			if	( s->priority == priority ){
				if	( s->functionCallType == 0 ){
					s->nearFunction();
				} else if	( s->functionCallType == 1 ){
					s->farFunction();
				}
			}
			s++;
		}
	}
}

// end of file
