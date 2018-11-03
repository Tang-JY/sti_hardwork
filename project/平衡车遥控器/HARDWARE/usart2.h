#ifndef __USART2_H
#define __USART2_H

#include "stm32f4xx.h"

//����������
#define TX2_BUFFER_LEN 512
#define RX2_BUFFER_LEN 256



//���ݷ���ͬ����
typedef struct{
	u8 reg[4];//�ĸ��Ĵ��������ܣ�0��������1����ʼ�ƶ���2���Ƿ�ת�䣻3��ת�䷽��0��1�ң�
//	float kp;	
//	float ki;
//	float kd;
//	float x_target;
//	float R;//ת��뾶
	float vc_set;
	float vd_set;
	
}DataSync_TX_TypeDef;


//���ݽ���ͬ����
typedef struct{
	//��Ϣ
	char msg[16];
	u8 reg[4];//�ĸ��Ĵ��������ܣ�0������Ŀ���־��1�����±�־��2��������3������
	
	//ʵ��ֵ
//	float x;
	float R;
	float v;
	float roll;
	float rpmc;
	float rpmd;
	
	//�м����
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

