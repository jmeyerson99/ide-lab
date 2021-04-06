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

    // TODO - loop forever until a switch gets pressed to start running
		
    // Main loop below
    //  Read the camera data, process the line array. Based on that, turn the servo and spin the DC motors
    while (1) { 
#ifdef DEBUG_CAM
      Debug_Camera();
#endif /* DEBUG_CAM */
			
			// Get a line from the camera
			while (1) {
				if(Get_Line(line_data)) {break;}
			}
			UART0_Put("Do I ever get here?"); // DEBUG
      // process the line here - create binary plot
      double line_avg = 0;
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
				UART0_Put("Line_data["); UART0_PutNumU(i); UART0_Put("] = "); UART0_PutNumU(line_data[64]); UART0_Put("\r\n"); // DEBUG
      }
      line_avg = line_avg/128;

      // use smoothline to make binary plot
      for(int i = 0; i < 128; i++){
        binline[i] = line_avg < smoothline[i] ? 1 : 0;
      }
      // find the switching indices
      int left_side_change_index = 0;
      int right_side_change_index = 0;
      for (int i = 1; i < 128; i++) {
        if (line_data[i-1] == 0 && line_data[i] == 1) {right_side_change_index = i;break;}
      }
      for (int i = 127; i < 1; i--) {
        if (line_data[i] == 1 && line_data[i+1] == 0) {left_side_change_index = i;break;}
      }

      int ajdusted_mdpt = (left_side_change_index + right_side_change_index) / 2;
      
      // if adjusted_mdpt = 0, STOP
      if (0 == ajdusted_mdpt) {car_mode = STOP;} 

      // determine turning offsets based on the midpoint of left and right side change index
      double turn_percentage = 0.0;
      if (ajdusted_mdpt > 64) {
        turn_percentage = (ajdusted_mdpt - 64) / 64;
        car_mode = TURN_LEFT;
        // turn left
      }
      if (ajdusted_mdpt < 64) {
        turn_percentage = (64 - ajdusted_mdpt) / 64;
        car_mode = TURN_RIGHT;
        // turn right
      }

      // create an enum for car states based on the line data 
      // use a switch statement to control car logic
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
            Set_Servo_Position(SERVO_LEFT_MAX * turn_percentage); // TODO - this might be wrong
            break;

          case TURN_RIGHT:
            Set_Servo_Position(SERVO_RIGHT_MAX * turn_percentage); // TODO - this might be wrong
            break;
      }

    }
}
