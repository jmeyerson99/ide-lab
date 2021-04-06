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
 *	PTB8			- camera CLK
 *	PTB23 		- camera SI
 *  ADC0_DP1 	- camera AOut
 *
 * Author:  Alex Avery
 * Created:  11/20/15
 * Modified:  11/23/15
 */

#include "camera.h"

// Local variables
// Pixel counter for camera logic
// Starts at -2 so that the SI pulse occurs
//		ADC reads start
static int pixcnt = -2;
// clkval toggles with each FTM interrupt
static int clkval = 0;
// line stores the current array of camera data
static uint16_t line[128];

static boolean line_ready = FALSE;

#ifdef DEBUG_CAMERA
// These variables are for streaming the camera data over UART
static int capcnt = 0;
static char str[100];
#endif

// ADC0VAL holds the current ADC value
static uint16_t ADC0VAL;

void Camera_Init() {
  GPIO_Init(); // For CLK and SI output on GPIO
	FTM2_Init(); // To generate CLK, SI, and trigger ADC
	ADC0_Init();
	PIT_Init();	// To trigger camera read based on integration time
}

int Get_Line(uint16_t* data) { 
	/* //Find alternative for commented out block
	while (1) {
		NVIC_DisableIRQ(FTM2_IRQn);
		if (TRUE == line_ready) {break;}
		NVIC_EnableIRQ(FTM2_IRQn);
	}
	for (int i = 0; i < 128; i++) {
		data[i] = line[i];
	}
	NVIC_EnableIRQ(FTM2_IRQn);
	line_ready = FALSE;
	*/
	if (TRUE == line_ready) {
		for (int i = 0; i < 128; i++) {
			data[i] = line[i];
		}
		line_ready = FALSE;
		return 1;
	}
	line_ready = FALSE;
	return 0;
}

#ifdef DEBUG_CAM
void Debug_Camera() {
    // Every 2 seconds
    //if (capcnt >= (2/INTEGRATION_TIME)) {
    if (capcnt >= (175)) {
        GPIOB_PCOR |= (1 << 22);
        // send the array over uart
        sprintf(str,"%i\n\r",-1); // start value
        UART0_Put(str);
        for (int i = 0; i < 127; i++) {
            sprintf(str,"%i\n\r", line[i]);
            UART0_Put(str);
        }
        sprintf(str,"%i\n\r",-2); // end value
        UART0_Put(str);
        capcnt = 0;
        GPIOB_PSOR |= (1 << 22);
    }
}	
#endif /* DEBUG_CAM */


/* ADC0 Conversion Complete ISR  */
void ADC0_IRQHandler() {
	// Reading ADC0_RA clears the conversion complete flag
	ADC0VAL = (uint16_t) ADC0_RA;
}

/* 
* FTM2 handles the camera driving logic
*	This ISR gets called once every integration period
*		by the periodic interrupt timer 0 (PIT0)
*	When it is triggered it gives the SI pulse,
*		toggles clk for 128 cycles, and stores the line
*		data from the ADC into the line variable
*/
void FTM2_IRQHandler(){ // For FTM timer
	// Clear interrupt
  FTM2_SC &= ~(FTM_SC_TOF_MASK);
	
	// Toggle clk
	GPIOB_PTOR = (1 << 9);
	
	// Line capture logic
	if ((pixcnt >= 2) && (pixcnt < 256)) {
		if (!clkval) {	// check for falling edge
			// ADC read (note that integer division is 
			//  occurring here for indexing the array)
			line[pixcnt/2] = ADC0VAL;
		}
		pixcnt += 1;
	} 
	else if (pixcnt < 2) {
		if (pixcnt == -1) {
			GPIOB_PSOR |= (1 << 23); // SI = 1
		} else if (pixcnt == 1) {
			GPIOB_PCOR |= (1 << 23); // SI = 0
			// ADC read
			line[0] = ADC0VAL;
		} 
		pixcnt += 1;
	} 
	else {
		GPIOB_PCOR |= (1 << 9); // CLK = 0
		clkval = 0; // make sure clock variable = 0
		pixcnt = -2; // reset counter
		line_ready = TRUE; // indicate the camera has completed a line scan
		// Disable FTM2 interrupts (until PIT0 overflows
		//   again and triggers another line capture)
		FTM2_SC &= ~FTM_SC_TOIE_MASK;
	}
}

/* PIT0 determines the integration period
*		When it overflows, it triggers the clock logic from
*		FTM2. Note the requirement to set the MOD register
* 	to reset the FTM counter because the FTM counter is 
*		always counting, I am just enabling/disabling FTM2 
*		interrupts to control when the line capture occurs
*/
void PIT0_IRQHandler() {
#ifdef DEBUG_CAM
	// Increment capture counter so that we can only 
	//	send line data once every ~2 seconds
	capcnt += 1;
#endif /* DEBUG_CAM */
	// Clear interrupt by writing a 1 to TIF bit
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK; // NOTE - channel 0
	
	// Setting mod resets the FTM counter
	FTM2->MOD = (DEFAULT_SYSTEM_CLOCK)/(100000); // NOTE: MOD = 200 b/c SYS_CLK * MOD = 10us // TODO - change this to be divided by 2 like the init code??
	
	// Enable FTM2 interrupts (camera)
	FTM2_SC |= FTM_SC_TOIE_MASK;
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
	
	// Set the period (~10us)
	FTM2->MOD = (DEFAULT_SYSTEM_CLOCK)/(100000)/2; // NOTE: MOD = 200 b/c SYS_CLK * MOD = 10us
	
	// 50% duty
	FTM2_C0V = ((DEFAULT_SYSTEM_CLOCK)/(100000))/4; // NOTE: DUTY CYCLE = MOD / 2
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
	
	// Enable hardware trigger from FTM2
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

/* Initialization of PIT timer to control 
 * integration period
 */
void PIT_Init(void){
	// Setup periodic interrupt timer (PIT)
	
	// Enable clock for timers
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	PIT_MCR &= ~(PIT_MCR_MDIS_MASK); // Enable module before pany other setup is done
	
	// Enable timers to continue in debug mode
	PIT_MCR &= ~(PIT_MCR_FRZ_MASK); // In case you need to debug
	
	// PIT clock frequency is the system clock
	// Load the value that the timer will count down from
	PIT_LDVAL0 = (unsigned int)(INTEGRATION_TIME*(float)DEFAULT_SYSTEM_CLOCK); // NOTE: channel 0, integration time (33.75 us - 100 ms)
	
	// Enable timer interrupts
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK; //NOTE: channel 0
	
	// Enable the timer
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK; //NOTE: channel 0

	// Clear interrupt by writing a 1 to TIF bit
	PIT_TFLG0 |= PIT_TFLG_TIF_MASK; // NOTE - channel 0

	// Enable PIT interrupt in the interrupt controller
	NVIC_EnableIRQ(PIT0_IRQn);
}


/* Set up pins for GPIO
 * 	PTB9 		- camera clk
 *	PTB23		- camera SI
 *	PTB22		- red LED
 */
void GPIO_Init(void){
	// Enable LED and GPIO so we can see results
	LED_Init();
	
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; // Enable clock on PORTB
	// Initialize PTB9 as an output (camera clock)
	PORTB_PCR9 = PORT_PCR_MUX(1);
	GPIOB_PDDR |= (1 << 9);
	
	// Initialize PTB23 as an output (camera SI)
	PORTB_PCR23 = PORT_PCR_MUX(1);
	GPIOB_PDDR |= (1 << 23);
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
