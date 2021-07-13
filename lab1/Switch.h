/*
Title: Switch.h
Purpose: On-board LED interface
Name: Jacob Meyerson
Date: 1/29/21
*/
// NOTE: The Switches on the K64 are all active LOW
#ifndef SWITCH_H
#define SWITCH_H

#include "MK64F12.h"             // Device header

void Switch2_Init(void);
unsigned char Switch2_Pressed(void);

#endif /* SWITCH_H */
