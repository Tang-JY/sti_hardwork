#ifndef __FH_MOTOR_H
#define __FH_MOTOR_H

#include "stm32f4xx.h"
#include "pwm.h"


//��ʼ��
void FH_Motor_Init(u32 freq,float min,float max);

//����pwmֵ������Ϊ����������ʾ����ת
void FH_Motor_SetPWM(float pwm0,float pwm1);

#endif
