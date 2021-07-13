/* 
Title: main.c 
Purpose: the main code for lab 1
Name: Jacob Meyerson
Date: 1/29/21
*/

// NOTE: The LEDs and the Switches on the K64 are all active LOW

#include "MK64F12.h"             // Device header

#include "Led.h"
#include "Switch.h"

// Declare function prototypes
void delay(void);
int main(void);

void delay(void) {
	volatile long j = 0;
	for (j=0; j < 2000000; j++)
		;
}

int main(void) {
	// FYI:
	// PCOR Turns Pin to ZERO  (clear)
	// PSOR Turns Pin to ONE   (sets)

	//Initialize any supporting variables
	unsigned int push_button_cnt = 0;
	
	// Initialize buttons and LEDs
	LED_Init();
	Switch2_Init();
	
	for(;;)
	{
		//Turn on an LED configuration when switch2 is pressed
		if(Switch2_Pressed())
		{
			// display yellow when the switch is pressed 
			// Comment this out when performing the LED cycles
			/*
			LED_On('y');
			delay();
			LED_Off();
			*/
		  
			// first time pressed cycle R,G,B - delay() in between colors
			// use LED_On()
			// Comment this out when performing just the yellow LED
			if (push_button_cnt % 3 == 0) {
				// RED
				LED_On('r'); 
				delay(); 
				LED_Off();
				
				// GREEN
				LED_On('g'); 
				delay(); 
				LED_Off();
				
				// BLUE
				LED_On('b'); 
				delay(); 
				LED_Off();
			}
			// second time pressed cycle C,M,Y- delay() in between colors
			// use LED_On()
			if (push_button_cnt % 3 == 1) {
				// CYAN
				LED_On('c'); 
				delay(); 
				LED_Off();
				
				// MAGENTA
				LED_On('m'); 
				delay(); 
				LED_Off();
				
				// YELLOW
				LED_On('y'); 
				delay(); 
				LED_Off();
			}
			// third time pressed W		- delay() in between colors
			// use LED_On()
			if (push_button_cnt % 3 == 2) {
				// WHITE
				LED_On('w'); 
				delay(); 
				LED_Off();
			}
			push_button_cnt++; // increment push button counter
		}
		// Turn off LED when no switch is pressed
		else
		{
			LED_Off();
		}
	}
}
