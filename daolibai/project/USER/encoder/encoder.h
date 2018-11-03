#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"

//此代码必须使用32位定时器（TIM2或TIM5）

//编码器为512线
//减速箱为64/1

//TIM2为编码器记数，
//PB10 PB11 作为编码器A B输入



#define LINE_NUM 2000   //编码器线数
#define GEARBOX  64 //减速比


typedef struct{
	s8 derc;//旋转方向
	float rpm;//旋转速度
	float deg;//当前绝对角度
	float deg_last;
	float deg_rel;//以开机时为0°的相对角度
	float deg_rel_last;//上次相对角度，用于计算速度
}EncoderTypeDef;

extern  EncoderTypeDef encoder0;
extern  EncoderTypeDef encoder1;


void encoder_Init(float dt);//外设引脚初始化

void encoder_Update(void);//数据读取

#endif

