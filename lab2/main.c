/*
Title: main.c
Description: Main loop code for UART lab
Authors: Jacob Meyerson
Date: 2/5/21
*/

#include "MK64F12.h"
#include "uart.h"
#include <stdio.h>

#include <string.h>

// Function prototypes
void put(char *ptr_str);
int main(void);

int main() {
	
	uart_init();
	
	/* UART Function Implementation: */
	// put("IDE: Lab 2 Demonstration by Jacob Meyerson");    /* Transmit this through UART */
	
	/* UART Echo Program */
	while (1) {
		uart_put("Enter a sentence: "); 
		
		char user_str[CHAR_COUNT];
		unsigned int char_cnt = 0;
		char temp_char = 0x00;
		while (CR != temp_char) {
			temp_char = uart_getchar();
			if (CHAR_COUNT > char_cnt) {
				uart_putchar(temp_char);
				strncat(user_str, &temp_char, 1);
				char_cnt++;
			}
		} 
		uart_put("\n\r");
		uart_put("Your sentence: ");
		uart_put(&user_str[0]);
		uart_put("\n\r");
		
		memset(user_str, 0, strlen(user_str));
	}
}  

void put(char *ptr_str) {
	while(*ptr_str) {
		uart_putchar(*ptr_str++);
	}
}
