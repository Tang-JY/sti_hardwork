#include "pwm.h"

//APB1����ʱ��Ƶ��Ϊ42M����AHB1�����ķ�Ƶϵ����Ϊ1����APB1�ϵĶ�ʱ��ʱ��Ҫ��2

#define PWM_PSC 42.0f           //Ԥ��Ƶϵ��
#define PWM_TIM3 84000000.0f  //�ڲ�ʱ��Ƶ��

vu32 arr;//��������װ��ֵ
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
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);// ʹ��TIM3ʱ��
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
	
	//GPIO����ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM3);                                                                      	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource0,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1,GPIO_AF_TIM3);	

   //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7 ; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1 ; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_Init(GPIOB, &GPIO_InitStructure);
 
	//TIM3����APB1�����ϣ�APB1����ʱ��Ϊ42MHz��TIM3��õ�ʱ��Ϊ84M
	arr = (u32)(PWM_TIM3/(PWM_PSC*freq));
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = (u32)(PWM_PSC-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ��
	
	TIM_ARRPreloadConfig(TIM3, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	

	//TIM_OCInitStructure.TIM_Pulse = 300; //���ô�װ�벶��ȽϼĴ���������ֵ
	//������ֻ�и߼���ʱ�����õ�
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM ����Ƚ� ���� �ߡ�����Ч��ƽΪ�ߵ�ƽ��
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ1������ֵС���趨ֵʱΪ��Ч��ƽ��
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR1�ϵ�Ԥװ�ؼĴ���

  
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


 // TIM_CtrlPWMOutputs(TIM3,ENABLE);	//MOE �����ʹ��	
 //�߼���ʱ������
 
	TIM_SetCompare1(TIM3,0);
	TIM_SetCompare2(TIM3,0);
	TIM_SetCompare3(TIM3,0);
	TIM_SetCompare4(TIM3,0);//���ñȽ�ֵ��PWM���ռ�ձ�Ϊ0
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



