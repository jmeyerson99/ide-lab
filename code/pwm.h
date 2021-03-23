#ifndef PWM_H_
#define PWM_H_

// Function prototypes
void PWM_Init(void);
void Servo_Init(void);
void FTM0_PWM_Init(void);
void FTM3_PWM_Init(void);
void FTM0_Set_Duty_Cycle(unsigned int duty_cycle, unsigned int frequency, int dir);
void FTM3_Set_Duty_Cycle(float duty_cycle);
void FTM3_IRQHandler(void);
void EN_init(void);

/* DC Motor Direction ENUM */
typedef enum {FORWARD=0, REVERSE=1} DC_Motor_Direction;

#endif /* PWM_H_ */
