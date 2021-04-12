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
void PDB_Init(void);
void ADC1_Init(void);
void DAC0_Init(void);
void ADC1_IRQHandler(void);
void FTM_Init(void);
void FTM0_IRQHandler(void);
void print_BPM(void);

/* From clock setup 0 in system_MK64f12.c */
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */

static volatile int cnt = 0;
static volatile double vout = 0.0;
static volatile int peaks = 0;

static volatile int print = 0;
/*
static int increment_counter = 0; // use this to incrememnt the counter

static double last_vout = 0.0;
static int slope = 0;
static int last_slope = 0; 
static int loop_cnt = 0;
static double BPM_averager = 0.0;*/
static char str[100];

static double data[5000];
 
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
	ADC1_SC1A = 0;
	ADC1_SC1A = ADC_SC1_ADCH(3); // 00011 is for DADP3 or DAD3, chooses  DADP3 when DIFF = 0
	ADC1_SC1A |= ADC_SC1_AIEN_MASK; // enable interrupts
	ADC1_SC1A &= ~(ADC_SC1_DIFF_MASK); // set DIFF to 0

	// Enable NVIC interrupt
	NVIC_EnableIRQ(ADC1_IRQn);
}
 
// ADC1 Conversion Complete ISR
void ADC1_IRQHandler() {
	vout = (((3300.0/65536.0) * ADC1_RA)/1000.0);
	
	
	data[cnt] = vout; //(uint16_t) ADC1_RA; // vout;
	cnt++;
	if (cnt == 5000) { print = 1;cnt = 0;peaks = 0;}
	
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

void DAC0_Init() {
	// Enable DAC clock
	SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;
	
	DAC0_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK;
	DAC0_C1 = 0;
}
 
int main(void) {
	// Initialize modules
	UART0_Init();
	DAC0_Init();
	ADC1_Init();
	PDB_Init();
	FTM_Init();

	// Start the PDB (ADC Conversions)
	PDB0_SC |= PDB_SC_SWTRIG_MASK;

	for(;;) {
		// Vcc = 3.3 V 
		// Resolution = 16 bits
		// vout = ((3300 mV / (2^16) levels) * ADC)
		// use pin ADC0_DP0
		//vout = (((3300.0/65536.0) * ADC1_RA)/1000.0);
		
		//sprintf(str,"vout = %f, cnt = %d\n\r", vout, cnt); // DEBUG
	  //UART0_Put(str); // DEBUG
		
		if (print) {
				int above_threshold = 0;
				for (int i = 0; i < 5000; i++) {
					if (data[i] > 3.0 && above_threshold == 0) { peaks++; above_threshold = 1;}
					if (data[i] < 3.0 && above_threshold == 1) { above_threshold = 0;}
				}
				sprintf(str,"peaks = %d, BPM = %d\n\r", peaks, peaks); // print the counter (counter incrememts by 1 every 1 ms)
				UART0_Put(str);
				print = 0;
		}
	
		//sprintf(str,"vout = %f, slope = %d, last_slope = %d\n\r", vout, slope, last_slope);
		//UART0_Put(str);
	}
}

void print_BPM() {
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
	print = 1;
}

void FTM0_IRQHandler(void){ // For FTM timer
	// Clear the interrupt in regster FTM0_SC
	FTM0_SC &= ~(FTM_SC_TOF_MASK);
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
	
	
	data[cnt] = vout; //(uint16_t) ADC1_RA; // vout;
	cnt++;
	if (cnt == 5000) { print=1;cnt = 0;peaks = 0;} 
}

void FTM_Init(void){
	// Enable clock for FTM module (use FTM0)
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

	// turn off FTM Mode to write protection;
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;

	// divide the input clock down by 128,
	FTM0_SC |= FTM_SC_PS(7); // 111 = divide by 128

	// reset the counter to zero
	FTM0_CNT &= ~(FTM_CNT_COUNT_MASK);

	// Set the overflow rate
	// (Sysclock/128)- clock after prescaler
	// (Sysclock/128)/1000- slow down by a factor of 1000 to go from
	// Mhz to Khz, then 1/KHz = msec
	// Every 1msec, the FTM counter will set the overflow flag (TOF) and
	FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// Select the System Clock
	FTM0_SC |= FTM_SC_CLKS(1); // 1 = system clock

	// Enable the interrupt mask. Timer Overflow Interrupt Enable
	FTM0_SC |= FTM_SC_TOIE_MASK;
	
	//NVIC_EnableIRQ(FTM0_IRQn);
}
