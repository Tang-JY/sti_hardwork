#ifndef __S_ENCODER_H
#define __S_ENCODER_H

#include "stm32f4xx.h"

//此编码器不用于计算速度，仅用于手动输入设备的编码器

typedef struct{
	float *data;//被控数据
	float dx;//每次编码器有脉冲时给与的数据增量
	float range[2];//数据被修改的范围
}SoftEncoder_TypeDef;


//encoder0 使用TIM2(A5 B3),开关引脚C0
//encoder1 使用TIM2(A0 A1),开关引脚B11


void softEncoder_Init(void);

void softEncoder_CFG(SoftEncoder_TypeDef *encoder,float *data, float min, float max);//数据结构体配置

void softEncoder_Binding(u8 encoder, SoftEncoder_TypeDef* data);//将数据绑定到编码器，编码器转动时修改对应数据
//参数：编码器编号0/1（编码器0使用PG6 PB3 PB4）  ,对应数据结构体

u8 softEncoder_GetKeyState(u8 encoder);//获得编码器上开关的状态

void softEncoder_UpdateEncoderState(void);//更新编码器状态

#endif

