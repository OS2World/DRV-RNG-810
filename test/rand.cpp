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
compile with borlands compiler for os/2

invoke

	rand filespec file_size

	creates a file with file_size random bytes.
*/
#include <assert.h>
#include <stdio.h>
#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <stdlib.h>
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
int main(int argc,char *argv[])
{
	APIRET rv;
	HFILE handle;
	ULONG action;
	ULONG parameterBufferLength=0;
	ULONG commandBufferLength=0;

	if (argc != 3)
	{
	  cerr << "Incorrect number of arguements." << endl;
	  return 1;
	}
	rv = DosOpen( "$RANDOM$", &handle, &action, 0, 0, 1, OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, 0 );
	assert( rv == NO_ERROR );
	ifstream rand(handle);
	if(!rand)
	{
	  perror("Unable to open random number gnerator.");
	  return 2;
	};
	ofstream out(argv[1],ios::binary);
	if(!out)
	{
	  perror("Unaable to open output file.");
	  return 3;
	};
	char buf[512];
	int remain=atoi(argv[2]);
	while (remain)
	{
		int count = (remain > sizeof(buf)) ? sizeof(buf) : remain;
		rand.read(buf,count);
		int size = rand.gcount();
		out.write(buf,size);
		remain -= size;
	};
}
