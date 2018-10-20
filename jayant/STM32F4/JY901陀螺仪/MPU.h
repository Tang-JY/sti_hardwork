#ifndef __MPU_H

#define __MPU_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "usart3.h"

typedef struct{
	float ax;
	float ay;
	float az;//单位kg*m*s^-2
	float wx;
	float wy;
	float wz;//单位°/s
	u16 hx;
	u16 hy;
	u16 hz;
	float roll;
	float pitch;
	float yaw;//单位°
	s32 pressure;//单位Pa
	s32 height;//单位cm
}mpuTypeDef;


typedef struct{
	u8 YY;
	u8 MM;
	u8 DD;
	u8 hh;
	u8 mm;
	u8 ss;
	u16 ms;//时间		
	s16 acc[3];//加速度
	u8 acc_time;	
	s16 gyro[3];//角速度
	u8 gyro_time;	
	s16 ang[3];//角度
	u8 ang_time;	
	s16 mag[3];//磁场
	u8 mag_time;	
	u16 d_state[4];//端口状态
	s32 press[2];//气压、高度输出	
	s32 LonLat[2];//经纬度	
	s32 GPSV[2];//地速输出	
	
}rawDataTypeDef;//传感器原始数据结构体

extern rawDataTypeDef raw;//原始数据存储结构体
extern mpuTypeDef mpu;//实时数据存储结构体


void MPU_Init(unsigned int baud_rate);

void CopeSerialData(u8 Data);//串口3中断服务函数调用的数据处理函数

void mpu_Update(void);//数据更新

//void data_Init(void);//数据校准


#endif


