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

#include <stdlib.h>

//#define VERBOSE

// Function prototypes
void Car_Init(void);
int process_line_data(void);
void get_calibration_values(void);

// Car drive states
enum DRIVE_MODE {ACCELERATE, STRAIGHT, STOP, TURN_LEFT, TURN_RIGHT, BRAKE}; 
static enum DRIVE_MODE car_mode;

// Local data
static uint16_t smoothline[128];
static uint16_t binline[128];
static uint16_t line_data[128];

// Calibration Data
static double kp = 0.67;
static double kd = 0.45;
static double ki = 0.15;

static double error = 0.0;
static double old_error1 = 0.0;
static double old_error2 = 0.0;

#ifdef VERBOSE
static char str[256];
#endif

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
	
		get_calibration_values();
	
	  Set_Servo_Position(SERVO_CENTER_DUTY_CYCLE);
	  Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);

    // TODO - loop forever until a switch gets pressed to start running
		
    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
      Debug_Camera();
#endif /* DEBUG_CAM */
			
			int adjusted_mdpt = process_line_data();

#ifdef VERBOSE
			//UART3_Put("left edge= "); UART3_PutNumU(left_side_change_index); UART3_Put("\r\n"); // DEBUG
			//UART3_Put("right edge= "); UART3_PutNumU(right_side_change_index); UART3_Put("\r\n"); // DEBUG
			UART3_Put("adjusted midpoint= "); UART3_PutNumU(adjusted_mdpt); UART3_Put("\r\n"); // DEBUG
#endif
      // determine turning offsets based on the midpoint of left and right side change index
      
#ifdef VERBOSE
			//UART3_Put("turn_percentage:"); sprintf(str,"%lf\n\r",turn_percentage); UART3_Put(str); // DEBUG
#endif
      if (0 == adjusted_mdpt) { // if adjusted_mdpt = 0, STOP
				car_mode = STOP;
#ifdef VERBOSE
				UART3_Put(" STOP \r\n"); // DEBUG
#endif
			} 
			else if (adjusted_mdpt > (64-8) && adjusted_mdpt < (64+8)) { // go straight
				car_mode = STRAIGHT;
#ifdef VERBOSE
				UART3_Put(" CONTINUE \r\n"); // DEBUG
#endif
			}
      else if (adjusted_mdpt > 64) { // turn left
        car_mode = TURN_LEFT;
#ifdef VERBOSE
				UART3_Put(" TURN LEFT \r\n"); // DEBUG
#endif
      }
      else if (adjusted_mdpt < 64) { // turn right
        car_mode = TURN_RIGHT;
#ifdef VERBOSE
				UART3_Put(" TURN RIGHT \r\n"); // DEBUG
#endif
      }
			
      // create an enum for car states based on the line data 
      // use a switch statement to control car logic
			
			// PID TURN LOGIC HERE
			error = 64 - adjusted_mdpt; // index error (distance from midpoint)
			double raw_servo_duty = SERVO_CENTER_DUTY_CYCLE + kp * (64 - adjusted_mdpt);
			double servo_duty = raw_servo_duty + kp*error + ki*((old_error1 + old_error2)/2.0) + kd*(error - 2*old_error1 + old_error2); // TODO - should raw_servo_duty be previous_servo_duty?
			old_error1 = error;
			old_error2 = old_error1;
			// Clip servo values
			if (servo_duty < SERVO_LEFT_MAX) {servo_duty = SERVO_LEFT_MAX;}
			if (servo_duty > SERVO_RIGHT_MAX) {servo_duty = SERVO_RIGHT_MAX;}
			
      switch (car_mode) {
					case BRAKE:
						
          case ACCELERATE:

          case STRAIGHT:
            Set_Servo_Position(servo_duty);
            Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;

          case STOP:
						Spin_Left_Motor(0,FORWARD);
		        Spin_Right_Motor(0,FORWARD);
						break;

          case TURN_LEFT:
						//turn_percentage = (SERVO_RIGHT_MAX - SERVO_LEFT_MAX)*((adjusted_mdpt + 48.0)/(128.0 + 48.0))+SERVO_LEFT_MAX;
						//turn_percentage = SERVO_CENTER_DUTY_CYCLE - (turn_percentage - SERVO_CENTER_DUTY_CYCLE); // offset comes out flipped, so swap above/below center duty cycle
            Set_Servo_Position(servo_duty); 
					  Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;

          case TURN_RIGHT:
						//turn_percentage = (SERVO_RIGHT_MAX - SERVO_LEFT_MAX)*((adjusted_mdpt - 48.0)/(128.0 - 48.0))+SERVO_LEFT_MAX;
						//turn_percentage = SERVO_CENTER_DUTY_CYCLE - (turn_percentage - SERVO_CENTER_DUTY_CYCLE); // offset comes out flipped, so swap above/below center duty cycle
            Set_Servo_Position(servo_duty); 
						Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;
      }
    }
}

int process_line_data() {
	// Get a line from the camera
	Get_Line(line_data);

	// process the line here - create binary plot
	int line_avg = 0;
	// smooth out the line using 5 point averager (edge cases, then loop)
	smoothline[0] = (line_data[0] + line_data[1] + line_data[2])/3;
	smoothline[1] = (line_data[1] + line_data[1] + line_data[2] + line_data[0])/4;
	smoothline[127] = (line_data[127] + line_data[126] + line_data[125])/3;
	smoothline[126] = (line_data[126] + line_data[125] + line_data[124] + line_data[127])/4;
	for(int i = 0; i < 128; i++) {
		if(i > 1 && i < 126){
			smoothline[i] = (line_data[i] + line_data[i+1] + line_data[i+2] + line_data[i-1] + line_data[i-2])/5;
		}
		line_avg = line_avg + line_data[i];
#ifdef VERBOSE
		//UART3_Put("line["); UART3_PutNumU(i); UART3_Put("]="); UART3_PutNumU(line_data[i]); UART3_Put("\r\n"); // DEBUG
#endif
	}
	line_avg = line_avg/128;
#ifdef VERBOSE
		//UART3_Put("line_avg="); UART3_PutNumU(line_avg); UART3_Put("\r\n"); // DEBUG
#endif

	// use smoothline to make binary plot
	for(int i = 0; i < 128; i++){
		//binline[i] = smoothline[i] > line_avg ? 1 : 0; // TODO - MAKE THIS WORK
		binline[i] = smoothline[i] > 36000 ? 1 : 0; // TODO - remove the hard coded threshold
#ifdef VERBOSE
		//UART3_Put("bin["); UART3_PutNumU(i); UART3_Put("]="); UART3_PutNumU(binline[i]); UART3_Put("\r\n"); // DEBUG
#endif
	}
	// find the switching indices
	int left_side_change_index = 0;
	int right_side_change_index = 0;
	for (int i = 1; i < 128; i++) {
		if (binline[i-1] == 0 && binline[i] == 1) {left_side_change_index = i;break;}
	}
	for (int i = 127; i > 1; i--) {
		if (binline[i] == 0 && binline[i-1] == 1) {right_side_change_index = i;break;}
	}

	return (left_side_change_index + right_side_change_index) / 2; // return the adjusted mdpt of the data
}

void get_calibration_values() {
	char input[64];
	char *ptr; // used just to fill an arugment for strtod
	UART3_Put("kp=");
	UART3_GetString(input);
	kp = strtod(input, &ptr); // string to double in stdlib.h
	
	UART3_Put("kd=");
	UART3_GetString(input);
	kd = strtod(input, &ptr);
	
	UART3_Put("ki=");
	UART3_GetString(input);
	ki = strtod(input, &ptr);
}
