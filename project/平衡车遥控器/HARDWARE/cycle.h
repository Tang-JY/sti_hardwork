#ifndef __CYCLE_H
#define __CYCLE_H

#include  "stm32f4xx.h"
#include  "stm32f4xx_tim.h"

extern vu8 cycle_flag;

void cycle_Init(u8);//定时器控制周期(ms)

#endif

