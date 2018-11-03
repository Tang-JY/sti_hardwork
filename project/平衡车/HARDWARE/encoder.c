#include "encoder.h"
#include "delay.h"
#include "usart1.h"
//#include "pwm.h"

//ʵ�����������ṹ��
EncoderTypeDef encoder0;
EncoderTypeDef encoder1;

//���硢����ʱ��ƫ����
vs32 offset0 = 0, offset1 = 0;
//extern const u32 dt;
volatile float _dt = 10.0f;

void encoder_Init(float dt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	_dt = dt;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//��ʱ��2ʱ��ʹ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);//��ʱ��5ʱ��ʹ��
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//GPIOAʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);//GPIOBʱ��ʹ��
	
	//GPIOA��ʼ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;  //����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//GPIOB��ʼ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//GPIO����ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM5); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_TIM2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_TIM2);
	
	//��ʱ��ʱ����Ԫ����
	TIM2->ARR  =  4*LINE_NUM*GEARBOX;//Ȧ��*���ٱ���*����=��������������*4=����ֵ��תһȦ�պ����
	TIM2->CNT  =  2*LINE_NUM*GEARBOX;//��ʼֵΪһ��
	
	TIM5->ARR  =  4*LINE_NUM*GEARBOX;//Ȧ��*���ٱ���*����=��������������*4=����ֵ��תһȦ�պ����
	TIM5->CNT  =  2*LINE_NUM*GEARBOX;//��ʼֵΪһ��
	

	//���벶��ӳ��
	TIM2->CCMR1 |= 0x0001<<8;//TI2ӳ�䵽IC2
	TIM2->CCMR1 |= 0x0001<<0;//TI1ӳ�䵽IC1
	
	TIM5->CCMR1 |= 0x0001<<8;//TI2ӳ�䵽IC2
	TIM5->CCMR1 |= 0x0001<<0;//TI1ӳ�䵽IC1  
	
	//��ʱ��������ģʽ����
	TIM2->SMCR &= ~0x0007;TIM2->SMCR |= 0x0003;//������ģʽ3��˫���ؾ�����

	TIM5->SMCR &= ~0x0007;TIM5->SMCR |= 0x0003;//������ģʽ3��˫���ؾ�����
	
	

	encoder0.deg      = 180;//�����ʼ�Ƕ�Ϊ0��
	encoder0.deg_last = 0;
	encoder0.deg_rel  = 0;
	
	encoder1.deg      = 0;//�����ʼ�Ƕ�Ϊ0��
	encoder1.deg_last = 0;
	encoder1.deg_rel  = 0;
	
	//�ж�����
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
	
	//ʹ�ܶ�ʱ���ж�
	TIM2->DIER |=0x0001;
	TIM5->DIER |=0x0001;
	
	//��������
	TIM2->CR1  |= 0x0001;
	TIM5->CR1  |= 0x0001;
}


void TIM2_IRQHandler(void)
{
	static vs32 _offset = 4*LINE_NUM*GEARBOX;
	if(TIM2->SR & 0x0001){
		TIM2->SR &= ~(0x0001);
		
		if(!(TIM2->CR1 & (0x0001<<4))){//CR1 DRλΪ0����������			
			offset0 = _offset;
			
		}else{//�ݼ�����
			offset0 = -1*_offset;
		}	
	}
}

void TIM5_IRQHandler(void)
{
	static vs32 _offset = 4*LINE_NUM*GEARBOX;
	if(TIM5->SR & 0x0001){
		TIM5->SR &= ~(0x0001);
		
		if(!(TIM5->CR1 & (0x0001<<4))){//CR1 DRλΪ0����������
			offset1 = _offset;
			
		}else{
			offset1 = -1*_offset;
		}	
	}
}

void encoder_Update()
{//һ���������ڽ�������ʼ��ȡ
	static vs32 cnt0_last = 2*LINE_NUM*GEARBOX , cnt1_last = 2*LINE_NUM*GEARBOX;
	vs32 cnt0,cnt1;
	const float rpm_factor = 15000.0f/(LINE_NUM*GEARBOX*_dt);
	const float angle_factor = 90.0f/(1.0f*LINE_NUM*GEARBOX);
	
	//����ʱ��
	cnt0 = TIM2->CNT;
	cnt1 = TIM5->CNT;
	
	//�����ϴξ��ԽǶ�ֵ(-180~+180��)
	encoder0.deg_last = encoder0.deg;
	encoder1.deg_last = encoder1.deg;
	
	//�����ϴ���ԽǶ�ֵ��������ϵ�ʱ�ĽǶȣ�
	encoder0.deg_rel_last = encoder0.deg_rel;
	encoder1.deg_rel_last = encoder1.deg_rel;//�����ϴ���ԽǶ�ֵ
	
	//���㱾�ξ��ԽǶ�ֵ
	//encoder0.deg =  90.0f*(cnt0 - 2.0f*LINE_NUM*GEARBOX)/(LINE_NUM*GEARBOX);//���㱾�νǶ�ֵ
	//encoder1.deg =  90.0f*(cnt1 - 2.0f*LINE_NUM*GEARBOX)/(LINE_NUM*GEARBOX);//���㱾�νǶ�ֵ
	encoder0.deg = angle_factor*(cnt0 - 2.0f*LINE_NUM*GEARBOX);//���㱾�νǶ�ֵ
	encoder1.deg = angle_factor*(cnt1 - 2.0f*LINE_NUM*GEARBOX);
	
	//���㱾����ԽǶ�ֵ
	encoder0.deg_rel += angle_factor*(cnt0 - cnt0_last + offset0);
	encoder1.deg_rel += angle_factor*(cnt1 - cnt1_last + offset1);
	
	
	//�����ٶ�(dt (ms) = dt/60000 (min))
	encoder0.rpm = rpm_factor*(cnt0 - cnt0_last + offset0);
	encoder1.rpm = rpm_factor*(cnt1 - cnt1_last + offset1);
	
	
	//����offset
	offset0 = 0;
	offset1 = 0;
	
	//��¼����ֵ
	cnt0_last = cnt0;
	cnt1_last = cnt1;
}


