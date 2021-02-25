/*
Title: adc_main.c
Purpose: Main code for ADC Lab
Name: Jacob Meyerson
Date: 2/26/21
*/
 
#include "uart.h"
#include "MK64F12.h"
#include "led.h"
#include <stdio.h>

/* Function Prototypes */
void PDB_Init(void);
void ADC1_Init(void);
void DAC0_Init(void);
void ADC1_IRQHandler(void);
 
void PDB_Init() {
	// Enable clock for PDB module
	SIM_SCGC6 |= SIM_SCGC6_PDB_MASK; // Enable clock on PDB

	// Set continuous mode, software triggering, and PDB enabled
	PDB0_SC |= PDB_SC_CONT_MASK;    // continuous mode
	PDB0_SC |= PDB_SC_TRGSEL(15);   // set the PDB for software triggering (1111)
	PDB0_SC |= PDB_SC_PDBEN_MASK;   // bit 7

	// Set the mod field
	PDB0_MOD = 50000; // 50,000,000 / 50,000 = 1000
	
	//PDB0_CNT = 0x0000; this gets reset when the SWTRIG gets set

	// Configure the Interrupt Delay register.
	PDB0_IDLY = 10;

	// Enable the interrupt mask.
	PDB0_SC |= PDB_SC_PDBIE_MASK;   // bit 5

	// Enable LDOK to have PDB0_SC register changes loaded.
	PDB0_SC |= PDB_SC_LDOK_MASK;
	
	PDB0_CH1C1 = PDB_C1_EN(0x01) | PDB_C1_TOS(0x01);
}
 
void ADC1_Init() {
	unsigned int calib;
 
	// Turn on clock for ADC1
	SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;

	// Configure CFG Registers
	// Configure ADC to divide 50 MHz down to 6.25 MHz AD Clock (divide by 8), 16-bit single ended
	ADC1_CFG1 |= ADC_CFG1_ADIV(3); // 11 - divide by 8
	ADC1_CFG1 |= ADC_CFG1_MODE(3); // 11 - 16 bit single ended

	// Do ADC Calibration for Singled Ended ADC. Do not touch.
	ADC1_SC3 = ADC_SC3_CAL_MASK;
	while ( (ADC1_SC3 & ADC_SC3_CAL_MASK) != 0 );
	calib = ADC1_CLP0;
	calib += ADC1_CLP1;
	calib += ADC1_CLP2;
	calib += ADC1_CLP3;
	calib += ADC1_CLP4;
	calib += ADC1_CLPS;
	calib = calib >> 1;
	calib |= 0x8000;
	ADC1_PG = calib;

	// Configure SC registers.
	// Select hardware trigger.
	ADC1_SC2 |= ADC_SC2_ADTRG_MASK; // set to 1 for hardware trigger (0 for software trigger)

	// Configure SC1A register.
	// Select ADC Channel and enable interrupts. Use ADC1 channel DADP3 in single ended mode.
	// NOTE: use "=" for the macro, then use |= for the masks. If the macro goes last, then the register has to be cleared before it can be set. 
	ADC1_SC1A = ADC_SC1_ADCH(3); // 00011 is for DADP3 or DAD3, chooses  DADP3 when DIFF = 0
	ADC1_SC1A |= ADC_SC1_AIEN_MASK; // enable interrupts
	ADC1_SC1A &= ~(ADC_SC1_DIFF_MASK); // set DIFF to 0

	// Enable NVIC interrupt
	NVIC_EnableIRQ(ADC1_IRQn);
}
 
// ADC1 Conversion Complete ISR
void ADC1_IRQHandler() {
	// Read the result (upper 12-bits). This also clears the Conversion complete flag.
  unsigned short i = (unsigned short) (ADC1_RA >> 4); // Added casting to remove a warning

  //Set DAC output value (12bit)
  DAC0_DAT0L = i & DAC_DATL_DATA0_MASK;        
	DAC0_DAT0H = (i >> 8) & DAC_DATH_DATA1_MASK;
}

void DAC0_Init() {
	// Enable DAC clock
	SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;
	
	DAC0_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK;
	DAC0_C1 = 0;
}
 
int main(void) {
	int i = 0; char str[100];

	// Initialize modules
	UART0_Init();
	DAC0_Init();
	ADC1_Init();
	PDB_Init();

	// Start the PDB (ADC Conversions)
	PDB0_SC |= PDB_SC_SWTRIG_MASK;

	#define TEMP_SENSOR
	for(;;) {
#ifdef TEMP_SENSOR /* Define TEMP_SENSOR to print the temperature conversion*/
		double celcius = (((3300.0/65535) * ADC1_RA) - 500)/10; // TODO - figure out why this works
		double fahrenheit = ((9.0/5.0) * celcius) + 32;
		sprintf(str,"\n Celcius: %f Fahrenheit: %f \n\r", celcius, fahrenheit);
		UART0_Put(str);
#else /* TEMP_SENSOR */
		sprintf(str,"\n Decimal: %d Hexadecimal: %x \n\r", ADC1_RA, ADC1_RA);
		UART0_Put(str);
#endif /* TEMP_SENSOR */
		for(i = 0; i < 5000000; ++i ){
			// TODO - something happens here?	Or is this to delay?	
		}
	}
}
 
