#include <avr/io.h>
#include <util/delay.h>

#define TEST1_PRT A
#define TEST1_PIN 0
#define TEST1_POL 1

#define LED_PRT B
#define LED_PIN 0
#define LED_POL 1

#define MAIN
#include "avrutil.h"
#include "dev/serial.h"

int main(void)
{
	port_optimize_declare();

	write_pins_dir(
		set_pin_output(LED);
		set_pin(TEST1);
	);

	//serial_init(0, 115200, SERIAL_BITS_8, SERIAL_PARITY_NONE, SERIAL_STOP_BITS_1);
	//serial_init(1, 115200, SERIAL_BITS_8, SERIAL_PARITY_NONE, SERIAL_STOP_BITS_1);

    while (1) {
		if (read_pin(TEST1))
			set_pin(LED);
		else
			clr_pin(LED);
		/*u08 x;
    	x = serial_getch(0);
    	serial_putch(0, x);
    	x = serial_getch(1);
    	serial_putch(1, x);*/
    }
}
