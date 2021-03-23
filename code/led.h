/*
Title: led.h
Purpose: On-board LED interface
Authors: Jacob Meyerson & Charlie Poliwoda
Date: 1/29/21
*/
// NOTE: The LEDs on the K64 are all active LOW
#ifndef LED_H
#define LED_H

#include "MK64F12.h"                    // Device header

// Function prototypes
void LED_Off(void);
void LED_Init(void);
void LED_On(unsigned char color);
void Delay(void);
void Flash_LED(unsigned char color);

#endif /* LED_H */
