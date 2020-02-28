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



uint8_t game_start = 1;

uint8_t game_state = 0;

uint8_t last_game_state = 0;

uint8_t buzzer = 0;

uint8_t buzzer_toggle = 0;



uint16_t array[6] = { 65535, 65535, 65535, 65535, 65535}; // l_padde, r_padde, ball_next, ball_current, ball_last

uint8_t score[2] = { 0, 0 }; // l_score, r_score

char score_char[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

int dxdy[4] = { 0, 0 }; // dx, dy, max dx

uint16_t acc[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define PADDLE_WIDTH 2

#define PADDLE_HEIGHT 9

#define CIRLCE_RADIUS 3

#define PADDLE_SPEED 3

#define AI_PADDLE_SPEED 3

#define WIN_SCORE 9



ISR(TIMER1_COMPA_vect) {

	PORTB ^= (1 << PORTB2);

}

uint8_t buzzer_timer = 0;

ISR(TIMER0_COMPA_vect) {
	
	PORTB ^= (1 << PORTB3); 
	buzzer_timer++; 
	
	if (buzzer_timer >= 20) {
		
		buzzer = 0; 
		buzzer_toggle = 0;
		buzzer_timer = 0;
		TIMSK0 &= ~(1 << OCIE0A); 
		
	}
	
}



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



void array_init(uint8_t toggle) { // toggle set to 1 will reset scores

	if (toggle == 1) {

		score[0] = 0;

		score[1] = 0;

	}

	array[0] = 2 + 128*28;

	array[1] = 123 + 128*28;

	array[2] = 65535;

	array[3] = 63 + 128*31;

	array[4] = 65535;

	for (uint8_t i = 0; i < 2; i++) {

		dxdy[i] = 0;

	}

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



void start_screen(void) {

	game_state = 0;

	array_init(1);

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



void win_screen(void) {

	TIMSK0 &= ~(1 << OCIE0A);
	game_state = 31;

	for (uint8_t x = 35; x < 91; x++) {

		for (uint8_t y = 14; y < 51; y++) {

			clearpixel(buff, x, y);

		}

	}

	

	if (score[0] >= 10) {

		drawstring(buff, 79, 3, "L");

		} else {

		drawstring(buff, 79, 3, "R");

	}

	drawstring(buff, 40, 3, "Player");

	drawstring(buff, 39, 2, "Congrats");

	drawstring(buff, 52, 4, "W");

	drawstring(buff, 58, 4, "I");

	drawstring(buff, 64, 4, "N");

	drawstring(buff, 70, 4, "S");

	drawstring(buff, 51, 5, "quit");

	drawrect(buff, 35, 14, 55, 36);

	drawrect(buff, 49, 40, 28, 8);

	

	_delay_ms(100);

	write_buffer(buff);

	_delay_ms(100);

}



void arena_init(void) {

	clear_buffer(buff);

	drawrect(buff, 0, 0, 126, 63);

	drawchar(buff, 57, 0, score_char[score[0]]);

	drawchar(buff, 66, 0, score_char[score[1]]);

	//sprintf(String, "score[0]: %u char[0]: %c score[1]: %u char[1]: %c \n", score[0], score_char[score[0]], score[1], score_char[score[1]]);

	//USART_putstring(String);

	uint16_t circle_pos = array[3];

	uint16_t x_pos = circle_pos % 128;

	uint16_t y_pos = circle_pos / 128;

	fillcircle(buff, x_pos, y_pos, 3);

	drawline(buff, 63, 0, 63, 55, 1);

	drawrect(buff, 60, 55, 8, 8);

	drawline(buff, 62, 58, 62, 61, 0);

	drawline(buff, 63, 58, 63, 61, 0);

	drawline(buff, 65, 58, 65, 61, 0);

	drawline(buff, 66, 58, 66, 61, 0);

	fillrect(buff, array[0] % 128, array[0] / 128, PADDLE_WIDTH, PADDLE_HEIGHT);

	fillrect(buff, array[1] % 128, array[1] / 128, PADDLE_WIDTH, PADDLE_HEIGHT);

	_delay_ms(10);

	write_buffer(buff);

}



// generates an unsigned byte. High nibble represents next x position. Low nibble represents next y position.

void find_random_trajectory(int x_rand_in, int y_rand_in) {

	TIMSK0 |= (1 << OCIE0A);

	int next_move_x, next_move_y;

	int x_rand = abs(x_rand_in);

	int y_rand = abs(y_rand_in);

	if (x_rand <= 3) { next_move_x = (x_rand % 4) - 4; } else { next_move_x = (x_rand % 4) - 3; }

	if (y_rand <= 3) { next_move_y = (y_rand % 4) - 4; } else { next_move_y = (y_rand % 4) - 3; }

	if (next_move_x == 0) {

		next_move_x = 1;

	}

	if (next_move_y == 0) {

		next_move_y = 2;

	}

	array[2] = array[3] + (next_move_x + next_move_y * 128);

	dxdy[0] = (array[2] % 128) - (array[3] % 128);

	dxdy[1] = (array[2] / 128) - (array[3] / 128);

	dxdy[2] = dxdy[0];

	

}



void find_next_trajectory(void) {

	

	_delay_ms(700);

	array[4] = array[3];

	array[3] = array[2];

	

	long int y_holder = array[2] / 128;

	long int y_neg_test = y_holder - 3 + dxdy[1];

	long int y_pos_test = y_holder + 3 + dxdy[1];

	

	if (y_neg_test < 1 || y_pos_test > 62) {

		buzzer = 3;

		dxdy[1] = -dxdy[1];

		

	}

	

	long int x_holder = array[2] % 128;

	long int x_neg_test = x_holder - 3 + dxdy[0];

	long int x_pos_test = x_holder + 3 + dxdy[0];

	

	if (x_neg_test < 4) {

		

		uint8_t l_paddle_top = array[0] / 128;

		uint8_t l_paddle_bot = l_paddle_top + PADDLE_HEIGHT;

		uint16_t temp = array[3] / 128;

		array[2] = 5 + temp * 128 + dxdy[1] * 128;

		dxdy[0] = -dxdy[0];

		

		if (temp >= l_paddle_top && temp <= l_paddle_bot) {
			
			buzzer = 1;

			int bounce_dy = temp - l_paddle_top;

			if (bounce_dy == 4) {

				dxdy[0] == dxdy[0] + 1;

				} else if (bounce_dy <= 3) {

				dxdy[1] = bounce_dy - 4;

				} else if (bounce_dy <= 8) {

				dxdy[1] = bounce_dy - 4;

			}

			

			} else {

			buzzer = 2;

			game_start = 1;

			array_init(0);

			score[1] = score[1] + 1;

			if (score[1] >= WIN_SCORE) { win_screen(); };

			

		}

		

		} else if (x_pos_test > 123) {

		

		uint8_t r_paddle_top = array[1] / 128;

		uint8_t r_paddle_bot = r_paddle_top + PADDLE_HEIGHT;

		uint16_t temp = array[3] / 128;

		array[2] = 121 + temp * 128 + dxdy[1] * 128;

		dxdy[0] = -dxdy[0];

		

		if (temp >= r_paddle_top && temp <= r_paddle_bot) {

			

			int bounce_dy = temp - r_paddle_top;

			buzzer = 1;

			if (bounce_dy == 4) {

				dxdy[0] == dxdy[0] - 1;

				} else if (bounce_dy <= 3) {

				dxdy[1] = bounce_dy - 4;

				} else if (bounce_dy <= 8) {

				dxdy[1] = bounce_dy - 4;

			}

			

			} else {

			buzzer = 2;

			game_start = 1;

			array_init(0);

			score[0] = score[0] + 1;

			if (score[0] >= WIN_SCORE) { win_screen(); };

			

		}

		

		} else {

		

		array[2] = array[3] + dxdy[0] + dxdy[1] * 128;

		

	}

	

}

void acc_init(void) {
	
	for (uint8_t i = 0; i < 10; i++) {
		acc[i] = adc_read(4);
	}
	
}

uint16_t acc_mean(void) {
	
	uint16_t temp = adc_read(4);
	uint16_t sum = 0; 
	for (uint8_t i = 9; i > 0; i--) {
		acc[i] = acc[i - 1];
		sum = sum + acc[i];
	}
	acc[0] = temp;
	sum = sum + temp;
	
	return sum;
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
	
	DDRB |= (1 << PORTB3);

	DDRC = 0;

	PORTC = 0;

	
	TCCR0A = 0;
	
	TCCR0B = 0;
	
	TCCR0A |= (1 >> WGM01);
	
	TCCR0B |= (1 << CS02) | (1 << CS00);
	

	TCCR1B = 0;

	TCCR1B |= (1 << WGM12) | (1 << CS10) | (1 << CS12);

	OCR1A = 0x0FFF;

	sei();

	TIFR1 |= (1 << OCF1A);

	acc_init();
	
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

		

		if (game_state != 31) {

			TIMSK1 &= ~(1 << OCIE1A);

			PORTB &= !((1 << PORTB0) | (1 << PORTB2));

		}
		
		if (buzzer_toggle == 0) {
		
			if (buzzer != 0) {
				
				buzzer_toggle = 1; 
				
				TIMSK0 |= (1 << OCIE0A);
				
				TIFR0 |= (1 << OCF0A);
				
				if (buzzer = 1) {
					OCR0A = 3;
					} else if (buzzer = 2) {
					OCR0A = 6;
					} else if (buzzer = 3) {
					OCR0A = 9;
					} else {
					TIMSK0 &= ~(1 << OCIE0A);
				}
				
			}
			
		}

		

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

			

			int x_rand = rand() % 8;

			int y_rand = rand() % 8;

			

			if (game_state == 0) {

				if (in_boundary(x_screen, y_screen, 19 + 23 * 128, 117 + 32 * 128)) {

					game_state = 1;

					find_random_trajectory(x_rand, y_rand);

					} else if (in_boundary(x_screen, y_screen, 19 + 39 * 105, 105 + 48 * 128)) {

					game_state = 11;

					find_random_trajectory(x_rand, y_rand);

					} else if (in_boundary(x_screen, y_screen, 19 + 55 * 128, 69 + 63 * 128)) {

					game_state = 21;

					find_random_trajectory(x_rand, y_rand);

				}
				
				} else if ((game_state == 1) || (game_state == 11) || (game_state == 21)) {

				

				arena_init();

				if (game_start == 1) {

					find_random_trajectory(x_rand, y_rand);

					game_start = 0;

					} else {

					find_next_trajectory();

				}
				
				if (game_state == 21) {
					
					uint16_t mean = acc_mean();
					
					if (mean < 2620) {
						
						mean = 2620;
						
						} else if (mean > 3940) {
						
						mean = 3940;
						
					}
					
					mean = (mean - 2620) / 21;
					
					if (mean < 6) {
						
						mean = 6;
						
						} else if (mean > 57) {
						
						mean = 57;
						
					}
					
					if (mean < array[0] / 128) {
						
						array[0] = array[0] - PADDLE_SPEED * 128;
						
						} else if (mean > array[0] / 128) {
						
						array[0] = array[0] + PADDLE_SPEED * 128;
						
					}
					
				}				

				if (in_boundary(x_screen, y_screen, 60 + 55 * 128, 68 + 63 * 128)) {

					last_game_state = game_state;

					pause_screen();

				}

				

				if ((game_state == 1) || (game_state == 11)) {

					

					uint16_t player_y;

					if (y_screen < 5) {

						player_y = 5;

						} else if ( y_screen > 58) {

						player_y = 58;

						} else {

						player_y = y_screen;

					}
					
					if (game_state == 11 || game_state == 21) {

						if (array[3] / 128 - AI_PADDLE_SPEED < array[1] / 128 + 5) {

							if (!(array[1] / 128 <= AI_PADDLE_SPEED)) {

								array[1] = array[1] - AI_PADDLE_SPEED * 128;

							}

							} else if (array[3] / 128 + AI_PADDLE_SPEED > array[1] / 128 + 5) {

							if (!(array[1] / 128 >= 53 + AI_PADDLE_SPEED)) {

								array[1] = array[1] + AI_PADDLE_SPEED * 128;

							}

						}

					}

					

					if (in_boundary(x_screen, y_screen, 0, 40 + 63 * 128)) {

						if (player_y - PADDLE_SPEED < array[0] / 128 + 5) {

							if (!(array[0] / 128 <= 1)) {

								array[0] = array[0] - PADDLE_SPEED * 128;

							}

							} else if (player_y + PADDLE_SPEED > array[0] / 128 + 5) {

							if (!(array[0] / 128 >= 55)) {

								array[0] = array[0] + PADDLE_SPEED * 128;

							}

						}

						x_screen = 45;

						} else if (game_state == 1) {

						if (in_boundary(x_screen, y_screen, 87, 127 + 63 * 128)) {

							if (player_y < array[1] / 128 + 5) {

								if (!(array[1] / 128 <= PADDLE_SPEED)) {

									array[1] = array[1] - PADDLE_SPEED * 128;

								}

								} else if (player_y > array[1] / 128 + 5) {

								if (!(array[1] / 128 >= 53 + PADDLE_SPEED)) {

									array[1] = array[1] + PADDLE_SPEED * 128;

								}

							}

						}

						x_screen = 45;

					}

				}

				

				} else if (game_state == 255) {

				

				if (in_boundary(x_screen, y_screen, 43 + 32 * 128, 83 + 38 * 128)) {

					game_state = last_game_state;

					arena_init();

					} else if (in_boundary(x_screen, y_screen, 43 + 39 * 128, 83 + 50 * 128)) {

					start_screen();

					_delay_ms(1000);

					x_screen = 0;

					y_screen = 0;

				}

				

				} else if (game_state == 31) {

				

				TIMSK1 |= (1 << OCIE1A);

				

				if (in_boundary(x_screen, y_screen, 49 + 40 * 128, 77 + 48 * 128)) {

					start_screen();

					_delay_ms(1000);

					x_screen = 0;

					y_screen = 0;

				}

				

			}

		}

		_delay_ms(100);

	}