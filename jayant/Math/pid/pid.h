#ifndef __PID_H
#define __PID_H

//微分先行PID控制器，微分环节带低通滤波


//PID控制器
typedef struct {
	float dt; //循环周期(ms)
	float *kp;
	float *ki;
	float *kd;
	float *target;//目标值存储指针
	float *feedback;//当前值存储指针

	float f_H;//微分先行控制，低通滤波器上限频率

	float e[2];//本次误差与上次误差
	float feedback_LPF[2];//滤波后的本次目标值与上次目标值
	float ei;//误差积分值
	float fd;//反馈微分值
	float e_limit;//误差限辐（用于积分分离，可变kp参数和可变kd参数）
	float ei_limit;//积分限幅
	float u;//输出值
}PID_TypeDef;


void pid_params_CFG(PID_TypeDef *pid, float *kp, float *ki,float *kd, float dt);//pid可调参数配置

void pid_limits_CFG(PID_TypeDef *pid, float e_limit, float i_limit, float f_H);//pid非可调参数配置

void pid_data_CFG(PID_TypeDef *pid, float *target, float *feedback);//pid数据指针配置。

float pid_Calc(PID_TypeDef *pid);//pid计算



#endif 
