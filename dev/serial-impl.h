/*
 * serial-impl.h
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
#error Do not include this file directly, include serial.h
#endif

#ifdef UART_NUM
/* ATmega128 e.g. has definitions for both unnumbered and numbered register bits */
#undef rxbuf
#undef txbuf
#undef txstart
#undef txstop
#undef rxstart
#undef rxstop
#undef tx_done
#undef rx_ovf
#undef rx_err

#undef UCSRA
#undef UCSRB
#undef UCSRC
#undef UBRR
#undef UBRRL
#undef UBRRH

#undef RXC
#undef TXC
#undef UDRE
#undef FE
#undef DOR
#undef UPE
#undef MPCM

#undef RXCIE
#undef TXCIE
#undef UDRIE
#undef RXEN
#undef TXEN
#undef UCSZ
#undef UCSZ2
#undef RXB8
#undef TXB8

#undef UPM0
#undef USBS
#undef UCSZ0

/* define generic vars, regs, bits */
#define rxbuf   CATX(rxbuf,   UART_NUM)
#define txbuf   CATX(txbuf,   UART_NUM)
#define txstart CATX(txstart, UART_NUM)
#define txstop  CATX(txstop,  UART_NUM)
#define rxstart CATX(rxstart, UART_NUM)
#define rxstop  CATX(rxstop,  UART_NUM)
#define tx_done CATX(tx_done, UART_NUM)
#define rx_ovf  CATX(rx_ovf,  UART_NUM)
#define rx_err  CATX(rx_err,  UART_NUM)

#define UCSRA CATX(UCSR, CATX(UART_NUM, A))
#define UCSRB CATX(UCSR, CATX(UART_NUM, B))
#define UCSRC CATX(UCSR, CATX(UART_NUM, C))
#define UBRRL CATX(UBRR, CATX(UART_NUM, L))
#define UBRRH CATX(UBRR, CATX(UART_NUM, H))
#define UDR   CATX(UDR,  UART_NUM)

#define RXC   CATX(RXC,  UART_NUM)
#define TXC   CATX(TXC,  UART_NUM)
#define UDRE  CATX(UDRE, UART_NUM)
#define FE    CATX(FE,   UART_NUM)
#define DOR   CATX(DOR,  UART_NUM)
#define UPE   CATX(UPE,  UART_NUM)
#define MPCM  CATX(MPCM, UART_NUM)

#define RXCIE CATX(RXCIE, UART_NUM)
#define TXCIE CATX(TXCIE, UART_NUM)
#define UDRIE CATX(UDRIE, UART_NUM)
#define RXEN  CATX(RXEN,  UART_NUM)
#define TXEN  CATX(TXEN,  UART_NUM)
#define UCSZ2 CATX(UCSZ,  CATX(UART_NUM, 2))
#define RXB8  CATX(RXB8,  UART_NUM)
#define TXB8  CATX(TXB8,  UART_NUM)

#define UPM0  CATX(UPM,  CATX(UART_NUM, 0))
#define USBS  CATX(USBS, UART_NUM)
#define UCSZ0 CATX(UCSZ, CATX(UART_NUM, 0))

#undef  BUS_TXEN
#define BUS_TXEN  CATX(BUS_TXEN, UART_NUM)

/* functions */
#define serial_can_rx      CATX(serial_can_rx,        UART_NUM)
#define serial_can_tx      CATX(serial_can_tx,        UART_NUM)
#define serial_has_rx_data CATX(serial_has_rx_data,   UART_NUM)
#define serial_has_tx_data CATX(serial_has_tx_data,   UART_NUM)
#define _serial_putch      CATX(_serial_putch,        UART_NUM)
#define _serial_getch      CATX(_serial_getch,        UART_NUM)
#define _serial_init_div   CATX(_serial_init_div,     UART_NUM)
#define _serial_init       CATX(_serial_init,         UART_NUM)
#define USART_RX_func      CATX(USART_RX_func,        UART_NUM)
#define USART_RX_vect      CATX(USART, CATX(UART_NUM, _RX_vect))
#define USART_TX_vect      CATX(USART, CATX(UART_NUM, _TX_vect))
#define USART_UDRE_vect    CATX(USART, CATX(UART_NUM, _UDRE_vect))

#endif

extern u08 rxbuf[256]; /* byte wraparound makes circular buffer */
extern u08 txbuf[256];
extern volatile u08 txstart;
extern volatile u08 txstop;
extern volatile u08 rxstart;
extern volatile u08 rxstop;
extern u08 rx_ovf;
extern u08 rx_err;

