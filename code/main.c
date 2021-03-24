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
    UART0_Init();  // for serial
		Camera_Init(); // for camera
    UART3_Init();  // for bluetooth
    PWM_Init();    // for motors
    Servo_Init();  // for servo
    LED_Init();    // for on board LED
		EN_init();		 // for motor enable
}

int main(void) {
    Car_Init();
		
		//Spin_Left_Motor(30,FORWARD);
		//Spin_Right_Motor(30,FORWARD);
		
    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
    Debug_Camera();
#endif /* DEBUG_CAM */

    // process the line here
			for(int i = 1; i < 128; i++){
				if(i > 2 && i < 126){
					smoothline[i] = (line[i] + line[i+1] + line[i+2] + line[i-1] + line[i-2])/5;
				}
			}

    // create an enum for car states based on the line data 
    // use a switch statement to control car logic

    }
}
