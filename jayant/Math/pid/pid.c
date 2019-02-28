
#include <stdio.h>
#include <math.h>
#include "stm32f4xx.h"

#include "pid.h"

//pid参数配置
void pid_params_CFG(PID_TypeDef *pid, float *kp, float *ki, float *kd, float dt)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	pid->dt = dt;
    pid->e[0] = 0;
    pid->e[1] = 0;
    pid->feedback_LPF[0] = 0;
    pid->feedback_LPF[1] = 0;
    pid->ei = 0;
    pid->fd = 0;
    pid->u = 0;
}

//pid限辐配置
void pid_limits_CFG(PID_TypeDef *pid, float e_limit, float i_limit, float f_H)
{
	pid->ei_limit = i_limit;
	pid->e_limit = e_limit;
	pid->f_H = f_H;
}

//pid数据配置
void pid_data_CFG(PID_TypeDef *pid, float *target, float *feedback)
{
    pid->target = target;
    pid->feedback = feedback;  
}

//pid计算
float pid_Calc(PID_TypeDef *pid)
{
    float P,I,D;//实际的PID系数
    volatile float temp[3];//中间变量
    float feedback;
    
	feedback = *(pid->feedback);
	
    pid->e[1] = pid->e[0];//保存上次误差值
	pid->e[0] = *(pid->target) - feedback;
    
    //反馈值低通滤波(滤波结果只给D，减小高频影响；但不给P,I，防止响应变慢)
    pid->feedback_LPF[1] = pid->feedback_LPF[0];//保存上次滤波值
    temp[0] = 1.0f + 6.283185f*(pid->f_H)*pid->dt/1000.0f;//计算滤波器分母
	temp[1] = (temp[0] - 1.0f)/temp[0];
	temp[2] = 1/temp[0];
    pid->feedback_LPF[0] = feedback*temp[1] + pid->feedback_LPF[1]*temp[2];

	//计算非线性P系数
    //temp[0] = pid->e[0]/(1.0f*pid->e_limit);
    //temp[1] = temp[0]*temp[0]*temp[0]*temp[0] + 1.0f;   
    //temp[2] = 1.0f - 0.7f/temp[1];
    //P = temp[2]*(*(pid->kp));//非线性P系数 函数P = (1.0 - 0.7/(1+x^4))*kp,其中x = e/e_limit;
	P = *(pid->kp);//不再使用非线性PID

	//计算非线性D系数
	//if(pid->feedback[0] > pid->feedback[1]){//若反馈值在增长，则越靠近目标（e越小），D系数应当越大，D是减函数；若超调(e更小，变为负数)，D应当更大，防止过冲		
	//	if(pid->e[0] > -1.0f*pid->e_limit){
	//		D = 0.6f*(*(pid->kd))/(1.f + 0.4f*temp[0]);//非线性D系数 当e=0时D=0.6*kd;当e = -1*e_limit时，D=kd;
	//	}else{
	//		D = *(pid->kd);
	//	}
	//}else{//若反馈值在减少，则越靠近目标(e越大)，D系数应当越大，D是增函数；若超调(e更大，变为正数)，D应当更大，防止过冲
	//	if(pid->e[0] < 1.0f*pid->e_limit){
	//		D = 0.6f*(*(pid->kd))/(1.f - 0.4f*temp[0]);
	//	}else{
	//		D = *(pid->kd);
	//	}
	//	
	//}
	D = *(pid->kd);//不再使用非线性PID

	//积分分离
    if(pid->e[0] > pid->e_limit || pid->e[0] < -1.0f * pid->e_limit){
        I = 0;
		pid->ei = 0.0f;
    }else{
        I = *(pid->ki);
		pid->ei += 0.5f*(pid->e[0] + pid->e[1]) * pid->dt / 1000.0f;//梯形积分
	}
    
	//积分限幅（抗积分饱和）
    if(pid->ei >= pid->ei_limit ){
        pid->ei = pid->ei_limit;
    }else if(pid->ei <= -1.0f*pid->ei_limit){
        pid->ei = -1.0f*pid->ei_limit;
    }
	
    //微分计算（微分先行且使用滤波结果）
	pid->fd = 1000.0f*(pid->feedback_LPF[0] - pid->feedback_LPF[1])/pid->dt;

    pid->u = P*pid->e[0] + I*pid->ei - D*pid->fd;//因微分先行，微分符号变化，为减号
	pid->ui += pid->u;
	return pid->u;
}

