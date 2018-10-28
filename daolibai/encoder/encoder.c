#include "encoder.h"
#include "delay.h"
#include "usart1.h"
//#include "pwm.h"

//实例化编码器结构体
EncoderTypeDef encoder0;
EncoderTypeDef encoder1;

//上溢、下溢时的偏移量
vs32 offset0 = 0, offset1 = 0;
//extern const u32 dt;
volatile float _dt = 10.0f;

void encoder_Init(float dt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	_dt = dt;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//定时器2时钟使能
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//定时器5时钟使能
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//GPIOA时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOB时钟使能
	
	//GPIOA初始化
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  //复用
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//GPIOB初始化
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIO复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_TIM2);
	
	//定时器时基单元配置
	TIM2->ARR  =  4*LINE_NUM*GEARBOX;//圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值。转一圈刚好溢出
	TIM2->CNT  =  2*LINE_NUM*GEARBOX;//初始值为一半
	
	TIM5->ARR  =  4*LINE_NUM*GEARBOX;//圈数*减速倍数*线数=脉冲数，脉冲数*4=计数值。转一圈刚好溢出
	TIM5->CNT  =  2*LINE_NUM*GEARBOX;//初始值为一半
	

	//输入捕获映射
	TIM2->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM2->CCMR1 |= 0x0001<<0;//TI1映射到IC1
	
	TIM5->CCMR1 |= 0x0001<<8;//TI2映射到IC2
	TIM5->CCMR1 |= 0x0001<<0;//TI1映射到IC1  
	
	//定时器编码器模式配置
	TIM2->SMCR &= ~0x0007;
	TIM2->SMCR |= 0x0003;//编码器模式3，双边沿均计数

	TIM5->SMCR &= ~0x0007;
	TIM5->SMCR |= 0x0003;//编码器模式3，双边沿均计数
	
	

	encoder0.deg      = 180;//电机初始角度为0；
	encoder0.deg_last = 0;
	encoder0.deg_rel  = 0;
	
	encoder1.deg      = 0;//电机初始角度为0；
	encoder1.deg_last = 0;
	encoder1.deg_rel  = 0;
	
	//中断配置
	NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能定时器中断
	TIM2->DIER |=0x0001;
	TIM5->DIER |=0x0001;
	
	//计数器打开
	TIM2->CR1  |= 0x0001;
	TIM5->CR1  |= 0x0001;
}


void TIM2_IRQHandler(void)
{
	static vs32 _offset = 4*LINE_NUM*GEARBOX;
	if(TIM2->SR & 0x0001){
		TIM2->SR &= ~(0x0001);
		
		if(!(TIM2->CR1 & (0x0001<<4))){//CR1 DR位为0，递增计数			
			offset0 = _offset;
			
		}else{//递减计数
			offset0 = -1*_offset;
		}	
	}
}

void TIM5_IRQHandler(void)
{
	static vs32 _offset = 4*LINE_NUM*GEARBOX;
	if(TIM5->SR & 0x0001){
		TIM5->SR &= ~(0x0001);
		
		if(!(TIM5->CR1 & (0x0001<<4))){//CR1 DR位为0，递增计数
			offset1 = _offset;
		}else{
			offset1 = -1*_offset;
		}
	}
}

void encoder_Update()
{//一个控制周期结束，开始读取
	static vs32 cnt0_last = 2*LINE_NUM*GEARBOX , cnt1_last = 2*LINE_NUM*GEARBOX;
	vs32 cnt0,cnt1;
	const float rpm_factor = 15000.0f/(LINE_NUM*GEARBOX*_dt);
	const float angle_factor = 90.0f/(1.0f*LINE_NUM*GEARBOX);
	
	//读定时器
	cnt0 = TIM2->CNT;
	cnt1 = TIM5->CNT;
	
	//保存上次绝对角度值(-180~+180度)
	encoder0.deg_last = encoder0.deg;
	encoder1.deg_last = encoder1.deg;
	
	//保存上次相对角度值（相对于上电时的角度）
	encoder0.deg_rel_last = encoder0.deg_rel;
	encoder1.deg_rel_last = encoder1.deg_rel;//保存上次相对角度值
	
	//计算本次绝对角度值
	//encoder0.deg =  90.0f*(cnt0 - 2.0f*LINE_NUM*GEARBOX)/(LINE_NUM*GEARBOX);//计算本次角度值
	//encoder1.deg =  90.0f*(cnt1 - 2.0f*LINE_NUM*GEARBOX)/(LINE_NUM*GEARBOX);//计算本次角度值
	encoder0.deg = angle_factor*(cnt0 - 2.0f*LINE_NUM*GEARBOX);//计算本次角度值
	encoder1.deg = angle_factor*(cnt1 - 2.0f*LINE_NUM*GEARBOX);
	
	//计算本次相对角度值
	encoder0.deg_rel += angle_factor*(cnt0 - cnt0_last + offset0);
	encoder1.deg_rel += angle_factor*(cnt1 - cnt1_last + offset1);
	
	
	//计算速度(dt (ms) = dt/60000 (min))
	encoder0.rpm = rpm_factor*(cnt0 - cnt0_last + offset0);
	encoder1.rpm = rpm_factor*(cnt1 - cnt1_last + offset1);
	
	
	//重置offset
	offset0 = 0;
	offset1 = 0;
	
	//记录本次值
	cnt0_last = cnt0;
	cnt1_last = cnt1;
}


