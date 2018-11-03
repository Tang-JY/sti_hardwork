#include "encoder.h"
#include "delay.h"
#include "usart1.h"
//#include "pwm.h"

EncoderTypeDef encoder;
EncoderTypeDef encoder1;
//ʵ�����������ṹ��

void encoder_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//��ʱ��2ʱ��ʹ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//��ʱ��5ʱ��ʹ��
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//GPIOAʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOBʱ��ʹ��
	
	//GPIOA��ʼ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  //����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//GPIOB��ʼ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
		//GPIO����ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource15,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3, GPIO_AF_TIM2);
	
	//��ʱ��ʱ����Ԫ����
	TIM2->ARR  =  4*LINE_NUM*GEARBOX;//Ȧ��*���ٱ���*����=��������������*4=����ֵ��תһȦ�պ����
	TIM2->CNT  =  2*LINE_NUM*GEARBOX;//��ʼֵΪһ��
	TIM2->CR1  |= 0x0001;//��������
	
	TIM5->ARR  =  4*LINE_NUM*GEARBOX;//Ȧ��*���ٱ���*����=��������������*4=����ֵ��תһȦ�պ����
	TIM5->CNT  =  2*LINE_NUM*GEARBOX;//��ʼֵΪһ��
	TIM5->CR1  |= 0x0001;//��������

	//���벶��ӳ��
	TIM2->CCMR1 |= 0x0001<<8;//TI2ӳ�䵽IC2
	TIM2->CCMR1 |= 0x0001<<0;//TI1ӳ�䵽IC1
	
	TIM5->CCMR1 |= 0x0001<<8;//TI2ӳ�䵽IC2
	TIM5->CCMR1 |= 0x0001<<0;//TI1ӳ�䵽IC1  
	
	//��ʱ��������ģʽ����
	TIM2->SMCR &= ~0x0007;TIM2->SMCR |= 0x0003;//������ģʽ3��˫���ؾ�����

	TIM5->SMCR &= ~0x0007;TIM5->SMCR |= 0x0003;//������ģʽ3��˫���ؾ�����

	encoder.deg=0;//�����ʼ�Ƕ�Ϊ0��
	encoder.deg_last=0;
	encoder.deg_rel=0;
	
	encoder1.deg=0;//�����ʼ�Ƕ�Ϊ0��
	encoder1.deg_last=0;
	encoder1.deg_rel=0;
}

void encoder_Update()
{//һ���������ڽ�������ʼ��ȡ
	s32 temp,temp1;
	static s32 count=0,count1=0;
	
	temp = TIM2->CNT - 2*LINE_NUM*GEARBOX;//���ټ�¼��ǰ����ֵ
	temp1 = TIM5->CNT - 2*LINE_NUM*GEARBOX;
	
	encoder.deg_last = encoder.deg;//�����ϴξ��ԽǶ�ֵ
	encoder1.deg_last = encoder1.deg;//�����ϴξ��ԽǶ�ֵ
	
	encoder.deg_rel_last = encoder.deg_rel;//�����ϴ���ԽǶ�ֵ
	encoder1.deg_rel_last = encoder1.deg_rel;//�����ϴ���ԽǶ�ֵ
	
	encoder.deg =(-180.0f*temp)/(2*LINE_NUM*GEARBOX);//���㱾�νǶ�ֵ
	encoder1.deg =(180.0f*temp1)/(2*LINE_NUM*GEARBOX);//���㱾�νǶ�ֵ
	
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
	
	encoder.deg_rel = encoder.deg + count*360;//���㱾����ԽǶ�ֵ
	encoder1.deg_rel = encoder1.deg + count1*360;//���㱾����ԽǶ�ֵ
	
	
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


