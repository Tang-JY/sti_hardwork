#include "pid.h"
#include "pwm.h"
#include "cycle.h"
#include "usart1.h"
#include "encoder.h"

extern PID_TypeDef pid;

extern float Kp;
extern float Ki;
extern float Kd;

extern float PWM_output;
extern float target_angle;
extern float current_angle;

void init_handle(void) {
	// pid 初始化
    pid_params_CFG(&pid, &Kp, &Ki, &Kd, 0.001);  // 设置控制周期为1ms
    pid_limits_CFG(&pid, 15, 100, 0);  // 非同步参数设置未知
    pid_data_CFG(&pid, &target_angle, &current_angle); // 传入目标角度和当前角度的地址

    // 编码器初始化
	encoder_Init(0.001);

	//PWM 初始化
    PWM_Init(50000, 0, 95);  // 占空比单位？

	// uart 初始化
	usart1Init(115200);

    // motor 初始化
    FH_Motor_Init(40000, 0, 95);
}