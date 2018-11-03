#include "signal.h"
#include <math.h>
#include <stdio.h>

#define PI (3.14159265f)

//默认参数
float default_freq = 1000.0f;
float default_pp = 1.0f;
float default_bias = 0.0f;
float default_duty = 50.0f;
float default_phase = 0.0f;

void siganal_CFG(signal_TypeDef *signal, signalType type, float *out, float *freq, float *pp, float *bias, float *duty, float* phase, float dt)
{
	signal->out = out;
	if(freq!=NULL){
		signal->freq = freq;
	}else{
		signal->freq = &default_freq;
	}
	
	if(pp!=NULL){
		signal->pp = pp;
	}else{
		signal->pp = &default_pp;
	}
	
	if(duty!=NULL){
		signal->duty = duty;
	}else{
		signal->duty = &default_phase;
	}
	
	if(phase!=NULL)
	{
		signal->phase = phase;
	}else{
		signal->phase = &default_phase;
	}
	
	if(bias!=NULL)
	{
		signal->bias = bias;
	}else{
		signal->bias = &default_bias;
	}
	
	signal->n = 0;
	signal->dt = dt;
	signal->type = type;
}

float signal_Output(signal_TypeDef *signal)
{
	float (*func)(float) = NULL;
	float Amp,k;
	u32 N = 1000.0f/((*signal->freq)*(signal->dt));
	//printf(" N:%d n:%d\r\n",N,signal->n);
	if(signal->type == 0){//正弦波
		func = sinf;
	}else if(signal->type == 1){//方波
		func = rectf;
	}else if(signal->type == 2){//三角波
		func = trianglef;	
	}
	
	if(func != NULL){
		Amp = (*(signal->pp))/2.0f;
		k = 2*PI*(*signal->freq)*(signal->dt)/1000.0f;
		*(signal->out) = Amp * func(k*(signal->n)  + (*signal->phase)) + (*signal->bias);
		//printf(" bias:%f \r\n",*signal->bias);
		(signal->n)++;
		(signal->n) %= N;	
	}
	return *(signal->out);
}

static float rectf(float t)
{
	float u;
	if(t<=PI){
		u = 1.0f;
	}else{
		u = -1.0f;
	}
	return u;
}

static float trianglef(float t)
{
	float u;
	if(t<=PI/2.0f){
		u = 2.0f*t/PI;
	}else if(t>PI/2.0f && t<3.0f*PI/2.0f){
		u = 2.0f - 2.0f*t/PI;
	}else{
		u = 2.0f*t/PI - 4.0f;
	}
	
	return u;
}