static inline int serial_can_rx() { return rxstop + 1 != rxstart; }
static inline int serial_can_tx() { return txstop + 1 != txstart; }
static inline int serial_has_rx_data() { return rxstart != rxstop; }
static inline int serial_has_tx_data() { return txstart != txstop; }

/* no simultaneous calls allowed */
static inline void _serial_putch(u08 c)
{
	u08 txstop_l;
	txstop_l = txstop;
	while ((u08)(txstop_l + 1) == txstart);
	txbuf[txstop_l] = c;
	txstop = txstop_l + 1;
#if (CATX(BUS_TXEN, _USE))
	port_optimize_declare();
	set_pin(BUS_TXEN);
	// TODO: delay?
#endif
	UCSRB |= _BV(UDRIE); /* calls ISR */
}

/* no simultaneous calls allowed */
static inline u08 _serial_getch()
{
	u08 rxstart_l = rxstart;
	while (rxstart_l == rxstop);
	u08 c = rxbuf[rxstart_l];
	rxstart = rxstart_l + 1;
	return c;
}

static inline void _serial_init_div(u16 baudiv, u16 data_bits, u16 parity_bits, u16 stop_bits)
{
	UBRRH = (u08)(baudiv >> 8);
	UBRRL = (u08)baudiv;

	/* HACK: should fall in place */
#ifdef URSEL
	UCSRC = ((parity_bits & 3) << UPM0) | ((stop_bits & 1) << USBS) | (((data_bits) & 3) << UCSZ0) | _BV(URSEL);
#else
	UCSRC = ((parity_bits & 3) << UPM0) | ((stop_bits & 1) << USBS) | (((data_bits) & 3) << UCSZ0);
#endif

	UCSRB = _BV(RXCIE) | _BV(TXCIE) | _BV(RXEN) | _BV(TXEN) | ((data_bits & 4) ? _BV(UCSZ2) : 0);
}

static inline void _serial_init(u32 baud, u16 data_bits, u16 parity_bits, u16 stop_bits)
{
	u16 baudiv = (u16)(F_CPU / 1600 + ((u16)(baud/100)>>1)) / (u16)(baud/100) - 1;
	_serial_init_div(baudiv, data_bits, parity_bits, stop_bits);
}

#ifdef MAIN /* define ISRs in just one .c file */

u08 rxbuf[256]; /* byte wraparound makes circular buffer */
u08 txbuf[256];
volatile u08 txstart;
volatile u08 txstop;
volatile u08 rxstart;
volatile u08 rxstop;
u08 tx_done;
u08 rx_err;
u08 rx_ovf;

#ifndef SERIAL_USE_DPC
static inline
#endif
void USART_RX_func(int param) /* called from interrupt */
{
	u08 rxstop_l = rxstop;
#ifdef SERIAL_USE_DPC
	while (UCSRA & _BV(RXC)) /* have data */
#endif
	{
		if ((u08)(rxstop_l + 1) != rxstart) { /* have buffer */
			if (UCSRA & (_BV(FE) | _BV(DOR) | _BV(UPE))) {
				rx_err = 1;
				if (UCSRA & _BV(DOR))
					rx_ovf = 1;
			}
			rxbuf[rxstop_l] = UDR;
			rxstop = rxstop_l + 1;
#ifdef SERIAL_USE_DPC
			if (param)
				UCSRB |= _BV(RXCIE);
#endif
		}
		else { /* try again later, but disable interrupt in order not to run again immediately */
#ifdef SERIAL_USE_DPC
			UCSRB &= ~_BV(RXCIE);
			dpc_post(&USART_RX_func, 1);
			return;
#else
		rx_err = 1;
		rx_ovf = 1;
#endif
		}
	}
}

ISR(USART_RX_vect) /* byte receive complete */
{
	USART_RX_func(0);
}

ISR(USART_UDRE_vect) /* data register empty */
{
	u08 txstart_l = txstart;
	if (txstart_l != txstop) {
		UDR = txbuf[txstart_l + 1]; /* feed one more byte, will be triggered again */
		txstart = txstart_l + 1;
	}
	else
		UCSRB &= ~_BV(UDRIE); /* no more bytes, don't trigger again */
}

ISR(USART_TX_vect) /* all bytes transmitted */
{
#if (CATX(BUS_TXEN, _USE))
	port_optimize_declare();
	clr_pin(BUS_TXEN);
#endif
}
#endif /* MAIN */

/* functions */
#undef serial_can_rx
#undef serial_can_tx
#undef serial_has_rx_data
#undef serial_has_tx_data
#undef _serial_putch
#undef _serial_getch
#undef _serial_init_div
#undef _serial_init
#undef USART_RX_func
#undef USART_RX_vect
#undef USART_TX_vect
#undef USART_UDRE_vect
