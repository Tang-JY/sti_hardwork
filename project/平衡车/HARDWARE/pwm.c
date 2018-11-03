#include "pwm.h"

//APB1总线时钟频率为42M，从AHB1得来的分频系数不为1，故APB1上的定时器时钟要乘2

#define PWM_PSC 42.0f           //预分频系数
#define PWM_TIM3 84000000.0f  //内部时钟频率

vu32 arr;//计数器重装载值
volatile float pwm_min = 0.0f, pwm_max = 100.0f;

void PWM_Init(float freq,float min,float max)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	pwm_min = min;
	pwm_max = max;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);// 使能TIM3时钟
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
	
	//GPIO复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM3);                                                                      	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource0,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1,GPIO_AF_TIM3);	

   //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7 ; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //复用输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 ; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	//TIM3挂在APB1总线上，APB1总线时钟为42MHz，TIM3获得的时钟为84M
	arr = (u32)(PWM_TIM3/(PWM_PSC*freq));
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = (u32)(PWM_PSC-1); //设置用来作为TIMx时钟频率除数的预分频值  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位、
	
	TIM_ARRPreloadConfig(TIM3, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	

	//TIM_OCInitStructure.TIM_Pulse = 300; //设置待装入捕获比较寄存器的脉冲值
	//此属性只有高级定时器才用到
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM 输出比较 极性 高。（有效电平为高电平）
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式1（计数值小于设定值时为有效电平）
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR1上的预装载寄存器

  
  //TIM_OCInitStructure.TIM_Pulse = 150;
	//TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Enable;
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

  
  //TIM_OCInitStructure.TIM_Pulse = 150;
	//TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Enable;
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

  
  //TIM_OCInitStructure.TIM_Pulse = 150;
	//TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Enable;
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);


 // TIM_CtrlPWMOutputs(TIM3,ENABLE);	//MOE 主输出使能	
 //高级定时器才有
 
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
	TIM_SetCompare3(TIM3,0);
	TIM_SetCompare4(TIM3,0);//设置比较值，PWM输出占空比为0
}

u8 Set_PWM_DutyRatio(u8 channel,float duty)
{
	float compare;
	if(duty > pwm_max)
	{
		duty = pwm_max;
	}
	else if(duty < pwm_min)
	{
		duty = pwm_min;
	}
	
	compare = (float)arr*duty/100.0f;
	switch(channel)
	{
		case 1:TIM3->CCR1 = (u32)compare;break;
		case 2:TIM3->CCR2 = (u32)compare;break;
		case 3:TIM3->CCR3 = (u32)compare;break;
		case 4:TIM3->CCR4 = (u32)compare;break;
		default:return 1;
	}
	return 0;
}



