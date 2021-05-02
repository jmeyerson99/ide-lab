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
#include "switch.h"

#include <stdlib.h>

//#define VERBOSE
//#define FINISH_LINE_STOP
//#define BLUETOOTH_CALIBRATE

// Camera constants
#define TRACK_MIDPOINT 65.5 // index that we center the array around
#define HARD_TURN_OFFSET 9  // how far the error from the center is to turn hard
#define MAX_CAMERA_VALUE_THRESHOLD_PERCENTAGE 80 // percentage of max camera value to be considered "track"

// Constant arrays for speed modes
#define NUMBER_OF_MODES 3
static unsigned int min_motor_speeds[] = {75, 85, 90};
static unsigned int max_motor_speeds[] = {85, 95, 100};
static unsigned int slight_turn_percentages[] = {90, 90, 90};
static unsigned int hard_turn_percentages[] = {25, 10, 5};
static double kps[] = {0.13, 0.115, .10};
static double kis[] = {0.0, 0.0, 0.0};
static double kds[] = {0.75, 0.75, 0.79};
static unsigned char LED_colors[] = {'g', 'y', 'm'};

// Function prototypes
void Car_Init(void);
int Process_Line_Data(void);
#ifdef FINISH_LINE_STOP
void Stop_Car(void);
#endif

// Car drive states
enum DRIVE_MODE {ACCELERATE, STRAIGHT, STOP, HARD_LEFT, HARD_RIGHT, SLIGHT_LEFT, SLIGHT_RIGHT}; 
static enum DRIVE_MODE car_mode;
static enum DRIVE_MODE previous_mode;

// Local data for the camera line
static uint16_t smoothline[128];
static uint16_t binline[128];
static uint16_t line_data[128];

// Motor speed constants (default values for slow mode)
static unsigned int MIN_MOTOR_SPEED = 75;
static unsigned int MAX_MOTOR_SPEED = 85;
#define ACCELERATION_FACTOR 5 // how much to increase the speed when accelerating

// Motor turning percentage constants (default values for slow mode)
static unsigned int SLIGHT_TURN_PERCENTAGE = 90; // percentage of current speed
static unsigned int HARD_TURN_PERCENTAGE = 25;   // percentage of current speed

// Calibration Data for PID (default values for slow mode)
static double kp = 0.13; 
static double kd = 0.75; 
static double ki = 0.0;

// PID Error Variables
static double error = 0.0;
static double old_error1 = 0.0;
static double old_error2 = 0.0;

// Motor Duty Cycle/Speed Variables
static double previous_servo_duty = SERVO_CENTER_DUTY_CYCLE;
static unsigned int current_motor_speed = 0;

#ifdef VERBOSE
static char str[256];
#endif

void Car_Init() {
		__asm("CPSID I");
    UART0_Init();   // for serial
		Camera_Init();  // for camera
    UART3_Init();   // for bluetooth
    PWM_Init();     // for motors
    Servo_Init();   // for servo
    LED_Init();     // for on board LED
		EN_init();		  // for motor enable
		Switch2_Init(); // for switch 2
		Switch3_Init(); // for switch 3
	__asm("CPSIE I");
	
		// Bluetooth Calibrate
#ifdef BLUETOOTH_CALIBRATE
		char input[64];
		char str[64];
		char *ptr; // used just to fill an arugment for strtod
		UART3_GetString(input);
		kd = strtod(input, &ptr); //strtoul(input, &ptr, 10); // // string to double in stdlib.h
		UART3_Put("kd:"); sprintf(str,"%lf\n\r", kd); UART3_Put(str); // DEBUG
#endif
}

