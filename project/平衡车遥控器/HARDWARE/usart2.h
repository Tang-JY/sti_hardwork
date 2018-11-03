#ifndef __USART2_H
#define __USART2_H

#include "stm32f4xx.h"

//缓冲区长度
#define TX2_BUFFER_LEN 512
#define RX2_BUFFER_LEN 256



//数据发送同步器
typedef struct{
	u8 reg[4];//四个寄存器，功能：0：启动；1：开始移动；2：是否转弯；3：转弯方向（0左，1右）
//	float kp;	
//	float ki;
//	float kd;
//	float x_target;
//	float R;//转弯半径
	float vc_set;
	float vd_set;
	
}DataSync_TX_TypeDef;


//数据接收同步器
typedef struct{
	//消息
	char msg[16];
	u8 reg[4];//四个寄存器，功能：0：到达目标标志；1：倒下标志；2：保留；3：保留
	
	//实际值
//	float x;
	float R;
	float v;
	float roll;
	float rpmc;
	float rpmd;
	
	//中间变量
	//float vc_set;
	float roll_target;
	float rpmc_set;
	float rpmd_set;
	float pwm0;
	float pwm1;

	

}DataSync_RX_TypeDef;



void usart2_Init(u32 baud, DataSync_TX_TypeDef *tx, DataSync_RX_TypeDef *rx);

void usart2_SendSync(void);



#endif

