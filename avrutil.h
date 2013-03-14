/*
 * avrutil.h
 *
 * Copyright (C) 2010 Razvan Tataroiu, razvan784@gmail.com .
 * 
 * Useful macro definitions for AVR microcontrollers
 * + transparent assignation of functionality / signal names to 
 * individual port pins and generation of optimal code depending
 * of their placement to certain pins or ports (on the same port
 * or on different ports). E.g. do a PORT= instead of multiple writes
 * to the same port (ports are defined with volatile attribute,
 * thus consecutive writes cannot be optimized by the compiler).
 * + typedefs
 * + busywaiting delay macros: includes the avrlibc ones, adds _delay_ns
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301, USA
 */

#ifndef _AVRUTIL_H_
#define _AVRUTIL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Short typedefs */
typedef unsigned char u08;
typedef u08 u8;
typedef unsigned int u16;
typedef unsigned long u32;
typedef char i08;
typedef short i16;
typedef long i32;
typedef char bool;
#define true 1
#define false 0

/* Some useful macros */
#ifndef sbi
#define sbi(stuff, bit) (stuff) |=  _BV(bit);
#endif
#ifndef cbi
#define cbi(stuff, bit) (stuff) &= ~_BV(bit);
#endif
#ifndef cli
#define cli() __asm__ __volatile__ ("cli" ::)
#endif
#ifndef sei
#define sei() __asm__ __volatile__ ("sei" ::)
#endif
#ifndef nop
#define nop() __asm__ __volatile__ ("nop" ::)
#endif
#ifndef wdr
#define wdr() __asm__ __volatile__ ("wdr" ::)
#endif

#define CAT(a,b) a ## b
#define CATX(a,b) CAT(a,b)

/*
   Pin functionality assignment macros
   with optimized access and easy portability

Usage:
#define FUNCTION1_PORT A
#define FUNCTION1_PIN  3
#define FUNCTION1_POL  1

#define FUNCTION2_PORT A
#define FUNCTION2_PIN  6
#define FUNCTION2_POL  0

write_pins(
    set_pin(FUNCTION1);  //request write
    clr_pin(FUNCTION2);  //request write
);                       //do actual write

This should be properly optimized by the compiler
so as to be equivalent with PORTA |= _BV(3) | _BV(6)

To set pin directions as well as values:
write_pins_dir(
	set_pin_output(FUNCTION1);
	set_pin(FUNCTION1);
	set_pin_input(FUNCTION2);
);

To set only pin directions, keeping the current values
(allows stronger optimization):
set_pin_directions(
	set_output(FUNCTION1);
	set_input(FUNCTION2);
);

To read pin values in an optimized fashion (e.g. don't query the
same port 5 times if the 5 functions happen to be on the same
port, knowing that on a different platform they might not be)
one should first port_optimize_declare(); then
read_pins(
    read_pin(FUNCTION1);   //request read
    read_pin(FUNCTION2);
);                         //do actual read
x = get_pin(FUNCTION1);    //use value
if (get_pin(FUNCTION2)
    do_some_stuff();
else
    do_something_else();

If any of the set_pin, clr_pin, set_output, set_input macros
is called outside a block, they act immediately. In this case
one also needs to port_optimize_declare(); in each function
before use.
If read_pin is called outside the block it reads and returns
the value immediately. Do not call get_pin without fisrt calling
a read_pins block. After a read_pins block, get_pin can be called
any number of times.

The set, clr, read and get macros have a variant with an _absolute
suffix which writes/reads the absolute logic value, ignoring the
polarity specification, which work both inside and outside optimizing
blocks, with the exception of read_pin_absolute which only works
outside, because if it were used inside it would have the same meaning
as read_pin.

Disclaimer: no warranty. Always check the assembly dump to insure that
optimizations are actually taking place. In some cases the code
generated can be much worse than simply calling the macros outside
the optimizing blocks or using classic techniques.
*/

#define _UNMMIO8(dptr) ((u08 *)&dptr) // convert dereferenced volatile pointer to a pointer - obtain pointer to named register ("undo MMIO8")
#define _NULLDEREF (*((u08 *)0)) // dereference a null pointer :)

