#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"

//�˴������ʹ��32λ��ʱ����TIM2��TIM5��

//������Ϊ512��
//������Ϊ64/1

//TIM2Ϊ������������
//PB10 PB11 ��Ϊ������A B����



#define LINE_NUM 512   //����������
#define GEARBOX  64 //���ٱ�


typedef struct{
	s8 derc;//��ת����
	float rpm;//��ת�ٶ�
	float deg;//��ǰ���ԽǶ�
	float deg_last;
	float deg_rel;//�Կ���ʱΪ0�����ԽǶ�
	float deg_rel_last;//�ϴ���ԽǶȣ����ڼ����ٶ�
}EncoderTypeDef;

extern  EncoderTypeDef encoder0;
extern  EncoderTypeDef encoder1;


void encoder_Init(float dt);//�������ų�ʼ��

void encoder_Update(void);//���ݶ�ȡ

#endif

