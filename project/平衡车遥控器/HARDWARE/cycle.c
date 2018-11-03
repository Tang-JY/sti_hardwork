#include "cycle.h"

vu8 cycle_flag=0;

//APB1预分频系数为4，APB1时钟总线频率42M，
//分频系数不是1时，定时器时钟频率为总线频率的2倍
//故TIM7频率为84M
void cycle_Init(u8 delt)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);//使能定时器7时钟
	
	TIM7->PSC = 83;//84M分频为1M，每周期时间1us
	TIM7->ARR = 1000*delt;//ms转化为us	
	TIM7->DIER |=0x0001;//使能定时器中断
	TIM7->CR1  |=0x0001;//使能计数器
}


void TIM7_IRQHandler()
{
	vu16 status = TIM7->SR;
	if(status&0x0001)
	{
		TIM7->SR &= ~0x0001;//清除中断标志位
		cycle_flag = 1;
	}
}



