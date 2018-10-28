// Copyright 2018 yellowos
#ifndef PID_PID_H_
#define PID_PID_H_

#define E_PREVIOUS 1
#define E_CURRENT 0

struct pid{
    float* target;  // 被控量目标值地址
    float* feedback;  // 被控量当前值地址
    float dt;  // 控制周期，单位s
    float kp;  // P系数
    float ki;  // I系数
    float kd;  // D系数
    float ei;  // 误差积分
    float e[2];  // 误差
    float e_limit;  // 积分分离阈值
    float ei_limit;  // 积分限幅阈值
    float u;  // 输出值
};

typedef struct pid pid_TypeDef;

void pidInit(
    pid_TypeDef *pid,
    float *target,
    float *feedback,
    float dt,
    float p,
    float i,
    float d,
    float e_limit,
    float ei_limit);

float pidCompute(pid_TypeDef *pid);

#endif  // PID_PID_H_
