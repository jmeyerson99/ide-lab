#ifndef PWM_H_
#define PWM_H_

void FTM0_PWM_Init(void);
void FTM3_PWM_Init(void);
void FTM0_set_duty_cycle(unsigned int duty_cycle, unsigned int frequency, int dir);
void FTM3_set_duty_cycle(float duty_cycle);
void FTM3_IRQHandler(void);
void EN_init(void);

#endif /* PWM_H_ */
