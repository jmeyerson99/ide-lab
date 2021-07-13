/*
Title: uart.c
Purpose: Implement UART routines for serial IO
Name: Jacob Meyerson
Date: 2/5/21
*/

#include "uart.h"

void uart_init() {
	// define variables for baud rate and baud rate fine adjust
	uint16_t ubd, brfa;

	// Enable clock for UART
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; // Enable clock on PORTB
	SIM_SCGC4 |= SIM_SCGC4_UART0_MASK; // Enable clock for UART0
	 
	// Configure the port control register to alternative 3 (which is UART mode for K64)
	PORTB_PCR16 = PORT_PCR_MUX(3); // UART0 RX
	PORTB_PCR17 = PORT_PCR_MUX(3); // UART0 TX

	/* Configure the UART for establishing serial communication*/

	// Disable transmitter and receiver until proper settings are chosen for the UART module
	UART0_C2 &= ~(UART_C2_TE_MASK); // Disable UART0 Transmitter Enable
	UART0_C2 &= ~(UART_C2_RE_MASK); // Disable UART0 Receiver Enable

	// Select default transmission/reception settings for serial communication of UART by clearing the control register 1
	UART0_C1 = 0x00;

	// UART Baud rate is calculated by: baud rate = UART module clock / (16 × (SBR[12:0] + BRFD))
	// 13 bits of SBR are shared by the 8 bits of UART3_BDL and the lower 5 bits of UART3_BDH 
	// BRFD is dependent on BRFA, refer Table 52-234 in K64 reference manual
	// BRFA is defined by the lower 4 bits of control register, UART0_C4 

	// calculate baud rate settings: ubd = UART module clock/16* baud rate
	ubd = (uint16_t)((SYS_CLOCK)/(BAUD_RATE * 16));  

	// clear SBR bits of BDH
	UART0_BDH &= ~(UART_BDH_SBR_MASK);

	// distribute this ubd in BDH and BDL
	UART0_BDH |= (ubd >> 8) & UART_BDH_SBR_MASK; // Put the most significant 8 bits (only 5 actually, but need to shift it by 8) in the BDH register
	UART0_BDL |= ubd & UART_BDL_SBR_MASK;        // Put the least significant 8 bits in the BDL register

	// BRFD = (1/32)*BRFA 
	// make the baud rate closer to the desired value by using BRFA
	brfa = (((SYS_CLOCK * 32)/(BAUD_RATE * 16)) - (ubd * 32));

	// write the value of brfa in UART0_C4
	UART0_C4 |= (brfa & UART_C4_BRFA_MASK);
		
	// Enable transmitter and receiver of UART
	UART0_C2 |= UART_C2_TE_MASK; // Enable UART0 Transmitter Enable
	UART0_C2 |= UART_C2_RE_MASK; // Enable UART0 Receiver Enable
}

uint8_t uart_getchar() {
	/* Wait until there is space for more data in the receiver buffer */
	while ((UART0_S1 & UART_S1_RDRF_MASK) >> (UART_S1_RDRF_SHIFT) == 0) { // wait until the RDRF bit is set
		// wait until there's a character in the receive buffer
	}

	/* Return the 8-bit data from the receiver */
	return UART0_D;
}

void uart_putchar(char ch) {
	/* Wait until transmission of previous bit is complete */
	while ((UART0_S1 & UART_S1_TDRE_MASK) >> (UART_S1_TDRE_SHIFT) == 0) { // wait until the TDRE bit is set
		// wait until there's space in the transmit buffer
	}
	
	/* Send the character */
	UART0_D = ch;
}

void uart_put(char *ptr_str) {
	/* Use putchar to print string */
	while (*ptr_str) {  // loop until the end of the string (null terminator reached)
		uart_putchar(*ptr_str++);   // echo the character
	}
}