// if some ports do not exist, define them as null.
#ifndef PORTA
#define PORTA _NULLDEREF
#endif
#ifndef PORTB
#define PORTB _NULLDEREF
#endif
#ifndef PORTC
#define PORTC _NULLDEREF
#endif
#ifndef PORTD
#define PORTD _NULLDEREF
#endif
#ifndef PORTE
#define PORTE _NULLDEREF
#endif
#ifndef PORTF
#define PORTF _NULLDEREF
#endif
#ifndef DDRA
#define DDRA _NULLDEREF
#endif
#ifndef DDRB
#define DDRB _NULLDEREF
#endif
#ifndef DDRC
#define DDRC _NULLDEREF
#endif
#ifndef DDRD
#define DDRD _NULLDEREF
#endif
#ifndef DDRE
#define DDRE _NULLDEREF
#endif
#ifndef DDRF
#define DDRF _NULLDEREF
#endif
#ifndef PINA
#define PINA _NULLDEREF
#endif
#ifndef PINB
#define PINB _NULLDEREF
#endif
#ifndef PINC
#define PINC _NULLDEREF
#endif
#ifndef PIND
#define PIND _NULLDEREF
#endif
#ifndef PINE
#define PINE _NULLDEREF
#endif
#ifndef PINF
#define PINF _NULLDEREF
#endif

#define optimized_port_write(port, set, clr) \
if (_UNMMIO8(port)) { /* port exists */ \
	if (set) { \
		if (set == 0xff) \
			port = 0xff; \
		else \
			if (!clr) \
				port |= set; \
			else if ((set | clr) == 0xff) /* all bits are specified, no need to know prev val */ \
				port = set; \
			else \
				port = (port | set) & ~clr; \
	} \
	else if (clr == 0xff) \
		port = 0; \
	else if (clr) \
		port &= ~clr; \
}

#define optimized_set_input(ddr, pins) \
if (_UNMMIO8(ddr) && pins) { \
	if (pins == 0xff) \
		ddr = 0; \
	else \
		ddr &= ~pins; \
}

#define optimized_set_output(ddr, pins) \
if (_UNMMIO8(ddr) && pins) { \
	if (pins == 0xff) \
		ddr = 0xff; \
	else \
		ddr |= pins; \
}

#define port_optimize_declare() \
	u08 _setA = 0, _setB = 0, _setC = 0, _setD = 0, _setE = 0, _setF = 0; \
	u08 _clrA = 0, _clrB = 0, _clrC = 0, _clrD = 0, _clrE = 0, _clrF = 0; \
	u08 _sezA = 0, _sezB = 0, _sezC = 0, _sezD = 0, _sezE = 0, _sezF = 0; \
	u08 _clzA = 0, _clzB = 0, _clzC = 0, _clzD = 0, _clzE = 0, _clzF = 0; \
	u08 _rdA = 0, _rdB = 0, _rdC = 0, _rdD = 0, _rdE = 0, _rdF = 0; \
	u08 _port_optimize_in_block = 0; if (_port_optimize_in_block); \
	if (_setA); if (_setB); if (_setC); if (_setD); if (_setE); if (_setF); \
	if (_clrA); if (_clrB); if (_clrC); if (_clrD); if (_clrE); if (_clrF); \
	if (_sezA); if (_sezB); if (_sezC); if (_sezD); if (_sezE); if (_sezF); \
	if (_clzA); if (_clzB); if (_clzC); if (_clzD); if (_clzE); if (_clzF); \
	if (_rdA); if (_rdB); if (_rdC); if (_rdD); if (_rdE); if (_rdF)

