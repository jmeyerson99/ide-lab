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

//#define VERBOSE

// Function prototypes
void Car_Init(void);

// Car drive states
enum DRIVE_MODE {ACCELERATE, STRAIGHT, STOP, TURN_LEFT, TURN_RIGHT}; 
static enum DRIVE_MODE car_mode;

// Local data
static uint16_t smoothline[128];
static uint16_t binline[128];
static uint16_t line_data[128];

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
				UART3_Put("line_avg="); UART3_PutNumU(line_avg); UART3_Put("\r\n"); // DEBUG
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

      int adjusted_mdpt = (left_side_change_index + right_side_change_index) / 2;

#ifdef VERBOSE
			UART3_Put("left edge= "); UART3_PutNumU(left_side_change_index); UART3_Put("\r\n"); // DEBUG
			UART3_Put("right edge= "); UART3_PutNumU(right_side_change_index); UART3_Put("\r\n"); // DEBUG
			UART3_Put("adjusted midpoint= "); UART3_PutNumU(adjusted_mdpt); UART3_Put("\r\n"); // DEBUG
#endif
      // determine turning offsets based on the midpoint of left and right side change index
      double turn_percentage = 0.0;
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
        turn_percentage = (adjusted_mdpt - (64-8)) / (64-8);
        car_mode = TURN_LEFT;
#ifdef VERBOSE
				UART3_Put(" TURN LEFT \r\n"); // DEBUG
#endif
      }
      else if (adjusted_mdpt < 64) { // turn right
        turn_percentage = ((64-8) - adjusted_mdpt) / (64-8);
        car_mode = TURN_RIGHT;
#ifdef VERBOSE
				UART3_Put(" TURN RIGHT \r\n"); // DEBUG
#endif
      }
			
      // create an enum for car states based on the line data 
      // use a switch statement to control car logic
			double servo_duty;
      switch (car_mode) {
          case ACCELERATE:

          case STRAIGHT:
            Set_Servo_Position(SERVO_CENTER_DUTY_CYCLE);
            Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;

          case STOP:
						Spin_Left_Motor(0,FORWARD);
		        Spin_Right_Motor(0,FORWARD);
						break;

          case TURN_LEFT:
						servo_duty = SERVO_CENTER_DUTY_CYCLE - ((SERVO_CENTER_DUTY_CYCLE - SERVO_LEFT_MAX) * turn_percentage);
#ifdef VERBOSE
			char str[256]; // used later
			sprintf(str,"%lf\n\r",servo_duty);
			UART3_Put(str); // DEBUG
#endif
            Set_Servo_Position(SERVO_LEFT_MAX); // TODO - scaling doesn't work!!
					  Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;

          case TURN_RIGHT:
						servo_duty = ((SERVO_RIGHT_MAX - SERVO_CENTER_DUTY_CYCLE) * turn_percentage) + SERVO_CENTER_DUTY_CYCLE;
#ifdef VERBOSE 
			sprintf(str,"%lf\n\r",servo_duty);
			UART3_Put(str); // DEBUG
#endif
            Set_Servo_Position(SERVO_RIGHT_MAX); // TODO - scaling doesn't work!!
						Spin_Left_Motor(MIN_LEFT_MOTOR_SPEED,FORWARD);
		        Spin_Right_Motor(MIN_RIGHT_MOTOR_SPEED,FORWARD);
            break;
      }
    }
}
