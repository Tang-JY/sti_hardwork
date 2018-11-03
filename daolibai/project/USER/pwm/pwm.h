#ifndef __PWM_H
#define __PWM_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

void PWM_Init(float freq,float min,float max);
//PWM初始化函数，入口参数为PWM波频率及上下限辐,单位Hz(频率不超过1M)

u8 Set_PWM_DutyRatio(u8 channel,float duty_ratio);
//占空比设置函数，入口参数为引脚通道数及占空比（0~100）
//TIM3通道1 ： PA6
//TIM3通道2 ： PA7
//TIM3通道3 ： PB0
//TIM3通道4 ： PB1

#endif