// write pins by defined functionality
// * initialize variables that describe how each port should be updated
// * allow user to specify which signals to set/clear
// * call optimized write for each port, updating them if necessary
#define write_pins(statements) { \
	u08 _setA = 0, _setB = 0, _setC = 0, _setD = 0, _setE = 0, _setF = 0; \
	u08 _clrA = 0, _clrB = 0, _clrC = 0, _clrD = 0, _clrE = 0, _clrF = 0; \
	u08 _port_optimize_in_block = 1; \
	statements \
	optimized_port_write(PORTA, _setA, _clrA); \
	optimized_port_write(PORTB, _setB, _clrB); \
	optimized_port_write(PORTC, _setC, _clrC); \
	optimized_port_write(PORTD, _setD, _clrD); \
	optimized_port_write(PORTE, _setE, _clrE); \
	optimized_port_write(PORTF, _setF, _clrF); \
	_port_optimize_in_block = 0; \
}

#define _set_pin_1(portletter, pin) CAT(_set, portletter) |= _BV(pin)
#define _set_pin_2(portletter, pin) CAT(PORT, portletter) |= _BV(pin)
#define _clr_pin_1(portletter, pin) CAT(_clr, portletter) |= _BV(pin)
#define _clr_pin_2(portletter, pin) CAT(PORT, portletter) &= ~_BV(pin)

#define set_pin_absolute(func) \
	if (_port_optimize_in_block) \
		_set_pin_1(CAT(func, _PRT), CAT(func, _PIN)); \
	else \
		_set_pin_2(CAT(func, _PRT), CAT(func, _PIN))

#define clr_pin_absolute(func) \
	if (_port_optimize_in_block) \
		_clr_pin_1(CAT(func, _PRT), CAT(func, _PIN)); \
	else \
		_clr_pin_2(CAT(func, _PRT), CAT(func, _PIN))

#define set_pin(func) \
	if (CAT(func, _POL)) \
		set_pin_absolute(func); \
	else \
		clr_pin_absolute(func)

#define clr_pin(func) \
	if (CAT(func, _POL)) \
		clr_pin_absolute(func); \
	else \
		set_pin_absolute(func)

#define _toggle_pin_2(portletter, pin) CAT(PORT, portletter) ^= _BV(pin)
#define toggle_pin(func) _toggle_pin_2(CAT(func, _PRT), CAT(func, _PIN))

/*
Setting pin values as well as directions (input/output)

PORT DDR
0    0    in  float
0    1    out 0
1    1    out 1
1    0    in  pull

usu. all inputs have pullup.
hiz enable:
  write DDR, write port - avoid hard toggle
hiz disable:
  write port, write DDR - same.
safe sequence for any operation:
  set inputs, write port, set outputs
*/
#define write_pins_dir(statements) { \
	u08 _setA = 0, _setB = 0, _setC = 0, _setD = 0, _setE = 0, _setF = 0; \
	u08 _clrA = 0, _clrB = 0, _clrC = 0, _clrD = 0, _clrE = 0, _clrF = 0; \
	u08 _sezA = 0, _sezB = 0, _sezC = 0, _sezD = 0, _sezE = 0, _sezF = 0; \
	u08 _clzA = 0, _clzB = 0, _clzC = 0, _clzD = 0, _clzE = 0, _clzF = 0; \
	u08 _port_optimize_in_block = 1; \
	statements \
	optimized_set_input (DDRA, _sezA); \
	optimized_set_input (DDRB, _sezB); \
	optimized_set_input (DDRC, _sezC); \
	optimized_set_input (DDRD, _sezD); \
	optimized_set_input (DDRE, _sezE); \
	optimized_set_input (DDRF, _sezF); \
	optimized_port_write(PORTA, _setA, _clrA); \
	optimized_port_write(PORTB, _setB, _clrB); \
	optimized_port_write(PORTC, _setC, _clrC); \
	optimized_port_write(PORTD, _setD, _clrD); \
	optimized_port_write(PORTE, _setE, _clrE); \
	optimized_port_write(PORTF, _setF, _clrF); \
	optimized_set_output(DDRA, _clzA); \
	optimized_set_output(DDRB, _clzB); \
	optimized_set_output(DDRC, _clzC); \
	optimized_set_output(DDRD, _clzD); \
	optimized_set_output(DDRE, _clzE); \
	optimized_set_output(DDRF, _clzF); \
	_port_optimize_in_block = 0; \
}

