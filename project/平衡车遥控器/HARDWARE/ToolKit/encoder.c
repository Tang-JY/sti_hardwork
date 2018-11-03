#include "encoder.h"
#include "delay.h"
#include "usart1.h"
//#include "pwm.h"

EncoderTypeDef encoder;
EncoderTypeDef encoder1;
//实例化编码器结构体

void encoder_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//定时器2时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//定时器5时钟使能
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//GPIOA时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOB时钟使能
	
	//GPIOA初始化
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  //复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//GPIOB初始化
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
		//GPIO复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource15,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3, GPIO_AF_TIM2);
	
	//定时器时基单元配置
	TIM2->ARR  =  4*LINE_NUM*GEARBOX;//圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值。转一圈刚好溢出
	TIM2->CNT  =  2*LINE_NUM*GEARBOX;//初始值为一半
	TIM2->CR1  |= 0x0001;//计数器打开
	
	TIM5->ARR  =  4*LINE_NUM*GEARBOX;//圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值。转一圈刚好溢出
	TIM5->CNT  =  2*LINE_NUM*GEARBOX;//初始值为一半
	TIM5->CR1  |= 0x0001;//计数器打开

	//输入捕获映射
	TIM2->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM2->CCMR1 |= 0x0001<<0;//TI1映射到IC1
	
	TIM5->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM5->CCMR1 |= 0x0001<<0;//TI1映射到IC1  
	
	//定时器编码器模式配置
	TIM2->SMCR &= ~0x0007;TIM2->SMCR |= 0x0003;//编码器模式3，双边沿均计数

	TIM5->SMCR &= ~0x0007;TIM5->SMCR |= 0x0003;//编码器模式3，双边沿均计数

	encoder.deg=0;//电机初始角度为0；
	encoder.deg_last=0;
	encoder.deg_rel=0;
	
	encoder1.deg=0;//电机初始角度为0；
	encoder1.deg_last=0;
	encoder1.deg_rel=0;
}

void encoder_Update()
{//一个控制周期结束，开始读取
	s32 temp,temp1;
	static s32 count=0,count1=0;
	
	temp = TIM2->CNT - 2*LINE_NUM*GEARBOX;//快速记录当前计数值
	temp1 = TIM5->CNT - 2*LINE_NUM*GEARBOX;
	
	encoder.deg_last = encoder.deg;//保存上次绝对角度值
	encoder1.deg_last = encoder1.deg;//保存上次绝对角度值
	
	encoder.deg_rel_last = encoder.deg_rel;//保存上次相对角度值
	encoder1.deg_rel_last = encoder1.deg_rel;//保存上次相对角度值
	
	encoder.deg =(-180.0f*temp)/(2*LINE_NUM*GEARBOX);//计算本次角度值
	encoder1.deg =(180.0f*temp1)/(2*LINE_NUM*GEARBOX);//计算本次角度值
	
	//TIM2
	if(encoder.deg_last<-170 && encoder.deg>170)
	{
		count--;
	}
	else if(encoder.deg_last>170 && encoder.deg<-170)
	{
		count++;
	}
	
	//TIM5
	if(encoder1.deg_last<-170 && encoder1.deg>170)
	{
		count1--;
	}
	else if(encoder1.deg_last>170 && encoder1.deg<-170)
	{
		count1++;
	}
	
	encoder.deg_rel = encoder.deg + count*360;//计算本次相对角度值
	encoder1.deg_rel = encoder1.deg + count1*360;//计算本次相对角度值
	
	
	encoder.rpm = (60000/(360*DT))*(encoder.deg_rel - encoder.deg_rel_last);
	encoder1.rpm = (60000/(360*DT))*(encoder1.deg_rel - encoder1.deg_rel_last);
	
	if(encoder.rpm >0)
	{
		encoder.derc=1;
	}
	else if(encoder.rpm<0)
	{
		encoder.derc=-1;
	}
	else
	{
		encoder.derc=0;
	}
	
	
	if(encoder1.rpm >0)
	{
		encoder1.derc=1;
	}
	else if(encoder1.rpm<0)
	{
		encoder1.derc=-1;
	}
	else
	{
		encoder1.derc=0;
	}
}


