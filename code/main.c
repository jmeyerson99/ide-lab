/*
Title: main.c
Purpose: main code for NXP Cup Race
Authors: Jacob Meyerson & Charlie Poliwoda
Date: 3/23/21
*/

#include "camera.h"
#include "led.h"
#include "uart.h"
#include "pwm.h"

// Function prototypes
void Car_Init(void);

void Car_Init() {
    Camera_Init(); // for camera
    UART0_Init();  // for serial
    UART3_Init();  // for bluetooth
    PWM_Init();    // for motors
    Servo_Init();  // for servo
    LED_Init();    // for on board LED
}

int main(void) {
    Car_Init();

    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
    Debug_Camera();
#endif /* DEBUG_CAM */

    // process the line here

    // create an enum for car states based on the line data 
    // use a switch statement to control car logic

    }
}