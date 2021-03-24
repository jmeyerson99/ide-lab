#ifndef PWM_H_
#define PWM_H_

/* DC Motor Direction ENUM */
typedef enum {FORWARD=0, REVERSE=1} DC_Motor_Direction;

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
