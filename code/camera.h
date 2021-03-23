/*
 * Freescale Cup linescan camera code
 *
 *	This method of capturing data from the line
 *	scan cameras uses a flex timer module, periodic
 *	interrupt timer, an ADC, and some GPIOs.
 *	CLK and SI are driven with GPIO because the FTM2
 *	module used doesn't have any output pins on the
 * 	development board. The PIT timer is used to 
 *  control the integration period. When it overflows
 * 	it enables interrupts from the FTM2 module and then
 *	the FTM2 and ADC are active for 128 clock cycles to
 *	generate the camera signals and read the camera 
 *  output.
 *
 *	PTB8	    - camera CLK
 *	PTB23 		- camera SI
 *  ADC0_DP1 	- camera AOut
 *
 * Author:  Alex Avery
 * Created:  11/20/15
 * Modified:  11/23/15
 */
#ifndef CAMERA_H
#define CAMERA_H

#include "MK64F12.h"
#include "uart.h"
#include "led.h"
#include "stdio.h"

// Default System clock value
// period = 1/20485760  = 4.8814395e-8
#define DEFAULT_SYSTEM_CLOCK 20485760u 

// Integration time (seconds)
// Determines how high the camera values are
// Don't exceed 100ms or the caps will saturate
// Must be above 1.25 ms based on camera clk 
//	(camera clk is the mod value set in FTM2)
#define INTEGRATION_TIME (2.0f * .0075f) //NOTE: default = .0075 //NOTE: this will change based on light levels??? More light, less time
#define DEBUG_CAM // define this for when the camera is being debugged (connected to matlab)

void FTM2_Init(void);
void GPIO_Init(void);
void PIT_Init(void);
void ADC0_Init(void);
void FTM2_IRQHandler(void);
void PIT0_IRQHandler(void);
void ADC0_IRQHandler(void);

void Camera_Init(void);

#ifdef DEBUG_CAM
void Debug_Camera(void);
#endif /* DEBUG_CAM */

// Pixel counter for camera logic
// Starts at -2 so that the SI pulse occurs
//		ADC reads start
static int pixcnt = -2;
// clkval toggles with each FTM interrupt
static int clkval = 0;
// line stores the current array of camera data
static uint16_t line[128];

// These variables are for streaming the camera data over UART
static int capcnt = 0;
static char str[100];

// ADC0VAL holds the current ADC value
static uint16_t ADC0VAL;

#endif /* CAMERA_H */