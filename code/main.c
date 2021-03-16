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
	FTM3_PWM_Init();


	// Print welcome over serial
	UART0_Put("Running... \n\r");
	
	#define PART_1
	
#ifdef PART_1
	// Generate 20% duty cycle at 10kHz
	//FTM0_set_duty_cycle(20,10000,1); //20% duty cycle, 10kHz, forward direction
	//for(;;) ;  //then loop forever
	
	//TODO: this is for the servo motor in step 15
	for(;;){  //then loop forever
		FTM3_set_duty_cycle(6.6);
		//delay(5);
		//FTM3_set_duty_cycle(8.3);
		//delay(5);
		//FTM3_set_duty_cycle(6.6);
		//delay(5);
		//FTM3_set_duty_cycle(4.9);
		//delay(5);
	}
	
#elif PART_2
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
	
#else
	// Enable clocks on Port D.
	SIM_SCGC5 = SIM_SCGC5_PORTD_MASK;
	// Configure the Signal Multiplexer for the LEDs
	PORTD_PCR0 = PORT_PCR_MUX(1);
	PORTD_PCR1 = PORT_PCR_MUX(1);
	PORTD_PCR2 = PORT_PCR_MUX(1);
	PORTD_PCR3 = PORT_PCR_MUX(1);
	// Configure the GPIO Pins for Output.
	GPIOD_PDDR |= ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
	int phase = 0;
	int forward = 1;
	while(1) {
		//Turn off all coils, Set the GPIO pins to 0.
		GPIOD_PCOR = 0x0F;
		
		// set one pin high at a time
		if(forward){
			if(phase == 0){GPIOD_PSOR = (1 << 0); phase++;} //A, 1a
			else if(phase == 1){GPIOD_PSOR = (1 << 1); phase++;} //B, 2b
			else if(phase == 2){GPIOD_PSOR = (1 << 2); phase++;} //C, 1b
			else{GPIOD_PSOR = (1 << 3); phase = 0; forward = 0;} //D, 2b
		}
		else{ //reverse
		if(phase == 0){GPIOD_PSOR = (1 << 3); phase++;} //D, 2b
			else if(phase == 1){GPIOD_PSOR = (1 << 2); phase++;} //C, 1b
			else if(phase == 2){GPIOD_PSOR = (1 << 1); phase++;} //B, 2a
			else{GPIOD_PSOR = (1 << 0); phase = 0; forward = 1;} //A, 1a
		}
		delay(2);
	}
#endif
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
