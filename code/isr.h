/*
Title: isr.h
Description: ISR handler
Authors: Jacob Meyerson
Date: 2/26/21
*/
#ifndef ISR_H
#define ISR_H

#include "MK64F12.h"
#include <stdio.h>
#include "uart.h"
#include "led.h"
#include "switch.h"

/* LED State ENUM */
typedef enum {OFF=0, ON=1} LED_State;

/* Boolean typedef */
typedef enum {FALSE=0, TRUE=1} bool;

/* PDB Status ENUM */
typedef enum {DISABLED=0, ENABLED=1} PDB_State;

/* Function prototypes */
void PDB0_IRQHandler(void);
void FTM0_IRQHandler(void);
void PORTA_IRQHandler(void);
void PORTC_IRQHandler(void);

#endif /* ISR_H */
