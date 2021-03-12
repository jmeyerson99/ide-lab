/*
 * Main module for testing the PWM Code for the K64F
 * 
 * Author:  
 * Created:  
 * Modified: Carson Clarke-Magrab <ctc7359@rit.edu> 
 */

#include "MK64F12.h"
#include "uart.h"
#include "pwm.h"

void delay(int del);

int main(void) {
	// Initialize UART and PWM
	UART0_Init();
	FTM0_PWM_Init();

	// Print welcome over serial
	UART0_Put("Running... \n\r");
	
	//PART 1
	// Generate 20% duty cycle at 10kHz
	//FTM0_set_duty_cycle(20,10000,1); //20% duty cycle, 10kHz, forward direction
	//for(;;) ;  //then loop forever
	
	
	//PART 2
	for(;;)  //loop forever
	{
		uint16_t dc = 0;
		uint16_t freq = 10000; // Frequency = 10 kHz 
		uint16_t dir = 0;
		char c = 48;
		unsigned int i=0;
		
		// 0 to 100% duty cycle in forward direction
		for (i=0; i<=100; i++) {
		    FTM0_set_duty_cycle(i,freq,~dir);
			
			delay(10);
		}
		
		// 100% down to 0% duty cycle in the forward direction
		for (i=100; i>0; i--) {
		    FTM0_set_duty_cycle(i,freq,~dir);
			
			delay(10);
		}
		
		
		// 0 to 100% duty cycle in reverse direction
		for (i=0; i<=100; i++) {
		    FTM0_set_duty_cycle(i,freq,dir);
			
			delay(10);
		}
		
		
		// 100% down to 0% duty cycle in the reverse direction
		for (i=100; i>0; i--) {
		    FTM0_set_duty_cycle(i,freq,dir);
			
			delay(10);
		}
	}
}


/**
 * Waits for a delay (in milliseconds)
 * 
 * del - The delay in milliseconds
 */
void delay(int del){
	int i;
	for (i=0; i<del*50000; i++){
		// Do nothing
	}
}

/*
// Enable clocks on Port D

// Configure the Signal Multiplexer for the Port D GPIO Pins

// Configure the GPIO Pins for Output

int forward = 1;
int phase = 0;

while(true){
	// turn off all coils, Set GPIO pins to 0

	// set one pin high at a time
	if(forward){
		if(phase == 0){//turn on coil A; phase++;} //A, 1a
		else if(phase == 1){//turn on coil B; phase++;} //B, 2b
		else if(phase == 2){//turn on coil C; phase++;} //C, 1b
		else{//turn on coil D; phase = 0;} //D, 2b
	}
	else{ //reverse
	if(phase == 0){//turn on coil D; phase++;} //D, 2b
		else if(phase == 1){//turn on coil C; phase++;} //C, 1b
		else if(phase == 2){//turn on coil B; phase++;} //B, 2a
		else{//turn on coil A; phase = 0;} //A, 1a
	}
	//NOTE: you need to write your own delay function??
	delay(10);
}
*/
