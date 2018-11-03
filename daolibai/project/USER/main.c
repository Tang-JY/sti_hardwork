#include "stm32f4xx.h"  

#include "pid/pid.h"
#include "uart/usart1.h"
#include "encoder/encoder.h"
#include "pwm/pwm.h"
#include "system/cycle.h"
#include "handle/init.h"
#include "handle/pid_handle.h"


#define ctrl_period 1
PID_TypeDef pid;

float Kp = 1;
float Ki = 0;
float Kd = 0;

float PWM_output;
float target_angle;
float current_angle;

int main(void) {
    init_handle();
    cycleInit(1);
    cycleTask_TypeDef pid_task;
    cycleForegroundTaskInit(&pid_task, ctrl_period, pid_handle);
    while (1) {

    }
}
