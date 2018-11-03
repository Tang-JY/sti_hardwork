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
	float az;//��λkg*m*s^-2
	float wx;
	float wy;
	float wz;//��λ��/s
	u16 hx;
	u16 hy;
	u16 hz;
	float roll;
	float pitch;
	float yaw;//��λ��
	s32 pressure;//��λPa
	s32 height;//��λcm
}mpuTypeDef;


typedef struct{
	u8 YY;
	u8 MM;
	u8 DD;
	u8 hh;
	u8 mm;
	u8 ss;
	u16 ms;//ʱ��		
	s16 acc[3];//���ٶ�
	u8 acc_time;	
	s16 gyro[3];//���ٶ�
	u8 gyro_time;	
	s16 ang[3];//�Ƕ�
	u8 ang_time;	
	s16 mag[3];//�ų�
	u8 mag_time;	
	u16 d_state[4];//�˿�״̬
	s32 press[2];//��ѹ���߶����	
	s32 LonLat[2];//��γ��	
	s32 GPSV[2];//�������	
	
}rawDataTypeDef;//������ԭʼ���ݽṹ��

extern rawDataTypeDef raw;//ԭʼ���ݴ洢�ṹ��
extern mpuTypeDef mpu;//ʵʱ���ݴ洢�ṹ��


void MPU_Init(unsigned int baud_rate);

void CopeSerialData(u8 Data);//����3�жϷ��������õ����ݴ�����

void mpu_Update(void);//���ݸ���

//void data_Init(void);//����У׼


#endif


