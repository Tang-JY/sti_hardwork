#include "cycle.h"
#include "sys.h"
#include "string.h"
#include <stdio.h>

vu8 cycle_flag = 0;


//APB1Ԥ��Ƶϵ��Ϊ4��APB1ʱ������Ƶ��42M��
//��Ƶϵ������1ʱ����ʱ��ʱ��Ƶ��Ϊ����Ƶ�ʵ�2��
//��TIM7Ƶ��Ϊ84M
void cycle_Init(float delt)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	vu32 arr;
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);//ʹ�ܶ�ʱ��7ʱ��
	
	TIM7->PSC = 83;//84M��ƵΪ1M��ÿ����ʱ��1us
	arr = 1000.0f*delt;//msת��ΪARR����ֵ
	TIM7->ARR = arr;
	TIM7->DIER |=0x0001;//ʹ�ܶ�ʱ���ж�
	TIM7->CR1  |=0x0001;//ʹ�ܼ�����
}


void TIM7_IRQHandler()
{
	vu16 status = TIM7->SR;//���Ĵ���
	
	if(status&0x0001)
	{
		TIM7->SR &= ~0x0001;//����жϱ�־λ
		cycle_flag = 1;
	}else{
		cycle_flag = 0;
	}
	return;
}

void timeMaster_setInt(int *data, const int new_data)
{
	INTX_DISABLE();
	*data = new_data;
	INTX_ENABLE();
}

void timeMaster_setFloat(float *data,const float new_data)
{
	INTX_DISABLE();
	*data = new_data;
	INTX_ENABLE();
}

void timeMaster_setByte(char *data,const char new_data)
{
	INTX_DISABLE();
	*data = new_data;
	INTX_ENABLE();
}

void timeMaster_memcpy(void *data,const void* new_data,unsigned int n)
{
	INTX_DISABLE();
	memcpy(data,new_data,n);
	INTX_ENABLE();
}

