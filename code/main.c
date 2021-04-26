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

#define TRACK_MIDPOINT 65.5 // index that we center the array around
static unsigned int HARD_TURN_OFFSET = 9; // how far the error from the center is to turn hard

// Function prototypes
void Car_Init(void);
int process_line_data(void);
void get_calibration_values(void);

// Car drive states
enum DRIVE_MODE {ACCELERATE, STRAIGHT, STOP, HARD_LEFT, HARD_RIGHT, SLIGHT_LEFT, SLIGHT_RIGHT, BRAKE}; 
static enum DRIVE_MODE car_mode;
static enum DRIVE_MODE previous_mode;

// Local data
static uint16_t smoothline[128];
static uint16_t binline[128];
static uint16_t line_data[128];

// Motor speed constants
static unsigned int MIN_MOTOR_SPEED = 75; // was 50 // 60
static unsigned int MAX_MOTOR_SPEED = 100; // was 70 // 80

static unsigned int SLIGHT_TURN_PERCENTAGE = 90; // was 80 // 80
static unsigned int HARD_TURN_PERCENTAGE = 10;   // was 60 // 60

// Calibration Data for PID
static double kp = 0.13; //.2 ideal for 50%/70% duty cycle (min/max)  // 0.15 ideal for 60%/80% duty cycle (min/max)
static double kd = 0.75; //.52 ideal for 50%/70% duty cycle (min/max) // 0.65 ideal for 60%/80% duty cycle (min/max)
static double ki = 0.0;  //0 ideal for 50%/70% duty cycle (min/max)   // 0 ideal for 60%/80% duty cycle (min/max)

static double error = 0.0;
static double old_error1 = 0.0;
static double old_error2 = 0.0;

static double previous_servo_duty = SERVO_CENTER_DUTY_CYCLE;
static unsigned int current_motor_speed = 0;

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
	
		//Calibrate
		/*
		char input[64];
		char str[64];
		char *ptr; // used just to fill an arugment for strtod
		UART3_GetString(input);
		kd = strtod(input, &ptr); //strtoul(input, &ptr, 10); // // string to double in stdlib.h
		UART3_Put("kd:"); sprintf(str,"%lf\n\r", kd); UART3_Put(str); // DEBUG
		*/
	
}


int main(void) {
    Car_Init();
		
		current_motor_speed = MIN_MOTOR_SPEED;
	
	  Set_Servo_Position(SERVO_CENTER_DUTY_CYCLE);
	  Spin_Left_Motor(current_motor_speed,FORWARD);
		Spin_Right_Motor(current_motor_speed,FORWARD);

    // TODO - loop forever until a switch gets pressed to start running
		
    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
      Debug_Camera();
#endif /* DEBUG_CAM */
			
			int adjusted_mdpt = process_line_data();

#ifdef VERBOSE
			//UART3_Put("adjusted midpoint= "); UART3_PutNumU(adjusted_mdpt); UART3_Put("\r\n"); // DEBUG
#endif
      // determine turning offsets based on the midpoint of left and right side change index
      
      if (0 == adjusted_mdpt) { // if adjusted_mdpt = 0, STOP
				car_mode = STOP;
			} 
			else if (adjusted_mdpt > (TRACK_MIDPOINT-8) && adjusted_mdpt < (TRACK_MIDPOINT+8)) { // go straight // TODO - calibrate range for straight
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
			
			// PID TURN LOGIC HERE
			double raw_servo_duty = SERVO_CENTER_DUTY_CYCLE + kp*(TRACK_MIDPOINT - adjusted_mdpt); // adjust the midpoint so we don't over or under turn (especially on left hand turns!!)
			error = raw_servo_duty - previous_servo_duty;
			double servo_duty = previous_servo_duty + kp * error + ki*((old_error1 + old_error2)/2.0) + kd*(error - 2*old_error1 + old_error2);
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
					case BRAKE:
						
          case ACCELERATE:
						Set_Servo_Position(servo_duty);
						current_motor_speed = current_motor_speed + 3; // TODO - calibrate this
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
					// TODO - determine if the turning percentages hold up for higher speeds
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
						Spin_Left_Motor(current_motor_speed,FORWARD);
		        Spin_Right_Motor(current_motor_speed *(HARD_TURN_PERCENTAGE/100.0),REVERSE); 
            break;
      }
			previous_mode = car_mode;
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
		//binline[i] = smoothline[i] > line_avg ? 1 : 0; // TODO - doesnt do carpet detection
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
