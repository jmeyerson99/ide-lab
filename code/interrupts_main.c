/*
Title: main.c
Description: Main loop code for timers and interrupts lab
Authors: Jacob Meyerson
Date: 2/26/21
*/

#include "MK64F12.h"
#include "uart.h"
#include "isr.h"
#include "switch.h"
#include <stdio.h>

/* From clock setup 0 in system_MK64f12.c */
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */

void PDB_Init(void);
void GPIO_Init(void);
void FTM_Init(void);
void Interrupts_Init(void);

int main(void){
	// initializations
	PDB_Init();
	GPIO_Init();
	FTM_Init();
	UART0_Init();
	Interrupts_Init();

	for(;;){
		// To infinity and beyond
	}
}

void PDB_Init(void){
	// Enable clock for PDB module
	SIM_SCGC6 |= SIM_SCGC6_PDB_MASK; // Enable clock on PDB

	// Set continuous mode, prescaler of 128, multiplication factor of 20,
	// software triggering, and PDB enabled
	PDB0_SC |= PDB_SC_CONT_MASK;    // continuous mode
	PDB0_SC |= PDB_SC_PRESCALER(7); // same as (111 >> (PDB_SC_PRESCALER_SHIFT)) & (PDB_SC_PRESCALER_MASK)
	PDB0_SC |= PDB_SC_MULT(2);      // 10 binary is factor of 20
	PDB0_SC |= PDB_SC_TRGSEL(15);   // set the PDB for software triggering
	PDB0_SC |= PDB_SC_PDBEN_MASK;   // bit 7

	// Set the mod field to get a 1 second period.
	// There is a division by 2 to make the LED blinking period 1 second.
	// This translates to two mod counts in one second (one for on, one for off)
	PDB0_MOD = PDB_MOD_MOD(DEFAULT_SYSTEM_CLOCK / (128*20));

	// Configure the Interrupt Delay register.
	PDB0_IDLY = 10;

	// Enable the interrupt mask.
	PDB0_SC |= PDB_SC_PDBIE_MASK;   // bit 5

	// Enable LDOK to have PDB0_SC register changes loaded.
	PDB0_SC |= PDB_SC_LDOK_MASK;
	
	//PDB0_SC |= PDB_SC_SWTRIG_MASK;  // Turn on the timer
}

void FTM_Init(void){
	// Enable clock for FTM module (use FTM0)
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

	// turn off FTM Mode to write protection;
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;

	// divide the input clock down by 128,
	FTM0_SC |= FTM_SC_PS(7); // 111 = divide by 128

	// reset the counter to zero
	FTM0_CNT &= ~(FTM_CNT_COUNT_MASK);

	// Set the overflow rate
	// (Sysclock/128)- clock after prescaler
	// (Sysclock/128)/1000- slow down by a factor of 1000 to go from
	// Mhz to Khz, then 1/KHz = msec
	// Every 1msec, the FTM counter will set the overflow flag (TOF) and
	FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// Select the System Clock
	FTM0_SC |= FTM_SC_CLKS(1); // 1 = system clock

	// Enable the interrupt mask. Timer Overflow Interrupt Enable
	FTM0_SC |= FTM_SC_TOIE_MASK;
}

void GPIO_Init(void){
	// initialize push buttons and LEDs
	Switch2_Init();
	Switch3_Init();
	LED_Init();

	// interrupt configuration for SW3(Rising Edge) and SW2 (Either)
	PORTA_PCR4 |= PORT_PCR_IRQC(9); // 1001 = rising edge
	PORTC_PCR6 |= PORT_PCR_IRQC(11); // 1011 = either edge
}

void Interrupts_Init(void){
	/* Can find these in MK64F12.h */
	// Enable NVIC for PORTA, PORTC, PDB0, FTM0
	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTC_IRQn);
	NVIC_EnableIRQ(PDB0_IRQn);
	NVIC_EnableIRQ(FTM0_IRQn);
}
