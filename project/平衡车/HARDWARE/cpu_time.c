#include "cpu_time.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"

void cpuWorkTimeInit(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void cpuWorkStart(void)
{
//	u16 temp=0;	
//		temp = GPIOB->ODR & (0x0001<<9);
//		if(temp){
//			GPIOB->BSRRH |= (0x0001<<9);
//		}else{
//			GPIOB->BSRRL |= (0x0001<<9);
//		
	GPIOB->BSRRH |= (0x0001<<9);//B9¸´Î»
}

void cpuWorkEnd(void)
{
//	u16 temp=0;	
//		temp = GPIOB->ODR & (0x0001<<9);
//		if(temp){
//			GPIOB->BSRRH |= (0x0001<<9);
//		}else{
//			GPIOB->BSRRL |= (0x0001<<9);
//		}
	GPIOB->BSRRL |= (0x0001<<9);//B9ÖÃÎ»
}

