#ifndef __PWM_H
#define __PWM_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

void PWM_Init(float freq,float min,float max);
//PWM��ʼ����������ڲ���ΪPWM��Ƶ�ʼ������޷�,��λHz(Ƶ�ʲ�����1M)

u8 Set_PWM_DutyRatio(u8 channel,float duty_ratio);
//ռ�ձ����ú�������ڲ���Ϊ����ͨ������ռ�ձȣ�0~100��
//TIM3ͨ��1 �� PA6
//TIM3ͨ��2 �� PA7
//TIM3ͨ��3 �� PB0
//TIM3ͨ��4 �� PB1

#endif


