/*
 * Pulse-Width-Modulation Code for K64
 * PWM signal can be connected to output pins PC3 and PC4
 * 
 * Author: Brent Dimmig <bnd8678@rit.edu>
 * Modified by: Carson Clarke-Magrab <ctc7359@rit.edu>
 * Created: 2/20/2014
 * Modified: 2/25/2020
 */
#include "MK64F12.h"
#include "pwm.h"

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK    20485760u 
#define FTM0_MOD_VALUE          (DEFAULT_SYSTEM_CLOCK/10000)
#define FTM3_MOD_VALUE          ((DEFAULT_SYSTEM_CLOCK/50/128) - 1) //NOTE: 50Hz



/*
 * Change the motor duty cycle and frequency
 *
 * @param duty_cycle (0 to 100)
 * @param frequency (~1000 Hz to 20000 Hz)
 * @param dir: 1 for pin C4 active, else pin C3 active 
 */
void FTM0_set_duty_cycle(unsigned int duty_cycle, unsigned int frequency, int dir)
{
	// Calculate the new cutoff value
	uint16_t mod = (uint16_t) (((DEFAULT_SYSTEM_CLOCK / frequency) * duty_cycle) / 100);
  
	// Set outputs 
	if(dir) {
	    FTM0_C3V = mod; 
	    FTM0_C2V = 0;
	} else {
	    FTM0_C2V = mod; 
	    FTM0_C3V = 0;
	}

	// Update the clock to the new frequency
	FTM0_MOD = (DEFAULT_SYSTEM_CLOCK / frequency);
}

/*
 * Change the motor duty cycle and frequency
 *
 * @param duty_cycle (0 to 100)
 * @param frequency (~1000 Hz to 20000 Hz)
 * @param dir: 1 for pin C4 active, else pin C3 active 
 */
void FTM3_set_duty_cycle(float duty_cycle)
{
	// Calculate the new cutoff value
	uint16_t mod = (uint16_t) (((FTM3_MOD_VALUE) * duty_cycle) / 100);
  
	// Set output 
	FTM3_C4V = mod;
}

/*
 * Initialize the FlexTimer for PWM
 */
void FTM0_PWM_Init()
{
	// 12.2.13 Enable the clock to the FTM3 Module
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;
	
	// Enable clock on PORT C so it can output the PWM signals
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
    PORTC_PCR3 = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch2
    PORTC_PCR4 = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch3
	
	// 39.3.10 Disable Write Protection
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM0_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM0_CNTIN = 0;
	
	// 39.3.5 Set the Modulo resister
	FTM0_MOD = FTM0_MOD_VALUE;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM0_C3SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C3SC &= ~FTM_CnSC_ELSA_MASK;
	
	// See Table 39-67,  Edge-aligned PWM, Low-true pulses (clear out on match)
	FTM0_C2SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C2SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.3 FTM Setup
	// Set prescale value to 1 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM0_SC = FTM_SC_PS(0) | FTM_SC_CLKS(1); 
}

void FTM3_PWM_Init()
{
	// 12.2.13 Enable the clock to the FTM3 Module
	SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK;
	
	// Enable clock on PORT C so it can output the PWM signals
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
  PORTC_PCR8 = PORT_PCR_MUX(3) | PORT_PCR_DSE_MASK; //enables channel 4 for PTC8
	
	// 39.3.10 Disable Write Protection
	FTM3_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM3_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM3_CNTIN = 0;
	
	// 39.3.5 Set the Modulo register
	FTM3_MOD = FTM3_MOD_VALUE;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM3_C4SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM3_C4SC &= ~FTM_CnSC_ELSA_MASK;
	
	
	// 39.3.3 FTM Setup
	// Set prescale value to 128 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM3_SC = FTM_SC_PS(7) | FTM_SC_CLKS(1); 
	
	NVIC_EnableIRQ(FTM3_IRQn);
}

void FTM3_IRQHandler()
{
	FTM3_SC &= ~FTM_SC_TOF_MASK;
}

