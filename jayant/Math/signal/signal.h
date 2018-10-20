#ifndef __SIGNAL_H
#define __SIGNAL_H 

#include "stm32f4xx.h"

typedef enum{
	sine     = 0,//正弦波(cos(wt))
	rect     = 1,//方波
	triangle = 2//三角波
}signalType;

typedef struct{
	u32 n;//时基
	float dt;//采样时间
	float *freq;//频率
	float *pp;//峰峰值
	float *bias;//直流偏置
	float *duty;//占空比
	float *phase;//相位
	float *out;//当前输出
	signalType type;
}signal_TypeDef;

void siganal_CFG(signal_TypeDef *signal, signalType type, float *out, float *freq, float *pp, float *bias, float *duty, float* phase, float dt);

float signal_Output(signal_TypeDef *signal);


//附加函数。方波与三角波
static float rectf(float);//小于pi为1，大于pi为-1

static float trianglef(float); 

#endif