int main(void) {
		double raw_servo_duty;
		double servo_duty;
		int adjusted_mdpt;
		int mode_select;
	
    Car_Init();
		
		// Get the speed mode from the user based on the switches
		mode_select = 0;
		while (!Switch2_Pressed()) {
			if (Switch3_Pressed()) {
				LED_Off();
				mode_select++;
				LED_On(LED_colors[mode_select % NUMBER_OF_MODES]);
				Delay(); // delay for ~2 seconds 
			}
		}
		 
		// Assign constants based on mode
		kp = kps[mode_select % NUMBER_OF_MODES];
		ki = kis[mode_select % NUMBER_OF_MODES];
		kd = kds[mode_select % NUMBER_OF_MODES];
		MIN_MOTOR_SPEED = min_motor_speeds[mode_select % NUMBER_OF_MODES];
		MAX_MOTOR_SPEED = max_motor_speeds[mode_select % NUMBER_OF_MODES];
		SLIGHT_TURN_PERCENTAGE = slight_turn_percentages[mode_select % NUMBER_OF_MODES];
		HARD_TURN_PERCENTAGE = hard_turn_percentages[mode_select % NUMBER_OF_MODES];
		RIGHT_TURN_OFFSET = right_turn_offsets[mode_select % NUMBER_OF_MODES];
		
		// START YOUR ENGINES
		
		current_motor_speed = MIN_MOTOR_SPEED;

	  Set_Servo_Position(SERVO_CENTER_DUTY_CYCLE);
	  Spin_Left_Motor(current_motor_speed,FORWARD);
		Spin_Right_Motor(current_motor_speed,FORWARD);
		
    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
      Debug_Camera();
#endif /* DEBUG_CAM */
			
			adjusted_mdpt = Process_Line_Data();

#ifdef VERBOSE
			//UART3_Put("adjusted midpoint= "); UART3_PutNumU(adjusted_mdpt); UART3_Put("\r\n"); // DEBUG
#endif
      // determine turning offsets based on the midpoint of left and right side change index
      
			// Turn off Carpet Detection
      /* if (0 == adjusted_mdpt) { // if adjusted_mdpt = 0, STOP
				car_mode = STOP;
			} */
			if (adjusted_mdpt > (TRACK_MIDPOINT-8) && adjusted_mdpt < (TRACK_MIDPOINT+8)) { // go straight // TODO - calibrate range for straight
				if (previous_mode == STRAIGHT || previous_mode == ACCELERATE) { car_mode = ACCELERATE; }
				else {car_mode = STRAIGHT; }
			}
      else if (adjusted_mdpt > TRACK_MIDPOINT) { // turn left
				if (adjusted_mdpt > TRACK_MIDPOINT + HARD_TURN_OFFSET) {car_mode = HARD_LEFT; }
				else {car_mode = SLIGHT_LEFT; }
      }
      else if (adjusted_mdpt < TRACK_MIDPOINT) { // turn right
				if (adjusted_mdpt < TRACK_MIDPOINT - HARD_TURN_OFFSET) {car_mode = HARD_RIGHT; }
				else {car_mode = HARD_RIGHT; }
      }			
			
			// PID TURN LOGIC
			raw_servo_duty = SERVO_CENTER_DUTY_CYCLE + kp*(TRACK_MIDPOINT - adjusted_mdpt); // adjust the midpoint so we don't over or under turn (especially on left hand turns!!)
			error = raw_servo_duty - previous_servo_duty;
			servo_duty = previous_servo_duty + kp * error + ki*((old_error1 + old_error2)/2.0) + kd*(error - 2*old_error1 + old_error2);
#ifdef VERBOSE
			//UART3_Put("previous duty:"); sprintf(str,"%lf\n\r",previous_servo_duty); UART3_Put(str); // DEBUG
			//UART3_Put("servo duty:"); sprintf(str,"%lf\n\r",servo_duty); UART3_Put(str); // DEBUG
			//UART3_Put("error:"); sprintf(str,"%lf\n\r",error); UART3_Put(str); // DEBUG
#endif
			// Clip servo values
			if (servo_duty < SERVO_LEFT_MAX) {servo_duty = SERVO_LEFT_MAX;}
			if (servo_duty > SERVO_RIGHT_MAX) {servo_duty = SERVO_RIGHT_MAX;}
			
			old_error1 = error;
			old_error2 = old_error1;
			previous_servo_duty = servo_duty;
			
      switch (car_mode) {				
          case ACCELERATE:
						Set_Servo_Position(servo_duty);
						current_motor_speed = current_motor_speed + ACCELERATION_FACTOR;
						if (current_motor_speed > MAX_MOTOR_SPEED) {current_motor_speed = MAX_MOTOR_SPEED;}
            Spin_Left_Motor(current_motor_speed,FORWARD); 
		        Spin_Right_Motor(current_motor_speed,FORWARD); 
            break;

          case STRAIGHT:
            Set_Servo_Position(servo_duty);
            Spin_Left_Motor(current_motor_speed,FORWARD);
		        Spin_Right_Motor(current_motor_speed,FORWARD);
            break;

          case STOP:
						Spin_Left_Motor(0,FORWARD);
		        Spin_Right_Motor(0,FORWARD);
						break;
					
          case HARD_LEFT:
            Set_Servo_Position(servo_duty); 
					  Spin_Left_Motor((current_motor_speed * (HARD_TURN_PERCENTAGE/100.0)),REVERSE); 
		        Spin_Right_Motor(current_motor_speed,FORWARD);
            break;
					
					case SLIGHT_LEFT:
						Set_Servo_Position(servo_duty); 
						Spin_Left_Motor((current_motor_speed * (SLIGHT_TURN_PERCENTAGE/100.0)),FORWARD); 
		        Spin_Right_Motor(current_motor_speed,FORWARD);
						break;
					
					case SLIGHT_RIGHT:
						Set_Servo_Position(servo_duty); 
						Spin_Left_Motor(current_motor_speed, FORWARD);						
						Spin_Right_Motor(current_motor_speed * (SLIGHT_TURN_PERCENTAGE/100.0), FORWARD);
						break;

          case HARD_RIGHT:
            Set_Servo_Position(servo_duty); 
						Spin_Left_Motor(current_motor_speed, FORWARD);
		        Spin_Right_Motor(current_motor_speed *((HARD_TURN_PERCENTAGE)/100.0),REVERSE); 
            break;
      }
			previous_mode = car_mode;
    }
}

