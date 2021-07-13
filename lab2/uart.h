/*
Title: uart.h
Purpose: Implement UART routines for serial IO
Name: Jacob Meyerson
Date: 2/5/21
*/
#ifndef UART_H
#define UART_H

#include "MK64F12.h"

#define BAUD_RATE 9600      // default baud rate 
#define SYS_CLOCK 20485760  // default system clock (see DEFAULT_SYSTEM_CLOCK  in system_MK64F12.c)
#define CHAR_COUNT 255      // change this to modify the max. permissible length of a sentence

#define CR 0x0D // carriage return constant

// Function prototypes
void uart_init(void);
uint8_t uart_getchar(void);
void uart_putchar(char ch);
void uart_put(char *ptr_str);

#endif /* UART_H */
