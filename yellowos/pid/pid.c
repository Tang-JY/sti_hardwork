// Copyright 2018 yellowos
#include "pid/pid.h"

void pidInit(
    pid_TypeDef *pid,
    float *target,
    float *feedback,
    float dt,
    float p,
    float i,
    float d,
    float e_limit,
    float ei_limit)
{
    pid->target = target;
    pid->feedback = feedback;
    pid->dt = dt;
    pid->kp = p;
    pid->ki = i;
    pid->kd = d;
    pid->e_limit = e_limit;
    pid->ei_limit = ei_limit;
    pid->e[E_CURRENT] = 0;
    pid->e[E_PREVIOUS] = 0;
    pid->ei = 0;
}

float pidCompute(pid_TypeDef *pid)
{
    pid->e[E_PREVIOUS] = pid->e[E_CURRENT];
    pid->e[E_CURRENT] = *pid->target - *pid->feedback;
    pid->ei += pid->e[E_CURRENT];
    pid->u += pid->kp * pid->e[E_CURRENT];
    pid->u += pid->ki * pid->ei * pid->dt;
    pid->u += pid->kd * (pid->e[E_CURRENT] - pid->e[E_PREVIOUS]) / pid->dt;
    return pid->u;
}
