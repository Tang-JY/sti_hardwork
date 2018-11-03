#ifndef __S_ENCODER_H
#define __S_ENCODER_H

#include "stm32f4xx.h"

//�˱����������ڼ����ٶȣ��������ֶ������豸�ı�����

typedef struct{
	float *data;//��������
	float dx;//ÿ�α�����������ʱ�������������
	float range[2];//���ݱ��޸ĵķ�Χ
}SoftEncoder_TypeDef;


//encoder0 ʹ��TIM2(A5 B3),��������C0
//encoder1 ʹ��TIM2(A0 A1),��������B11


void softEncoder_Init(void);

void softEncoder_CFG(SoftEncoder_TypeDef *encoder,float *data, float min, float max);//���ݽṹ������

void softEncoder_Binding(u8 encoder, SoftEncoder_TypeDef* data);//�����ݰ󶨵���������������ת��ʱ�޸Ķ�Ӧ����
//���������������0/1��������0ʹ��PG6 PB3 PB4��  ,��Ӧ���ݽṹ��

u8 softEncoder_GetKeyState(u8 encoder);//��ñ������Ͽ��ص�״̬

void softEncoder_UpdateEncoderState(void);//���±�����״̬

#endif

