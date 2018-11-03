#ifndef __FH_MOTOR_H
#define __FH_MOTOR_H

#include "stm32f4xx.h"
#include "pwm.h"


//初始化
void FH_Motor_Init(u32 freq,float min,float max);

//设置pwm值，可以为负，负数表示反向转
void FH_Motor_SetPWM(float pwm0,float pwm1);

#endif
