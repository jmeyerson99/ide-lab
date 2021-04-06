/*
Title: uart.h
Purpose: Implement UART routines for serial IO
Name: Jacob Meyerson
Date: 2/19/21
*/
#ifndef UART_H
#define UART_H

#include "MK64F12.h"
#include <stdint.h>
#include <stdio.h>

#define BAUD_RATE 9600      // default baud rate 
#define SYS_CLOCK 20485760  // default system clock (see DEFAULT_SYSTEM_CLOCK  in system_MK64F12.c)
#define CHAR_COUNT 80       // change this to modify the max. permissible length of a sentence

#define CR 0x0D   // carriage return constant
#define DEL 0x7F  // delete key constant

// Function prototypes
void UART0_Init(void);
uint8_t UART0_Getchar(void);
void UART0_Putchar(char ch);
void UART0_Put(char *ptr_str);
void UART0_PutNumU(int i);

void UART3_Init(void);
uint8_t UART3_Getchar(void);
void UART3_Putchar(char ch);
void UART3_Put(char *ptr_str);
void UART3_PutNumU(int i);

#endif /* UART_H */
