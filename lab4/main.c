/*
Title: main.c
Description: Main loop code for Bluetooth lab
Authors: Jacob Meyerson
Date: 2/19/21
*/

#include "MK64F12.h"
#include "uart.h"         // Main serial funcitonality
#include "Led.h"          // OPTIONAL: Could be useful to display info/warnings
#include <stdio.h>        // General funcitonality
#include <string.h>       // Useful for string operations and memset

int main()
{
	// Initialize LEDs (optional)
	LED_Init();
	
	// Initialize UART0 and UART3
	uart0_init();
	uart3_init();
	
	//#define LED_CODE /* uncomment this to enable the LED code and comment out chatroom */
	
#ifdef LED_CODE
	/* LED Code*/
	char control = uart3_getchar();
	while (1) {
		switch(control) {
			case '0':
				uart0_put("LED OFF\n\r");
				uart3_put("LED OFF\n\r");
				LED_Off();
				break;
			case '1':
				LED_Off();
				uart0_put("RED\n\r");
				uart3_put("RED\n\r");
				LED_On('r');
				break;
			case '2':
				LED_Off();
				uart0_put("BLUE\n\r");
				uart3_put("BLUE\n\r");
				LED_On('b');
				break;
			case '3':
				LED_Off();
				uart0_put("GREEN\n\r");
				uart3_put("GREEN\n\r");
				LED_On('g');
				break;
			default:
				uart0_put("Invalid input\n\r");
				uart3_put("Invalid input\n\r");
		}
		control = uart3_getchar();
	}
}
#else

	// Display startup message
	uart0_put("Chatroom has booted up\r\n> ");
	uart3_put("Chatroom has booted up\r\n> ");
	
	// Declare and reset buffers and print prompts
	char PC_buffer[CHAR_COUNT];
	char *PC_buffer_ptr = &PC_buffer[0];
	char phone_buffer[CHAR_COUNT];
	char *phone_buffer_ptr = &phone_buffer[0];
	
	/* Control loop */
	while(1) {
			
		/* Check if there is data available from the USB */
		if ((UART0_S1 & UART_S1_RDRF_MASK) >> (UART_S1_RDRF_SHIFT) == 1 /* Check the RDRF flag for the USB UART */) {
			/* NOTE: Never block in this statement. */
					
			// Retrieve the character
			char ch = UART0_D;
					
			/* If statements to handle the character */
			if (ch == '\n' || ch == '\r') {
				/** 
					* Enter pressed. Phone terminal may not \r or \n
				  * Manual entering of a character may be needed
					* Some terminals enter \r then \n. Handle this.
				*/
							 
				/* Handle text coming from the PC */ 
							
				/*     ON THE PHONE SIDE    */
				// Clear current text
				uart3_put("\r");
				for (int i = 0; i < CHAR_COUNT; i++) {
				uart3_put(" ");
				}
				uart3_put("\r");
							
				// Print the PC's message with a newline
				uart3_put("PC> ");
				uart3_put(PC_buffer);
				uart3_put("\n\r");
							
				// Restore the current text
				uart3_put("> ");
				uart3_put(phone_buffer);
							
				/*      ON THE PC SIDE      */
				uart0_put("\r");
				for (int i = 0; i < CHAR_COUNT; i++) {
				uart0_put(" ");
				}
				uart0_put("\r");
				uart0_put("PC> ");
				uart0_put(PC_buffer);
				
				// Clear buffer
				memset(PC_buffer, 0, sizeof(PC_buffer));
				PC_buffer_ptr = &PC_buffer[0];

				// Newline and prompt
				uart0_put("\n\r");
				uart0_put("> ");
							
			} else if (ch == '\b' || ch == DEL) {
				/* 
				 * Backspace detected. Some terminals don't use \b character
				 * Some enter DEL. Use an ASCII table & debugger to figure out 
				 * the correct character to check for
				*/
				/* Handle backspace */
							
				// Check for buffer underflow
				if (strlen(PC_buffer) > 0) {
					PC_buffer_ptr--;
					*PC_buffer_ptr = '\0';
					
					uart0_putchar('\b');
					uart0_putchar(' ');
					uart0_putchar('\b');
				}		
				
			} else {
				/* Non-special character entered */
							
				// Check for buffer overflow
				if (strlen(PC_buffer) < CHAR_COUNT) {
					uart0_putchar(ch);
					*PC_buffer_ptr = ch;
					PC_buffer_ptr++;
				}			
			} 
		}
			
		/*  
			Repeat the logic for handling PC characters, for the Phone characters
		
			Separate non-blocking if statements function as different tasks
			This is a simple form of multi-tasking & task scheduling.
			Useful for simple polling opperations. 
			We will learn how to handle this more effciently in later labs. 
		*/
		/* Check if there is data available from the phone */
		if ((UART3_S1 & UART_S1_RDRF_MASK) >> (UART_S1_RDRF_SHIFT) == 1 /* Check the RDRF flag for the USB UART */) {
			/* NOTE: Never block in this statement. */
					
			// Retrieve the character
			char ch = UART3_D;
					
			/* If statements to handle the character */
			if (ch == '\n' || ch == '\r') {
				/** 
					* Enter pressed.
				*/
							 
				/* Handle text coming from the Phone */ 
							
				/*     ON THE PC SIDE    */
				// Clear current text
				uart0_put("\r");
				for (int i = 0; i < CHAR_COUNT; i++) {
				uart0_put(" ");
				}
				uart0_put("\r");
							
				// Print the Phone's message with a newline
				uart0_put("Phone> ");
				uart0_put(phone_buffer);
				uart0_put("\n\r");
							
				// Restore the current text
				uart0_put("> ");
				uart0_put(PC_buffer);
							
				/*      ON THE PHONE SIDE      */
				uart3_put("\r");
				for (int i = 0; i < CHAR_COUNT; i++) {
				uart3_put(" ");
				}
				uart3_put("\r");
				uart3_put("Phone> ");
				uart3_put(phone_buffer);
				
				// Clear buffer
				memset(phone_buffer, 0, sizeof(phone_buffer));
				phone_buffer_ptr = &phone_buffer[0];

				// Newline and prompt
				uart3_put("\n\r");
				uart3_put("> ");
							
			} else if (ch == '\b' || ch == DEL) {
				/* Handle backspace */
							
				// Check for buffer underflow
				if (strlen(phone_buffer) > 0) {
					phone_buffer_ptr--;
					*phone_buffer_ptr = '\0';
					
					uart3_putchar('\b');
					uart3_putchar(' ');
					uart3_putchar('\b');
				}		
				
			} else {
				/* Non-special character entered */
							
				// Check for buffer overflow
				if (strlen(phone_buffer) < CHAR_COUNT) {
					uart3_putchar(ch);
					*phone_buffer_ptr = ch;
					phone_buffer_ptr++;
				}			
			} 
		}
	}   
}  
#endif /* LED_CODE */
