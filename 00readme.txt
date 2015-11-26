; RANDDRV.SYS random number driver for RNG-810 or similar hardware
; Copyright (C) 1994 Paul Elliot

This is an OS/2 IO driver for the CALNET random number generator
model RNG-810.

To use, open the device $RANDOM$ and read random bytes.

From the hardware point of view, the RNG-810 is a very simple device.
Simply do a "in" instruction on the port (300h 302h 304h or 306h) depending
on its jumpers and you receive a random byte. The device uses Johnson noise
to get its random bits. The device is subject to one restriction: You must
wait 40usec between in port instructions or the bytes will not be randomly
independent. Without this restriction, the read routine of this IO driver
would be in essence a simple "REP INSB" instruction.

In order to get around this problem, the driver uses a timer interrupt
18.2 times per second, to read random bytes into a random bytes into
a buffer. The read routine copies this data to the application, possibly blocking
for the buffer to fill up.  Because of this this driver is much slower than it
would be required by the speed of the random number generator.

If you want, you could hack the driver to get around this problem.

One approach would be to replace the reliance on the timer interrupt
with a simple spin wait, directly reading the 0 timer. This would not
be portable, and its hogs the CPU.

The other way would to be use the HRDIRVER technique, to speed up
the timer interrupt. This does not appear to be an IBM supported technique
and may be vulnerable to future changes in the operating system.

The best way would be to talk to IBM for a supported way to increase timer
resolution.

To install:

copy RANDRV.SYS somewhere and add the following line to your CONFIG.SYS
file.

DEVICE=somewhere\RANDDRV.SYS 0X300

This assumes you have configured your random number generator to use 
port 0x300.

This code is written to use TASM (ideal mode) and borlands 16-bit c++ compiler.
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

