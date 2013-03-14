/*
 * serial.h
 *
 * Copyright (C) 2010 Razvan Tataroiu, razvan784@gmail.com .
 *
 * Buffered, interrupt-driven serial driver, UART-number-independent
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
#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "avrutil.h"

#define SER_TIMEOUT 11520 //2s
#define TURNAROUND_DELAY_US 10

#define SERIAL_BITS_9      7
#define SERIAL_BITS_8      3
#define SERIAL_BITS_7      2
#define SERIAL_BITS_6      1
#define SERIAL_BITS_5      0

#define SERIAL_PARITY_NONE 0
#define SERIAL_PARITY_EVEN 2
#define SERIAL_PARITY_ODD  3

#define SERIAL_STOP_BITS_1 0
#define SERIAL_STOP_BITS_2 1

#ifdef UCSR0A    // multiple UARTs
#define UART_NUM 0
#include "dev/serial-impl.h"
#undef UART_NUM
#else            // single UART
#include "dev/serial-impl.h"
#endif

#ifdef UCSR1A
#define UART_NUM 1
#include "dev/serial-impl.h"
#undef UART_NUM
#endif

#ifdef UCSR2A
#define UART_NUM 2
#include "dev/serial-impl.h"
#undef UART_NUM
#endif

#ifdef UCSR3A
#define UART_NUM 3
#include "dev/serial-impl.h"
#undef UART_NUM
#endif

#ifdef UCSR0A    // multiple UARTs
#define serial_init(n, a, b, c, d) CATX(_serial_init, n) (a, b, c, d)
#define serial_init_div(n, a, b, c, d) CATX(_serial_init_div, n) (a, b, c, d)
#define serial_getch(n) CATX(_serial_getch, n) ()
#define serial_putch(n, x) CATX(_serial_putch, n) (x)
#else
#define serial_init(n, a, b, c, d) _serial_init(a, b, c, d)
#define serial_init_div(n, a, b, c, d) _serial_init_div(a, b, c, d)
#define serial_getch(n) _serial_getch()
#define serial_putch(n, x) _serial_putch(x)
#endif

#endif
