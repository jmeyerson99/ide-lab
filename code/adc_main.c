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
#include <string.h>

/* Function Prototypes */
void ADC0_Init(void);
void ADC0_IRQHandler(void);
void FTM2_Init(void);
void FTM2_IRQHandler(void);
void print_BPM(void);

/* From clock setup 0 in system_MK64f12.c */
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */

static volatile int cnt = 0;
static volatile double vout = 0.0;
static int above_peak = 0;
static int increment_counter = 0; // use this to incrememnt the counter
static char str[100];
 
static double BPM_data[5];
static int data_cnt = 0;
 
// ADC0 Conversion Complete ISR
void ADC0_IRQHandler() {
	vout = (((3300.0/65536.0) * ADC0_RA)/1000.0);

	/*
	data[cnt] = vout; //(uint16_t) ADC1_RA; // vout;
	cnt++;
	
	if (cnt == 5000) { 
		int above_threshold = 0;
		//double slope = 0; // slope
		//double last_slope = 0; // slope
		for (int i = 1; i < 5000; i++) {
			//slope = data[i] - data[i-1]; // slope
			
			if (data[i] > 3.0 && above_threshold == 0) { peaks++; above_threshold = 1;}
			if (data[i] < 1.5 && above_threshold == 1) { above_threshold = 0;}
			
			//if (last_slope > 0 && slope < 0) {peaks++;} // slope
			
			//last_slope = slope; // slope
		}
		BPM = peaks * 60/5;
		print = 1;
		cnt = 0;
		peaks = 0;
	}
	*/
	
	/*
	last_slope = slope;
	last_vout = vout;
	vout = (((3300.0/65536.0) * ADC1_RA)/1000.0);
	//sprintf(str,"vout = %f\n\r", vout); // print the counter (counter incrememts by 1 every 1 ms)
	//UART0_Put(str);

	if (vout - last_vout > 0) {slope = 1;}
	if (vout - last_vout < 0) {slope = -1;}
	*/
}
 
int main(void) {
	// Initialize modules
	UART0_Init();
	ADC0_Init();
	FTM2_Init();

	for(;;) {
		// Vcc = 3.3 V 
		// Resolution = 16 bits
		// vout = ((3300 mV / (2^16) levels) * ADC)
		// use pin ADC0_DP0
		//vout = (((3300.0/65536.0) * ADC0_RA)/1000.0);
		
		//sprintf(str,"vout = %f, cnt = %d\n\r", vout, cnt); // DEBUG
	  //UART0_Put(str); // DEBUG

		//sprintf(str,"cnt = %d, BPM = %g\n\r", cnt, cnt*60.0/1000.0); // print the counter (counter incrememts by 1 every 1 ms)
		//UART0_Put(str);
		
		//for (int j = 0; j < 5000000; j++) {}
	
		//sprintf(str,"vout = %f, slope = %d, last_slope = %d\n\r", vout, slope, last_slope);
		//UART0_Put(str);
	}
}

	/*
	sprintf(str,"Cnt = %d\n\r", cnt); // print the counter (counter incrememts by 1 every 1 ms)
	UART0_Put(str);
	if (loop_cnt == 5) {
		sprintf(str,"BPM = %lf\n\r", (BPM_averager/5)); // print the counter (counter incrememts by 1 every 1 ms)
		UART0_Put(str);
		cnt = 0;
		//print_data = 0;
		increment_counter = 0;
		loop_cnt = 0;
		BPM_averager = 0;
	}
	BPM_averager = BPM_averager + (cnt/(1000.0))*(60.0);
	loop_cnt++; */
	
	/*
	int above_threshold = 0;
	for (int i = 0; i < 5000; i++) {
		if (data[i] > 3.1 && above_threshold == 0) { peaks++; above_threshold = 1;}
		if (data[i] < 3.1 && above_threshold == 1) { above_threshold = 0;}
	}
	sprintf(str,"peaks = %d, BPM = %d\n\r", peaks, peaks * 60/5); // print the counter (counter incrememts by 1 every 1 ms)
	UART0_Put(str);
	*/
	
void print_BPM() {
	BPM_data[data_cnt] = (1.0/cnt) *  (1000.0 * 60.0);
	data_cnt++;
	if (data_cnt == 5) {
		double BPM = (BPM_data[0] + BPM_data[1] + BPM_data[2] + BPM_data[3] + BPM_data[4])/5.0;
		sprintf(str,"cnt = %d, BPM = %g\n\r", cnt, BPM); // print the counter (counter incrememts by 1 every 1 ms)
		UART0_Put(str);
		data_cnt = 0;
	}
	
}

void FTM2_IRQHandler(void){ // For FTM timer
	// Clear the interrupt in regster FTM2_SC
	FTM2_SC &= ~(FTM_SC_TOF_MASK);
	/*
	if (slope == 1 && last_slope == -1 && increment_counter == 0) {
		// found peak, start counting
		increment_counter = 1;
	}
	else if (slope == 1 && last_slope == -1 && increment_counter == 1) {
		// reached second peak, print data
		print_BPM();
	}
	
	if (increment_counter) {
		cnt++;
	} */
	
	/*
	data[cnt] = vout; //(uint16_t) ADC1_RA; // vout;
	cnt++;
	if (cnt == 5000) { print=1;cnt = 0;peaks = 0;} 
	*/
	
	if (vout > 2.67 && above_peak == 0 && increment_counter == 0) {
		// found peak, start counting
		increment_counter = 1;
		above_peak = 1;
	}
	else if (vout > 2.67 && above_peak == 0 && increment_counter == 1) {
		// reached second peak, print data
		print_BPM();
		increment_counter = 0;
		above_peak = 0;
		cnt = 0;
	}
	else if (vout < 1.5 && above_peak == 1 && increment_counter == 1) {
		// dropped below the first peak
		above_peak = 0;
	}
	
	if (increment_counter) {
		cnt++;
	} 
}

