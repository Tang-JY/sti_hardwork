#ifndef __CYCLE_H
#define __CYCLE_H

#include  "stm32f4xx.h"
#include  "stm32f4xx_tim.h"

extern vu8 cycle_flag;

void cycle_Init(float dt);//定时器控制周期(ms)

void timeMaster_setInt(int* data, const int new_data);
void timeMaster_setFloat(float *data,const float new_data);
void timeMaster_setByte(char *data,const char new_data);
void timeMaster_memcpy(void *data,const void* new_data,unsigned int n);


#endif