/* Setting just the directions - can be further optimized
 * by oring and anding the DDR with a single read and write.
 * very similar to write_pins */
#define set_pin_directions(statements) { \
	u08 _sezA = 0, _sezB = 0, _sezC = 0, _sezD = 0, _sezE = 0, _sezF = 0; \
	u08 _clzA = 0, _clzB = 0, _clzC = 0, _clzD = 0, _clzE = 0, _clzF = 0; \
	u08 _port_optimize_in_block = 1; \
	statements \
	optimized_port_write(DDRA, _sezA, _clzA); \
	optimized_port_write(DDRB, _sezB, _clzB); \
	optimized_port_write(DDRC, _sezC, _clzC); \
	optimized_port_write(DDRD, _sezD, _clzD); \
	optimized_port_write(DDRE, _sezE, _clzE); \
	optimized_port_write(DDRF, _sezF, _clzF); \
	_port_optimize_in_block = 0; \
}

#define _set_pin_out_1(portletter, pin) CAT(_clz, portletter) |= _BV(pin)
#define _set_pin_out_2(portletter, pin) CAT(DDR, portletter) |= _BV(pin)
#define _set_pin_inp_1(portletter, pin) CAT(_sez, portletter) |= _BV(pin)
#define _set_pin_inp_2(portletter, pin) CAT(DDR, portletter) &= ~_BV(pin)

#define set_pin_input(func) \
	if (_port_optimize_in_block) \
		_set_pin_inp_1(CAT(func, _PRT), CAT(func, _PIN)); \
	else \
		_set_pin_inp_2(CAT(func, _PRT), CAT(func, _PIN))

#define set_pin_output(func) \
	if (_port_optimize_in_block) \
		_set_pin_out_1(CAT(func, _PRT), CAT(func, _PIN)); \
	else \
		_set_pin_out_2(CAT(func, _PRT), CAT(func, _PIN))

/* Reading pin values 
BEGIN_PORT_READ
NRPB(whatever) //need to read
DO_PORT_READ
x = RPB(whatever) //use
END_PORT_READ
*/

#define read_pins(statements) { \
	_rdA = 0, _rdB = 0, _rdC = 0, _rdD = 0, _rdE = 0, _rdF = 0; \
	u08 _port_optimize_in_block = 1; \
	statements \
	if (_rdA) _rdA = PINA; \
	if (_rdB) _rdB = PINB; \
	if (_rdC) _rdC = PINC; \
	if (_rdD) _rdD = PIND; \
	if (_rdE) _rdE = PINE; \
	if (_rdF) _rdF = PINF; \
	_port_optimize_in_block = 0; \
}

#define _read_pin_1(portletter, pin) CAT(_rd, portletter) |= _BV(pin)
#define _read_pin_2(portletter, pin) ((CAT(PIN, portletter) & _BV(pin)) ? 1 : 0)

#define read_pin_absolute(func) _read_pin_2(CAT(func, _PRT), CAT(func, _PIN))
#define read_pin(func) \
	(_port_optimize_in_block \
		? _read_pin_1(CAT(func, _PRT), CAT(func, _PIN)) \
		: (CAT(func, _POL) ? read_pin_absolute(func) : !read_pin_absolute(func)))

#define get_pin_1(portletter, pin) (CAT(_rd, portletter) & _BV(pin)) ? 1 : 0
#define get_pin_absolute(func) (get_pin_1(CAT(func, _PRT), CAT(func, _PIN)))
#define get_pin(func) (CAT(func, _POL) ? (get_pin_absolute(func)) : (!get_pin_absolute(func)))


/* nanosecond delay */
__attribute__((always_inline)) static inline void _delay_ns(double ns)
{
	u08 count;
	double desired_count = (double)(F_CPU) * ns / 3e9;
	if (desired_count < 1.0)
		count = 1;
	else if (desired_count > 255) {
		_delay_us(ns / 1000.0);
		count = (u08)((u32)(desired_count + 0.5) - (u32)(ns / 1000.0) * 1000);
	}
	else
		count = (u08)(desired_count + 0.5);
	_delay_loop_1(count);
}

#endif
