#include <math.h>

#include "pid.h"
#include "pwm.h"
#include "cycle.h"
#include "usart1.h"
#include "encoder.h"

extern PID_TypeDef pid;

extern float PWM_output;
extern float target_angle;
extern float current_angle;

extern int openloop_Mode;

void pid_handle(void) {
    encoder_Update();
    current_angle = encoder0.deg_rel;
    if(30 < fabs(current_angle - target_angle)) {
        openloop_Mode = 1;        
    } else {
        openloop_Mode = 0;
        pid_Calc(&pid);
        FH_Motor_SetPWM(pid.u, 0);
    }
}
