# PongGame

PongGame is a bare metal implmentation of the classic Pong Game. 

## Includes

PongGame utilizes the standard C libaries. 

```C
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
```

Pong Game utilizes two custom libraries. 

```C
#include "lcd.h"
#include "usart.h"
```

### lcd.h 

lcd.h contains a the memory space associated with the buffers for the LCD. lcd.h is responsible for writing geometires to the buffer, sending the buffer to the LCD, and clearing the buffer of the LCD.

### usart.h 

usart.h is a custom library provided by ESE350 TAs responsible for interpreting communication from the ATMega328p to a serial port. 

## main.c 

main.c contains the bulk of the code for PongGame. main.c contains globar variables, interrupt vectors, helper functions, and the main function. 

Global Variables 
```C
#define F_CPU 16000000UL     // clockspeed of atmega328p
#define FREQ 16000000        // clockspeed of atmega328p 
#define BAUD 9600            // baudrate for serial communication 
#define HIGH 1               // logical high 
#define LOW 0                // logical low 
#define BUFFER 1024          // size of buffer array 
#define BLACK 0x000001       
#define PADDLE_WIDTH 2       // pixel width of pong paddles 
#define PADDLE_HEIGHT 9      // pixel height of pong paddles 
#define CIRLCE_RADIUS 3      // pixel radius of pong ball 
#define PADDLE_SPEED 3       // pixel travel speed of player paddles  
#define AI_PADDLE_SPEED 3    // pixel travel speed of ai paddle 
#define WIN_SCORE 9          // score until game over 
char displayChar = 0;        
char String[10] = "";        // String used by USART_putstring to display serial communcation
uint8_t game_start = 1;      // toggle, if a game just started, game_start = 1, else game_start = 0
uint8_t game_state = 0;      // tracks the state of the game. 
uint8_t last_game_state = 0; // tracks the state of the game if user enters the pause screen 
uint8_t buzzer = 0;          // tracks the unique buzzer noises 
uint8_t buzzer_timer = 0;    // duration of buzzer ringing 
uint8_t buzzer_toggle = 0;   // acts as a flag for the buzzer, another buzzer sound cannot be sent until this flag is cleared. 
uint16_t array[6] = { 65535, 65535, 65535, 65535, 65535}; // array for holding paddle and ball locations. from left to right, left paddle - right paddle - ball next - ball current - ball last
uint8_t score[2] = { 0, 0 }; // tracks the score of the left player and right player, respectively 
char score_char[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }; // array for holding char representation of scores 
int dxdy[4] = { 0, 0 };      // tracks the change in x and change in y of the ball movement, respectively 
uint16_t acc[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // arrary for holding the last 10 values returned from the accelerometer for calculating the moving average 
```

Interrupt vectors 
```C
ISR(TIMER1_COMPA_vect);
ISR(TIMER0_COMPA_vect); 
```

Helper Functions 
```C
void adc_init()               // enables flags for reading adc 
uint16_t adc_read(uint8_t ch) // reads the adc value at cahnnel input ch 
void start_screen(void)       // contains the buffer for the start screen 
void pause_screen(void)       // contains the buffer for the pause screen 
void win_screen(void)         // contains the buffer for the win screen 
int in_boundary(uint8_t x, uint8_t y, uint16_t x0y0, uint16_t x1y1) // tests if the input from the touch screen in within a specified boundary, x0y is the top left of the boundary, x1y1 is the bottom right of the boundary
void array_init(uint8_t toggle) // clears the contents of array, if toggle = 0 then clears the contents of score
void arena_init(void)         // writes the buffer for the paddle positions, ball position, and arena to the LCD 
void find_random_trajectory(int x_rand_in, int y_rand_in) // finds a random trajectory for the ball at the beginning of each game. 
void find_next_trajectory(void) // calculates the next ball position and updates the relevant positions in array
void acc_init(void)           // populates the acc array at the start of the program 
uint16_t acc_mean(void)       // calculates the moving mean of the acc array 
```

main function 
The main function handles the transition of the game itself. It begins by initializing all relevant variables and interrupt flags for the game. The while loop handles the game state itself. Given the value of the game_state variable, different actions are performed. Each iteration of the while loop calculates the input of the touchscreen, and if necessary calls the helper functions to move the paddles and ball. 
