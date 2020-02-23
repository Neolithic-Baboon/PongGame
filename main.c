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

uint8_t game_state = 0;
uint8_t last_game_state = 0;

uint16_t array[6] = { 65535, 65535, 65535, 65535 }; // l_padde, r_padde, ball_current, ball_last
char score[2] = { 255, 255 }; // l_score, r_score

#define PADDLE_WIDTH 2
#define PADDLE_HEIGHT 8
#define CIRLCE_RADIUS 3

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

void array_init(void) {
	score[0] = '0';
	score[1] = '0';
	array[0] = 2 + 128*28;
	array[1] = 124 + 128*28;
	array[2] = 63 + 128*31;
	array[3] = 65535;
}

void start_screen(void) {
	game_state = 0;
	array_init();
	clear_buffer(buff);
	drawstring(buff, 36, 1, "Play Pong");
	drawstring(buff, 21, 3, "Player vs Player");
	drawstring(buff, 21, 5, "Player vs Comp");
	drawstring(buff, 21, 7, "Acc Mode");
	fillcircle(buff, 15, 27, 2);
	fillcircle(buff, 15, 43, 2);
	fillcircle(buff, 15, 59, 2);
	drawrect(buff, 19, 23, 98, 9);
	drawrect(buff, 19, 39, 86, 9);
	drawrect(buff, 19, 55, 50, 8);
	write_buffer(buff);
}

void pause_screen(void) {
	game_state = 255; 
	for (uint8_t x = 41; x < 86; x++) {
		for (uint8_t y = 22; y < 49; y++) {
			clearpixel(buff, x, y);
		}
	}
	drawrect(buff, 41, 22, 44, 27);
	drawstring(buff, 45, 3, "PAUSED");
	drawstring(buff, 45, 4, "resume");
	drawstring(buff, 45, 5, "quit");
	drawrect(buff, 43, 32, 40, 7);
	drawrect(buff, 43, 39, 40, 7);
	write_buffer(buff);
}

void arena_init(void) {
	clear_buffer(buff);
	drawrect(buff, 0, 0, 126, 63);
	drawchar(buff, 57, 0, score[0]);
	drawchar(buff, 66, 0, score[1]);
	uint16_t circle_pos = array[2];
	uint16_t x_pos = circle_pos % 128;
	uint16_t y_pos = circle_pos / 128;
	fillcircle(buff, x_pos, y_pos, 3);
	drawline(buff, 63, 0, 63, 55, 1);
	drawrect(buff, 60, 55, 8, 8);
	drawline(buff, 62, 58, 62, 61, 0);
	drawline(buff, 63, 58, 63, 61, 0);
	drawline(buff, 65, 58, 65, 61, 0);
	drawline(buff, 66, 58, 66, 61, 0);
	_delay_ms(10);
	write_buffer(buff);
}

// generates an unsigned byte. High nibble represents next x position. Low nibble represents next y position.
uint8_t find_random_trajectory(void) { 
	return rand() % 0xFF;
}

uint16_t find_next_trajectory(uint16_t current_pos, uint16_t prev_pos) {
	
}

int in_boundary(uint8_t x, uint8_t y, uint16_t x0y0, uint16_t x1y1) {
	uint8_t x0 = x0y0 % 128;
	uint8_t y0 = x0y0 / 128;
	uint8_t x1 = x1y1 % 128;
	uint8_t y1 = x1y1 / 128;
	if (x >= x0 && x <= x1) {
		if (y >= y0 && y <= y1) {
			return 1;
		}
	}
	return 0;
}

int main(void)
{
	//setting up the gpio for backlight
	USART_init();
	adc_init(); 
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
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(12000);
	clear_buffer(buff);	
	start_screen();
	
	//for (uint8_t i = 0; i < 11; i++)
	while (1)
	{
		
		uint16_t x_pos, y_pos, x_screen, y_screen;
		DDRC = 0x02;
		PORTC = 0x01;
		_delay_ms(10);
		if (!(PINC & (1 << PINC0))) {
			
			PORTB |= (1 << PORTB5);
			DDRC = 0x0A;
			PORTC = 0x08;
			_delay_ms(10);
			y_pos = adc_read(0);
			if (y_pos > 856) { y_pos = 865; } 
			else if (y_pos < 330) { y_pos = 330; }
			DDRC = 0x05;
			PORTC = 0x01;
			_delay_ms(10);
			x_pos = adc_read(3);
			if (x_pos < 135) { x_pos = 135; } 
			else if (x_pos > 845) { x_pos = 845; }
			x_screen = (x_pos - 135) * 2/11;
			y_screen = 64 - (y_pos - 330)*4/33;
			
		} else { PORTB &= ~(1 << PORTB5); }
			
		if (game_state == 0) {
			
			if (in_boundary(x_screen, y_screen, 19 + 23 * 128, 117 + 32 * 128)) {
				game_state = 1;
			} else if (in_boundary(x_screen, y_screen, 19 + 39 * 105, 105 + 48 * 128)) {
				game_state = 11;
			} else if (in_boundary(x_screen, y_screen, 19 + 55 * 128, 69 + 63 * 128)) {
				game_state = 21;
			}
			
			
		} else if ((game_state == 1) || (game_state == 11) || (game_state == 21)) {
			
			arena_init();
			if (in_boundary(x_screen, y_screen, 60 + 55 * 128, 68 + 63 * 128)) {
				last_game_state = game_state;
				pause_screen();
			}
			
		} else if (game_state == 255) {
			
			if (in_boundary(x_screen, y_screen, 43 + 32 * 128, 83 + 38 * 128)) {
				game_state = last_game_state;
				arena_init();
			} else if (in_boundary(x_screen, y_screen, 43 + 39 * 128, 83 + 45 * 128)) {
				start_screen();
				x_screen = 0;
				y_screen = 0;
			}	
		}
		
		
	}
}

