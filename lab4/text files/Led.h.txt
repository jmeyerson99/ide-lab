/*
Title: Led.h
Purpose: On-board LED interface
Name: Jacob Meyerson
Date: 1/29/21
*/
// NOTE: The LEDs on the K64 are all active LOW
#ifndef LED_H
#define LED_H

#include "MK64F12.h"                    // Device header

void LED_Off(void);
void LED_Init(void);
void LED_On(unsigned char color);
void delay(void);
void flash_LED(unsigned char color);

#endif /* LED_H */
