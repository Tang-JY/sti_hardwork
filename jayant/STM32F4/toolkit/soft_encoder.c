#include "soft_encoder.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "usart1.h"
#include <stdio.h>


#define ARR_VALUE (2*1*20)

SoftEncoder_TypeDef *soft_encoder0 = 0,*soft_encoder1 = 0;
volatile s32 offset0 = 0,offset1 = 0;//编码器上溢/下溢时计算用的补偿值


void softEncoder_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//定时器2时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//定时器5时钟使能
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//GPIOA时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOB时钟使能
	
	//GPIOA初始化（定时器引脚）
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  //复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//GPIOB初始化（定时器引脚）
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIOB初始化（开关引脚）
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIOC初始化（开关引脚）
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//GPIO复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3, GPIO_AF_TIM2);
	
	//定时器时基单元配置
	TIM2->ARR = ARR_VALUE;//转1圈溢出（圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值）
	TIM2->CNT = ARR_VALUE/2;//初始值为一半
	
	
	TIM5->ARR = ARR_VALUE;//转100圈溢出（圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值）
	TIM5->CNT = ARR_VALUE/2;//初始值为一半
	

	//输入捕获映射
	TIM2->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM2->CCMR1 |= 0x0001<<0;//TI1映射到IC1
	
	TIM5->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM5->CCMR1 |= 0x0001<<0;//TI1映射到IC1  
	
	//定时器编码器模式配置
	TIM2->SMCR &= ~0x0007;TIM2->SMCR |= 0x0002;//编码器模式2，在TI1边沿读TI2

	TIM5->SMCR &= ~0x0007;TIM5->SMCR |= 0x0002;//编码器模式2，在TI1边沿读TI2

	//中断配置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能定时器中断
	TIM2->DIER |=0x0001;
	TIM5->DIER |=0x0001;
	

	
	//计数器打开
	TIM2->CR1  |= 0x0001;
	TIM5->CR1  |= 0x0001;
		
	
	/*
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOB时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);//GPIOG时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);//SYSCFG时钟使能
	
	//设置GPIO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	//设置中断线
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource6);//中断线6连接到PG6上
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG,EXTI_PinSource7);//中断线7连接到PG7上
	
	//初始化线上中断，设置触发条件
	EXTI->IMR |= (0x01<<6);//中断屏蔽寄存器
	EXTI->IMR |= (0x01<<7);
	EXTI->FTSR |= (0x01<<6);//下降沿触发选择寄存器
	EXTI->FTSR |= (0x01<<7);
	EXTI->RTSR &= ~(0x01<<6);//关闭上升沿触发
	EXTI->RTSR &= ~(0x01<<7);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);*/
}

void TIM2_IRQHandler(void)
{
	//usart1_SendStr("TIM2进入中断\r\n");
	if(TIM2->SR & 0x0001){
		TIM2->SR &= ~(0x0001);
		
		if(!(TIM2->CR1 & (0x0001<<4))){//CR1 DR位为0，递增计数			
			//usart1_SendStr(" 编码器0上溢\r\n");
			offset0 = ARR_VALUE;
			
		}else{
			//usart1_SendStr(" 编码器0下溢\r\n");
			offset0 = -1*ARR_VALUE;
		}	
	}
}

void TIM5_IRQHandler(void)
{
	usart1_SendStr("TIM5进入中断\r\n");
	if(TIM5->SR & 0x0001){
		TIM5->SR &= ~(0x0001);
		
		if(!(TIM5->CR1 & (0x0001<<4))){//CR1 DR位为0，递增计数
			usart1_SendStr(" 编码器1上溢\r\n");
			offset1 = ARR_VALUE;;
		}else{
			usart1_SendStr(" 编码器1下溢\r\n");
			offset1 = -1*ARR_VALUE;
		}	
	}
}


