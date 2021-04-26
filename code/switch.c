/*
Title: Switch.c
Purpose: On-board push button switch interface
Name: Jacob Meyerson
Date: 1/29/21
*/

#include "switch.h"

void Switch2_Init() {
	// 12.2.12 System Clock Gating Control Register 5
	// Port C is Bit 11 in SCGC5
	// Enable clock for Port C PTC6 button
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; // Enable clock on PORTC (bit 11)
	
	// Configure the Mux for the button
	PORTC_PCR6 = PORT_PCR_MUX(1); // Switch 2
	 
	// Set the push button as an input
	// GPIOC_DDR is the direction control for Port C
	// set it to ZERO at BIT6 for an input
	GPIOC_PDDR |= (0 << 6); // SWITCH is an input
}

unsigned char Switch2_Pressed() {
	// check if switch2 pressed ()
	// if PORT C - BIT6 is ZERO, then the switch is pressed
	// PDIR - Port Data Input Register
	// return a ZERO if NOT Pressed
	// return a 0xFF if Pressed
	if ((GPIOC_PDIR & (1 << 6)) != 0) { // if PORT C BIT 6 is 0 (if switch is not pressed)
		return 0x00;
	}
	
	// then the switch is pressed
	return 0xFF;
}

void Switch3_Init() {
	// 12.2.12 System Clock Gating Control Register 5
	// Port A
	// Enable clock for Port A PTA4 button
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; // Enable clock on PORTA
	
	// Configure the Mux for the button
	PORTA_PCR4 = PORT_PCR_MUX(1); // Switch 3
	 
	// Set the push button as an input
	// GPIOA_DDR is the direction control for Port A
	// set it to ZERO at BIT4 for an input
	GPIOA_PDDR |= (0 << 4); // SWITCH is an input
}

unsigned char Switch3_Pressed() {
	// check if switch3 pressed ()
	// if PORT A - BIT4 is ZERO, then the switch is pressed
	// PDIR - Port Data Input Register
	// return a ZERO if NOT Pressed
	// return a 0xFF if Pressed
	if ((GPIOA_PDIR & (1 << 4)) != 0) { // if PORT A BIT 4 is 0 (if switch is not pressed)
		return 0x00;
	}
	
	// then the switch is pressed
	return 0xFF;
}

