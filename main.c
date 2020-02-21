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
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(10000);
	clear_buffer(buff);
	
	while (1)
	{
		drawrect(buff, 0, 0, 10, 10);
		drawcircle(buff, 42, 32, 10);
		fillrect(buff, 117, 0, 10, 10);
		fillcircle(buff, 84, 32, 10);
		write_buffer(buff);
		_delay_ms(50000);
		clear_buffer(buff);
		setpixel(buff, 42, 42);
		setpixel(buff, 43, 42);
		setpixel(buff, 44, 42);
		setpixel(buff, 45, 41);
		setpixel(buff, 46, 41);
		setpixel(buff, 47, 40);
		setpixel(buff, 48, 39);
		write_buffer(buff);
		_delay_ms(50000);
		clear_buffer(buff);
	}
}

