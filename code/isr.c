/*
Title: isr.c
Description: ISR handler
Authors: Jacob Meyerson
Date: 2/26/21
*/

#include "isr.h"

// variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
// global variables are static so they can remain the same across multiple functions
static unsigned int switch_2_cnt = 0;
static bool switch_2_pressed = FALSE;
static PDB_State pdb_state = DISABLED;
static LED_State led_state = OFF;

void PDB0_IRQHandler(void){ // For PDB timer
	UART0_Put("PDB ISR \n\r");// DEBUG
	// Clear the interrupt in register PDB0_SC
	PDB0_SC &= ~(PDB_SC_PDBIF_MASK);
	
	// toggle the output state for LED
	if (OFF == led_state) {
		led_state = ON;
		LED_On('r');
	}
	else {
		led_state = OFF;
		LED_Off();
	}
}
	
void FTM0_IRQHandler(void){ // For FTM timer
	// UART0_Put("FTM ISR \n\r");// DEBUG - this triggers every 1 msec as expected
	// Clear the interrupt in regster FTM0_SC
	FTM0_SC &= ~(FTM_SC_TOF_MASK);
	
	// if switch2 has been pressed, increment global counter
	if (switch_2_pressed) {
		switch_2_cnt++;
	}
}
	
void PORTA_IRQHandler(void){ // For switch 3
	// UART0_Put("Switch 3 pressed \r\n"); // DEBUG
	// Clear the interrupt
	PORTA_ISFR &= ~(PORT_ISFR_ISF(4));
	
	if (ENABLED == pdb_state) {
		// Disable the timer
		pdb_state = DISABLED; 
		PDB0_SC &= ~PDB_SC_PDBEN_MASK; // Disable the PDB
	}
	else {
		// Enable the timer and start it with a trigger
		pdb_state = ENABLED;
		PDB0_SC |= PDB_SC_PDBEN_MASK; // Enable the PDB
	}
}
	
void PORTC_IRQHandler(void){ // For switch 2
	// NOTE: fix. First interrupt starts timer, second interrupt stops timer and prints
	
	// Clear the interrupt
	PORTC_ISFR &= ~(PORT_ISFR_ISF(6));
	
	// Print the time the switch has been held down to the terminal screen
	while (Switch2_Pressed()) {
		switch_2_pressed = TRUE;
	}
	switch_2_pressed = FALSE;
	uint16_t count = FTM0_CNT & FTM_CNT_COUNT_MASK; // TODO - there's no way this is just time in seconds. What units is it?
	uint16_t time = count / switch_2_cnt; // TODO - check this 
	UART0_Put("The switch was pressed for ");
	UART0_PutNumU(time);
	UART0_Put(" seconds.");
}
