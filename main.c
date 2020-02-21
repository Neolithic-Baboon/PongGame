#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd.h"
#include "usart.h"

#define F_CPU 16000000UL
#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001

char displayChar = 0;

char String[10] = "";

void adc_init() {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t ch) {
	ch &= 0b00000111;
	ADMUX = (ADMUX & 0xF8) | ch;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	return(ADC);
}

int main(void)
{
	//setting up the gpio for backlight
	USART_init();
		
	DDRD |= 0x80;
	PORTD &= ~0x80;
	PORTD |= 0x00;
	
	DDRB |= 0x05;
	PORTB &= ~0x05;
	PORTB |= 0x00;
	
	DDRB |= (1 << PORTB5);
	PORTB &= ~(1 << PORTB5);
	
	DDRC = 0;
	PORTC = 0;
	DDRC |= (1 << PORTC1);
	PORTC &= ~(1 << PORTC1);
	PORTC |= (1 << PORTB2);
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(10000);
	clear_buffer(buff);
	
	adc_init();
	
	while (1)
	{
		
		uint16_t x_pos, y_pos; 
		if (!(PINC & (1 << PINC0))) {
			PORTB |= (1 << PORTB5);
			DDRC |= (1 << PORTC3);
			PORTC |= (1 << PORTC3);
			PORTC &= ~(1 << PORTC0);
			y_pos = adc_read(0);
			DDRC &= ~(1 << PORTC1);
			DDRC &= ~(1 << PORTC3);
			PORTC &= ~(1 << PORTC3);
			DDRC |= (1 << PORTC2);
			PORTC &= ~(1 << PORTC2);
			DDRC |= (1 << PORTC0);
			PORTC |= (1 << PORTC0);
			x_pos = adc_read(3);
			DDRC = 0;
			PORTC = 0;
			DDRC |= (1 << PORTC1);
			PORTC &= ~(1 << PORTC1);
			PORTC |= (1 << PORTB2);
			sprintf(String, "x_pos: %06u y_pos: %06u \n", x_pos, y_pos);
			USART_putstring(String);
			
		} else {
			PORTB &= ~(1 << PORTB5);
		}
		drawstring(buff, 0, 0, "Hello.");
		write_buffer(buff);
		_delay_ms(500);
		clear_buffer(buff);
	}
}

