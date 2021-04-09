/*
Title: isr.c
Description: ISR handler
Authors: Jacob Meyerson
Date: 2/26/21
*/

#include "isr.h"

// variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
// global variables are static so they can remain the same across multiple functions
static int switch_2_cnt = 0;
static bool switch_2_pressed = FALSE;
static PDB_State pdb_state = DISABLED;
/*
void PDB0_IRQHandler(void){ // For PDB timer
	// Clear the interrupt in register PDB0_SC
	PDB0_SC &= ~(PDB_SC_PDBIF_MASK);
	
	// toggle the output state for LED
	GPIOB_PTOR |= (1 << 22);
}
	
void FTM0_IRQHandler(void){ // For FTM timer
	// Clear the interrupt in regster FTM0_SC
	FTM0_SC &= ~(FTM_SC_TOF_MASK);
	
	// if switch2 has been pressed, increment global counter
	if (switch_2_pressed) {
		switch_2_cnt++;
	}
}
	
void PORTA_IRQHandler(void){ // For switch 3
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
		PDB0_SC |= PDB_SC_PDBEN_MASK;  // Enable the PDB
		PDB0_SC |= PDB_SC_SWTRIG_MASK; // Start the SW Triggering again to start the timer
	}
}
	
void PORTC_IRQHandler(void){ // For switch 2
	// Clear the interrupt
	PORTC_ISFR &= ~(PORT_ISFR_ISF(6));
	
	// Print the time the switch has been held down to the terminal screen
	
	// If the switch is pressed, then set the global variable
	if (Switch2_Pressed()) {
		switch_2_pressed = TRUE;
	}
	// Else, the switch isn't pressed anymore so display how long it has been pressed
	else {
		switch_2_pressed = FALSE; 
		UART0_Put("The switch was pressed for ");
		UART0_PutNumU(switch_2_cnt); // the cnt is the amount of time (in ms) it's been pressed, since it counts on the interrupt every ms from the FTM
		UART0_Put(" milliseconds.\r\n");
		switch_2_cnt = 0; // reset the counter
	}
} */
