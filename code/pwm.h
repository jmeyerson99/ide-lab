#ifndef PWM_H_
#define PWM_H_

/* DC Motor Direction ENUM */
typedef enum {FORWARD=0, REVERSE=1} DC_Motor_Direction;

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK    20485760u 
#define SERVO_FREQUENCY         50u    // 50 Hz
#define DC_MOTOR_FREQUENCY      10000u // 10 kHz
#define FTM0_MOD_VALUE          (DEFAULT_SYSTEM_CLOCK/DC_MOTOR_FREQUENCY)
#define FTM3_MOD_VALUE          ((DEFAULT_SYSTEM_CLOCK/SERVO_FREQUENCY/128) - 1) //NOTE: 50Hz

#define SERVO_CENTER_DUTY_CYCLE 6.6 // 6.6% duty cycle centers the steering
#define SERVO_LEFT_MAX          4.9 // 4.9% duty cycle is a hard left
#define SERVO_RIGHT_MAX         8.3 // 8.3% duty cycle is a hard right

// Function prototypes
void PWM_Init(void);
void Servo_Init(void);
void FTM0_PWM_Init(void);
void FTM3_PWM_Init(void);
void Spin_Left_Motor(unsigned int duty_cycle, DC_Motor_Direction dir);
void Spin_Right_Motor(unsigned int duty_cycle, DC_Motor_Direction dir);
void Set_Servo_Position(double duty_cycle);
void FTM3_Set_Duty_Cycle(double duty_cycle);
void FTM3_IRQHandler(void);
void EN_init(void);

#endif /* PWM_H_ */
