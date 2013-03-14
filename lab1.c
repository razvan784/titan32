#include<avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>

#define F_CPU 16e6
#include <util/delay.h>

#define EN PORTC.2

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))


uint8_t lungime;

void LCMD(unsigned char);
void LDATA(unsigned char);
void write(int) ;
void write_data( char*);

uint16_t duty_cycle_a;
uint16_t duty_cycle_b;
uint16_t duty_cycle_c;
uint16_t theTenBitResults;
int temp;

ISR(TIMER1_OVF_vect) {
    OCR1A = duty_cycle_a;
    OCR1B = duty_cycle_b;
}

ISR(TIMER2_OVF_vect) {
	OCR2 = duty_cycle_c;
}

ISR(ADC_vect) {
	
} 

void counter_setup(void) {
    DDRD |= _BV(PD4) | _BV(PD5) | _BV(PD7);

    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);
	
	TCCR2 = _BV(WGM21) | _BV (WGM20) | _BV(COM21) | _BV(CS21);
	TCNT2 = 0;

    // TOP value
    ICR1 = 255;
	
    OCR1A = 30;
    OCR1B = 30;
	OCR2 = 30;

    // enable interrupt on timer overflow 
   TIMSK = _BV(TOIE1) | _BV(TOIE2);
}


void lcd_setup (void) {
	DDRC=0xFF;	

	_delay_ms(20);					

	LCMD(0X28);
	LCMD(0X28);		
	LCMD(0X01);		
	LCMD(0X0F);		
	LCMD(0X06);		
}


void keypad_setup(void) {
	DDRD |= 0x0F;
	DDRB = 0x00;
	DDRC &= 0x7f;
	PORTC |= 0x80;
	PORTB= 0x07;
}


void adc_setup (void) {
	DDRA = 0;
	ADCSRA |= 1<<ADPS2;
	ADMUX  |= 1<<REFS0;
	ADCSRA |= 1<<ADIE;
	ADCSRA |= 1<<ADEN;
	ADCSRA |= 1<<ADPS2 | 1<<ADPS1;
}


unsigned char get_key() {
	int i=0;

	for(i=0;i<4;i++) {
			
		PORTD = ~(0x01<<i) & 0x0F;
		_delay_ms(30);

		if((PINC & 0x80) != 0x80) {
			return i*4+1;
		}
		if((PINB & 0x07)==6) {
			return i*4+2;
		}
		if((PINB & 0x07)==5) {
			return i*4+3;
		}
		if((PINB & 0x07)==3) {
			return i*4+4;
		}			
	}
	return 255;
}


void write_rbg() {
	LCMD(0X01);
	write_data("R-");write(OCR1B);
	write_data(" G-");write(OCR1A);
	write_data(" B-");write(OCR2);
	
	uint8_t theLowADC = ADCL;

	theTenBitResults = ADCH << 8 | theLowADC;
	
	temp = (theTenBitResults * 4.8 - 2735)/10;
	
	write_data(" T-");
	write(temp);
	LDATA(0xFD);write_data("C");
	
	if((PIND & 0x40) == 0x40)
		write_data(" noapte");
	else
		write_data(" zi");
		
	ADCSRA |= 1<<ADSC;
}



int main() {
	unsigned char key, pas = 0;;
	
	lcd_setup();	
	
	_delay_ms(50);

	keypad_setup();
	
	counter_setup();
	
	adc_setup();

    sei();

	duty_cycle_a = 30;
	duty_cycle_b = 30;
	duty_cycle_c = 30;
	
	ADCSRA |= 1<<ADSC;

	while(1) {
		key = get_key ();
		switch (key) {
			case 1 : {
				duty_cycle_b = 0;
				write_rbg();break;
			}
			case 5 : {
				if (duty_cycle_b > 0)
					duty_cycle_b -= 5;
				write_rbg();break;
			}
			case 9 : {
				if (duty_cycle_b < 255)
					duty_cycle_b += 5;
				write_rbg();break;
			}
			case 13 : {
				duty_cycle_b = 255;
				write_rbg();break;
			}
			
			case 2 : {
				duty_cycle_a = 0;
				write_rbg();break;
			}
			case 6 : {
				if (duty_cycle_a > 0)
					duty_cycle_a -= 5;
				write_rbg();break;
			}
			case 10 : {
				if (duty_cycle_a < 255)
					duty_cycle_a += 5;
				write_rbg();break;
			}
			case 14 : {
				duty_cycle_a = 255;
				write_rbg();break;
			}
			
			case 3 : {
				duty_cycle_c = 0;
				write_rbg();break;
			}
			case 7 : {
				if (duty_cycle_c > 0)
					duty_cycle_c -= 5;
				write_rbg();break;
			}
			case 11 : {
				if (duty_cycle_c < 255)
					duty_cycle_c += 5;
				write_rbg();break;
			}
			case 15 : {
				duty_cycle_c = 255;
				write_rbg();break;
			}
			default: {
				if (pas == 6) {
					pas = 0;
					write_rbg();
				}
				else
					pas++;
				
				break;
			}
		}		
	
	}
	
	return 1;
}

void write(int a) {
	char sir[10];
	sprintf(sir,"%d",a);

	int i;
	for(i = 0; i < strlen(sir); i++)
		LDATA(sir[i]);
	
	
}
 
void write_data( char* a) {
	int i;
	for(i=0;i<strlen(a);i++)
		LDATA(a[i]);

}


void LCMD(unsigned char CM) {
	_delay_ms(2);							
	PORTC=(CM & 0XF0)>>1|0X04;			
    PORTC ^= 0X04;                                
    PORTC=(CM<<3)|0X04;                 
    PORTC ^= 0X04;                                
	PORTC |= 0x80;
	
	if (CM == 0x01) 
		lungime = 0;
}


void LDATA(unsigned char DT) {

	int i;
	
    _delay_ms(2);                            
    PORTC = (DT & 0xF0)>>1|0x05;            
    PORTC ^= 0X04;                                
    PORTC = (DT<<3)|0x05;                
    PORTC ^= 0X04; 
	PORTC |= 0x80;
	lungime ++;
	if(lungime == 16)
		for(i=0;i<24;i++)
			LDATA(' ');
}

