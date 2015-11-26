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
typedef char far * myfp;

extern "C" {
extern near int block(unsigned long time,unsigned int dhflag);
extern near void TimerOn(void);
extern near void TimerOff(void);
extern near void PhysToVirt( dword phystovirt,int length, myfp far * far virtaddr);
};

