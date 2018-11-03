#include "MPU.h"
#include "usart3.h"
#include <stdio.h>
#include <string.h>
#include "REG.h"
#include "delay.h"

rawDataTypeDef raw;//原始数据存储结构体
mpuTypeDef mpu;//实时数据存储结构体


const float acc_g=9.8f;//重力加速度

void MPU_Init(unsigned int baud_rate)
{
	usart3_Init(baud_rate);//初始化串口3
	usart3_IRQHandlerRegister(CopeSerialData);
	
	return;
}

//读取串口3缓冲区的数据
void CopeSerialData(u8 Data)
{
	static unsigned char ucRxBuffer[12];
	static unsigned char ucRxCnt = 0;
	
	ucRxBuffer[ucRxCnt++]=Data;
		if (ucRxBuffer[0]!=0x55) //数据头不对，则重新开始寻找0x55数据头
	{
		ucRxCnt=0;
		return;
	}
	
	if (ucRxCnt<11) {return;}//数据不满11个，则返回
	else
	{
		switch(ucRxBuffer[1])
		{
			case 0x50:	memcpy(&raw.YY,&ucRxBuffer[2],8);
									break;//把原始数据写入原始数据结构体中
			case 0x51:	memcpy(&raw.acc[0],&ucRxBuffer[2],8);
									break;
			case 0x52:	memcpy(&raw.gyro[0],&ucRxBuffer[2],8);
									break;
			case 0x53:  memcpy(&raw.ang[0],&ucRxBuffer[2],8);						
									break;
			case 0x54:	memcpy(&raw.mag[0],&ucRxBuffer[2],8);
									break;
			case 0x55:	memcpy(&raw.d_state[0],&ucRxBuffer[2],8);
									break;
			case 0x56:	memcpy(&raw.press[0],&ucRxBuffer[2],8);
									break;
			case 0x57:	memcpy(&raw.LonLat[0],&ucRxBuffer[2],8);
									break;
			case 0x58:	memcpy(&raw.GPSV[0],&ucRxBuffer[2],8);
									break;
			default:break;
		}
		ucRxCnt=0;
	}
}



void mpu_Update(void)
{
	//姿态角更新
	mpu.roll = (float)raw.ang[0]/32768.0f*180.0f;
	mpu.pitch = (float)raw.ang[1]/32768.0f*180.0f;
	mpu.yaw = (float)(raw.ang[2])/32768.0f*180.0f;
	
	//加速度更新
	mpu.ax = (float)raw.acc[0]/32768.0f*16.0f*acc_g;
	mpu.ay = (float)raw.acc[1]/32768.0f*16.0f*acc_g;
	mpu.az = (float)raw.acc[2]/32768.0f*16.0f*acc_g;
	
	//角速度更新
	mpu.wx =  (float)raw.gyro[0]/32768.0f*2000.0f;
	mpu.wy =  (float)raw.gyro[1]/32768.0f*2000.0f;
	mpu.wz =  (float)raw.gyro[2]/32768.0f*2000.0f;
	
	//磁场更新
	mpu.hx = raw.mag[0];
	mpu.hy = raw.mag[1];
	mpu.hz = raw.mag[2];
	
	//气压、高度更新
	mpu.pressure = raw.press[0];
	mpu.height = raw.press[1];
}

//void data_Init()
//{
//	u8 cmd[5]={0xFF,0xAA,CALSW,0x03,0x00};
//	usart3_SendDatas(cmd,5);
//	
//	cmd[3]=0x00;
//	delay_ms(1);
//	usart3_SendDatas(cmd,5);
//	
//	cmd[2]=SAVE;
//	delay_ms(1);
//	usart3_SendDatas(cmd,5);

//}

