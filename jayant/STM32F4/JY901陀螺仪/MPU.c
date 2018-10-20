#include "MPU.h"
#include "usart3.h"
#include <stdio.h>
#include <string.h>
#include "REG.h"
#include "delay.h"

rawDataTypeDef raw;//ԭʼ���ݴ洢�ṹ��
mpuTypeDef mpu;//ʵʱ���ݴ洢�ṹ��


const float acc_g=9.8f;//�������ٶ�

void MPU_Init(unsigned int baud_rate)
{
	usart3_Init(baud_rate);//��ʼ������3
	usart3_IRQHandlerRegister(CopeSerialData);
	
	return;
}

//��ȡ����3������������
void CopeSerialData(u8 Data)
{
	static unsigned char ucRxBuffer[12];
	static unsigned char ucRxCnt = 0;
	
	ucRxBuffer[ucRxCnt++]=Data;
		if (ucRxBuffer[0]!=0x55) //����ͷ���ԣ������¿�ʼѰ��0x55����ͷ
	{
		ucRxCnt=0;
		return;
	}
	
	if (ucRxCnt<11) {return;}//���ݲ���11�����򷵻�
	else
	{
		switch(ucRxBuffer[1])
		{
			case 0x50:	memcpy(&raw.YY,&ucRxBuffer[2],8);
									break;//��ԭʼ����д��ԭʼ���ݽṹ����
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
	//��̬�Ǹ���
	mpu.roll = (float)raw.ang[0]/32768.0f*180.0f;
	mpu.pitch = (float)raw.ang[1]/32768.0f*180.0f;
	mpu.yaw = (float)(raw.ang[2])/32768.0f*180.0f;
	
	//���ٶȸ���
	mpu.ax = (float)raw.acc[0]/32768.0f*16.0f*acc_g;
	mpu.ay = (float)raw.acc[1]/32768.0f*16.0f*acc_g;
	mpu.az = (float)raw.acc[2]/32768.0f*16.0f*acc_g;
	
	//���ٶȸ���
	mpu.wx =  (float)raw.gyro[0]/32768.0f*2000.0f;
	mpu.wy =  (float)raw.gyro[1]/32768.0f*2000.0f;
	mpu.wz =  (float)raw.gyro[2]/32768.0f*2000.0f;
	
	//�ų�����
	mpu.hx = raw.mag[0];
	mpu.hy = raw.mag[1];
	mpu.hz = raw.mag[2];
	
	//��ѹ���߶ȸ���
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

