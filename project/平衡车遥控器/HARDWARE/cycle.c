#include "cycle.h"

vu8 cycle_flag=0;

//APB1Ԥ��Ƶϵ��Ϊ4��APB1ʱ������Ƶ��42M��
//��Ƶϵ������1ʱ����ʱ��ʱ��Ƶ��Ϊ����Ƶ�ʵ�2��
//��TIM7Ƶ��Ϊ84M
void cycle_Init(u8 delt)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);//ʹ�ܶ�ʱ��7ʱ��
	
	TIM7->PSC = 83;//84M��ƵΪ1M��ÿ����ʱ��1us
	TIM7->ARR = 1000*delt;//msת��Ϊus	
	TIM7->DIER |=0x0001;//ʹ�ܶ�ʱ���ж�
	TIM7->CR1  |=0x0001;//ʹ�ܼ�����
}


void TIM7_IRQHandler()
{
	vu16 status = TIM7->SR;
	if(status&0x0001)
	{
		TIM7->SR &= ~0x0001;//����жϱ�־λ
		cycle_flag = 1;
	}
}