int Process_Line_Data() {
	int line_avg;
	int i;
	int left_side_change_index;
	int right_side_change_index;
	uint16_t max_value;
#ifdef FINISH_LINE_STOP
		unsigned int threshold_changes;
#endif
	// Get a line from the camera
	Get_Line(line_data);

	// process the line here - create binary plot
	line_avg = 0;
	// smooth out the line using 5 point averager (edge cases, then loop)
	smoothline[0] = (line_data[0] + line_data[1] + line_data[2])/3;
	smoothline[1] = (line_data[1] + line_data[1] + line_data[2] + line_data[0])/4;
	smoothline[127] = (line_data[127] + line_data[126] + line_data[125])/3;
	smoothline[126] = (line_data[126] + line_data[125] + line_data[124] + line_data[127])/4;
	for(i = 0; i < 128; i++) {
		if(i > 1 && i < 126){
			smoothline[i] = (line_data[i] + line_data[i+1] + line_data[i+2] + line_data[i-1] + line_data[i-2])/5;
		}
		line_avg = line_avg + line_data[i];
		if (smoothline[i] > max_value) {max_value = smoothline[i];} // find the max value in the array
#ifdef VERBOSE
		UART3_Put("line["); UART3_PutNumU(i); UART3_Put("]="); UART3_PutNumU(line_data[i]); UART3_Put("\r\n"); // DEBUG
#endif
	}
	line_avg = line_avg/128;
#ifdef VERBOSE
		UART3_Put("line_avg="); UART3_PutNumU(line_avg); UART3_Put("\r\n"); // DEBUG
#endif
	
	// use smoothline to make binary plot
	for(i = 0; i < 128; i++){
		//binline[i] = smoothline[i] > (1.5 * line_avg) ? 1 : 0; // TODO - mess around with threshold? Maybe do (0.8 * MAX)
		binline[i] = smoothline[i] > ((MAX_CAMERA_VALUE_THRESHOLD_PERCENTAGE/100.0) * max_value) ? 1 : 0; // IDEAL
		//binline[i] = smoothline[i] > 19000 ? 1 : 0; // TODO - remove the hard coded threshold
#ifdef VERBOSE
		//UART3_Put("bin["); UART3_PutNumU(i); UART3_Put("]="); UART3_PutNumU(binline[i]); UART3_Put("\r\n"); // DEBUG
#endif
	}
#ifdef FINISH_LINE_STOP
	// Determine if on the finish line
	threshold_changes = 0;
	for (i = 1; i < 127; i++) {
			if (binline[i-1] == 0 && binline[i] == 1) {
				threshold_changes++;
			}
			else if (binline[i] == 0 && binline[i+1] == 1) {
				threshold_changes++;
			}
	}
#ifdef VERBOSE
		UART3_Put("threshold_changes="); UART3_PutNumU(threshold_changes); UART3_Put("\r\n"); // DEBUG
#endif
	// epect 6 edges on starting line
	if (4 < threshold_changes) {Stop_Car();}
#endif
	// find the switching indices
	left_side_change_index = 0;
	right_side_change_index = 0;
	for (i = 1; i < 128; i++) {
		if (binline[i-1] == 0 && binline[i] == 1) {left_side_change_index = i;break;}
	}
	for (i = 127; i > 1; i--) {
		if (binline[i] == 0 && binline[i-1] == 1) {right_side_change_index = i;break;}
	}

	return (left_side_change_index + right_side_change_index) / 2; // return the adjusted mdpt of the data
}

#ifdef FINISH_LINE_STOP
void Stop_Car() {
	NVIC_DisableIRQ(FTM2_IRQn);
	Set_Servo_Position(SERVO_CENTER_DUTY_CYCLE); 
	Spin_Left_Motor(0,FORWARD);
	Spin_Right_Motor(0,FORWARD); 
	while (1) {}
}
#endif