/*
void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR & (0x01<<6))
	{
		EXTI->PR &=(0x01<<6);//写1置0
		if(soft_encoder0!=0)//若soft_encoder0已分配到数据
		{
			if(GPIOB->IDR &(0x01<<3))
			{
					*(soft_encoder0->data) += soft_encoder0->dx;
					if(*(soft_encoder0->data) > soft_encoder0->range[1])
					{
						*(soft_encoder0->data) = soft_encoder0->range[1];
					}
				
			}else{
				
					*(soft_encoder0->data) -= soft_encoder0->dx;
					if(*(soft_encoder0->data) < soft_encoder0->range[0])
					{
						*(soft_encoder0->data) = soft_encoder0->range[0];		
					}
			}
		}		
	}
	
	if(EXTI->PR & (0x01<<7))
	{
		EXTI->PR &=(0x01<<7);
		if(soft_encoder1!=0)//若soft_encoder1已分配到数据
		{
			if(GPIOB->IDR &(0x01<<5))
			{
					*(soft_encoder1->data) += soft_encoder1->dx;
					if(*(soft_encoder1->data) > soft_encoder1->range[1])
					{
						*(soft_encoder1->data) = soft_encoder1->range[1];
					}
				
			}else{
				
					*(soft_encoder1->data) -= soft_encoder1->dx;
					if(*(soft_encoder1->data) < soft_encoder1->range[0])
					{
						*(soft_encoder1->data) = soft_encoder1->range[0];		
					}
			}
		}			
	}
}
*/

void softEncoder_CFG(SoftEncoder_TypeDef *encoder,float *data,float min, float max)
{
	encoder->data = data;
	encoder->dx = (max-min)/80.0f;
	encoder->range[0] = min;
	encoder->range[1] = max;
	printf(" min:%.2f max:%.2f\r\n",min,max);
}

void softEncoder_Binding(u8 encoder, SoftEncoder_TypeDef* data)
{
	if(!encoder)
	{
		soft_encoder0 = data;
	}else{
		soft_encoder1 = data;
	}
}

u8 softEncoder_GetKeyState(u8 encoder)
{
	static u8 sta0[4]={0},sta1[4]={0};
	if(!encoder)//encoder0
	{
		sta0[3] = sta0[2];
		sta0[2] = sta0[1];
		sta0[1] = sta0[0];
		sta0[0] = ((GPIOC->IDR & (0x01<<0))!=0);
		if(soft_encoder0 != NULL){
			if(sta0[3] && sta0[2] && !sta0[1] && !sta0[0]){
				return 1;
			}
		}
		return 0;
		
	}else{//encoder1
		
		sta1[3] = sta1[2];
		sta1[2] = sta1[1];
		sta1[1] = sta1[0];
		sta1[0] = ((GPIOB->IDR & (0x01<<11))!=0);
		if(soft_encoder1 != NULL){
			if(sta1[3] && sta1[2] && (!sta1[1]) && (!sta1[0])){
				return 1;
			}
		}
		return 0;		
	}
}

void softEncoder_UpdateEncoderState(void)
{
	static vu32 cnt0_last = ARR_VALUE/2, cnt1_last = ARR_VALUE/2;//上次定时器值
	vu32 cnt0,cnt1;
	float temp;
	vs32 num;
	
	cnt0 = TIM2->CNT;//读定时器
	cnt1 = TIM5->CNT;//读定时器
	
	//计算数据增量
	if(soft_encoder0 != NULL){
		num = offset0 + (s32)cnt0 - (s32)cnt0_last;
		temp = num*soft_encoder0->dx;
		*(soft_encoder0->data) += temp;
		//限辐
		if( *(soft_encoder0->data) > soft_encoder0->range[1]){
			*(soft_encoder0->data) = soft_encoder0->range[1];
		}else if(*(soft_encoder0->data) < soft_encoder0->range[0]){
			*(soft_encoder0->data) = soft_encoder0->range[0];
		}
	}
	
	//计算数据增量
	if(soft_encoder1 != NULL){
		
		num = offset1 + (s32)cnt1 - (s32)cnt1_last;
		temp = soft_encoder1->dx * num;		
		(*(soft_encoder1->data)) = (*(soft_encoder1->data)) + temp;
		
		//限辐
		if( (*(soft_encoder1->data)) > soft_encoder1->range[1]){
			*(soft_encoder1->data) = soft_encoder1->range[1];
		}else if( (*(soft_encoder1->data)) < soft_encoder1->range[0]){
			*(soft_encoder1->data) = soft_encoder1->range[0];
			
		}
	}
	
	cnt0_last = cnt0;//记录历史数据
	cnt1_last = cnt1;//记录历史数据
	offset0 = 0;
	offset1 = 0;
	
}
