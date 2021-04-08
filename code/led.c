/*
Title: led.c
Purpose: On-board LED interface
Name: Jacob Meyerson
Date: 1/29/21
*/

#include "led.h"

void LED_Off(void) {
	// set the bits to ONE to turn off LEDs
	// use PSOR to set a bit
	GPIOB_PSOR = (1 << 22) | (1 << 21); // RED and BLUE
	GPIOE_PSOR = (1 << 26);             // GREEN
}

void LED_Init(void) {
	// Enable clocks on Ports B and E for LED timing
	// We use PortB for RED and BLUE LED
	// We use PortE for GREEN LED
	// 12.2.12 System Clock Gating Control Register 5
	// Port B is Bit 10
	// Port E is Bit 13
				   // 0x0400 (Bit 10)                 0x2000 (Bit 13)
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; // Enable clock on PORTB (bit 10)
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Enable clock on PORTE (bit 13)
	 
	// Configure the Signal Multiplexer for GPIO
 	// Pin Control Register n  
	PORTB_PCR22 = PORT_PCR_MUX(1); // RED
	PORTB_PCR21 = PORT_PCR_MUX(1); // BLUE
	PORTE_PCR26 = PORT_PCR_MUX(1); // GREEN
	
	// Switch the GPIO pins to output mode
	// GPIOB_DDR is the direction control for Port B // should this be GPIOB_PDDR
	// GPIOE_DDR is the direction control for Port E
	// set it to ONE at BIT21, 22 on Port B for an output
	// set it to ONE at BIT26 on Port E for an output	 
	GPIOB_PDDR |= (1 << 22) | (1 << 21); // RED and BLUE
	GPIOE_PDDR |= (1 << 26);             // GREEN

	// Turn off the LEDs
	LED_Off();
}

void Delay(void) {
	volatile long j = 0;
	for (j=0; j < 2000000; j++)
		;
}

void Flash_LED(unsigned char color) {
	LED_On(color);
	Delay();
	LED_Off();
}

void LED_On (unsigned char color) {
	// set the appropriate color
	// you need to drive the appropriate pin OFF to turn on the LED
	
	// Handle the appropriate color
	switch(color) {
		case 'y':
			// Display yellow (red and green)
			GPIOB_PCOR = (1 << 22); // RED
			GPIOE_PCOR = (1 << 26); // GREEN
			break;
		case 'r':
			// Display red
			GPIOB_PCOR = (1 << 22); // RED
			break;
		case 'g':
			// Display green
			GPIOE_PCOR = (1 << 26); // GREEN
			break;
		case 'b':
			// Display blue
			GPIOB_PCOR = (1 << 21); // RED
			break;
		case 'c':
			// Display cyan (blue and green)
			GPIOB_PCOR = (1 << 21); // BLUE
			GPIOE_PCOR = (1 << 26); // GREEN
			break;
		case 'm':
			// Display magenta (blue and red)
			GPIOB_PCOR = (1 << 21); // BLUE
			GPIOB_PCOR = (1 << 22); // RED
			break;
		case 'w':
			// Display white (blue, green and red)
			GPIOB_PCOR = (1 << 21); // BLUE
			GPIOB_PCOR = (1 << 22); // RED
		  GPIOE_PCOR = (1 << 26); // GREEN
			break;
	}
}