/* Initialization of FTM2 for camera */
void FTM2_Init(){
	
	// Enable clock
	SIM_SCGC6 |= SIM_SCGC6_FTM2_MASK;

	// Disable Write Protection
	FTM2_MODE |= FTM_MODE_WPDIS_MASK;
	
	// Set output to '1' on init
	FTM2_MODE |= FTM_MODE_INIT_MASK;
	FTM2_OUTINIT |= FTM_OUTINIT_CH0OI_MASK; // NOTE: channel 0
	FTM2_MODE |= FTM_MODE_FTMEN_MASK; // enable all registers
	
	// Initialize the CNT to 0 before writing to MOD
	FTM2_CNT &= ~(FTM_CNT_COUNT_MASK);
	
	// Set the Counter Initial Value to 0
	FTM2_CNT = 0x0000;
	FTM2_CNTIN = 0;

	// Set the period (~10us)
	FTM2->MOD = (DEFAULT_SYSTEM_CLOCK)/(1000); 
	
	// 50% duty
	FTM2_C0V = ((DEFAULT_SYSTEM_CLOCK)/(1000/2)); 
	//NOTE: CNTIN = 0x0000 in EPWM mode
	//NOTE: 50% of the MOD register (~5us)

	// Set edge-aligned mode
	// Conditions: QUADEN = 0, DECAPEN = 0, COMBINE = 0, CPWMS = 0, MSnB = 1
	FTM2_QDCTRL &= ~FTM_QDCTRL_QUADEN_MASK; //NOTE: channel 0
	FTM2_COMBINE &= ~FTM_COMBINE_DECAPEN0_MASK; //NOTE: channel 0
	FTM2_COMBINE &= ~FTM_COMBINE_COMBINE0_MASK; //NOTE: channel 0
	FTM2_SC &= ~FTM_SC_CPWMS_MASK;
	FTM2_C0SC |= FTM_CnSC_MSB_MASK;            	//NOTE: channel 0
	
	// Enable High-true pulses
	// ELSB = 1, ELSA = 0
	FTM2_C0SC &= ~(FTM_CnSC_ELSA_MASK);
	FTM2_C0SC |= FTM_CnSC_ELSB_MASK;
	
	// Enable hardware trigger from FTM2
	FTM2_EXTTRIG |= FTM_EXTTRIG_CH0TRIG_MASK;
	
	// Don't enable interrupts yet (disable)
	FTM2_SC &= ~(FTM_SC_TOIE_MASK);
	
	// No prescalar, system clock
	FTM2_SC |= FTM_SC_PS(0); //  000 = divide by 1 (no prescalar)
	FTM2_SC |= FTM_SC_CLKS(1); // 1 = system clock
	
	// Set up interrupt
	FTM2_SC |= FTM_SC_TOIE_MASK;

	// Enable IRQ
	NVIC_EnableIRQ(FTM2_IRQn);
	
	// Re enable write protection
	FTM2_MODE &= ~(FTM_MODE_WPDIS_MASK);
}

/* Set up ADC for capturing camera data */
void ADC0_Init(void) {
  unsigned int calib;
	
  // Turn on ADC0
  SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;
	
	// Single ended 16 bit conversion, no clock divider
	ADC0_SC1A = 0;
	ADC0_SC1A &= ~(ADC_SC1_DIFF_MASK); // set DIFF to 0 for 16 bit conversion
	ADC0_CFG1 |= ADC_CFG1_ADIV(0); // 0 - divide by 1
	ADC0_CFG1 |= ADC_CFG1_MODE(3); // 11 - 16 bit single ended
	
	ADC0_SC1A |= ADC_SC1_ADCH(0); // 00000 is for DADP0 or DAD0, chooses  DADP0 when DIFF = 0
	ADC0_SC1A |= ADC_SC1_AIEN_MASK; // enable interrupts

	// Do ADC Calibration for Singled Ended ADC. Do not touch.
	ADC0_SC3 = ADC_SC3_CAL_MASK;
	while ( (ADC0_SC3 & ADC_SC3_CAL_MASK) != 0 );
	calib = ADC0_CLP0; calib += ADC0_CLP1; calib += ADC0_CLP2;
	calib += ADC0_CLP3; calib += ADC0_CLP4; calib += ADC0_CLPS;
	calib = calib >> 1; calib |= 0x8000;
	ADC0_PG = calib;
	
	// Select hardware trigger.
	ADC0_SC2 |= ADC_SC2_ADTRG_MASK; // set to 1 for hardware trigger (0 for software trigger)
	
	// Set up FTM2 trigger on ADC0
	SIM_SOPT7 |= SIM_SOPT7_ADC0TRGSEL(10); // 1010 - FTM2 trigger select
  SIM_SOPT7 |= SIM_SOPT7_ADC0ALTTRGEN_MASK; // alternative trigger edge enabled
  SIM_SOPT7 &= ~(SIM_SOPT7_ADC0PRETRGSEL_MASK); // Pretrigger A
	
	// Enable NVIC interrupt
	NVIC_EnableIRQ(ADC0_IRQn);
}
