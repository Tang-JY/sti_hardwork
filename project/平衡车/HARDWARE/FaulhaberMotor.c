#include "FaulhaberMotor.h"

#include "encoder.h"
#include "usart2.h"

#include <math.h>
#include <string.h>

const char msg_fault[16]   = "Fault";
const char msg_otw[16]     = "OTW";
const char msg_working[16] = "Working";


//_FaulhaberMotor_TypeDef motor0,motor1;
void FH_Motor_Init(u32 freq,float min,float max)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	//pwmʹ��
	PWM_Init(freq,min,max);
	//FH_MOtor_SetPWM(0,0);
	
	//GPIOEʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	//OTW��FAULT����
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;  //����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	//RESTAB��RESTCD����
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;  //���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;//����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//��©
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//H��ʹ��
	GPIO_SetBits(GPIOE,GPIO_Pin_3);//RSTAB
	GPIO_SetBits(GPIOE,GPIO_Pin_5);//RSTCD	
	
	//������Ϣ
	memcpy(txData.msg,msg_working,16);
}



void FH_Motor_SetPWM(float pwm0,float pwm1)
{
	if(      !GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14)){
		//�����������
		memcpy(txData.msg, msg_otw, 16);
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//RSTAB
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);//RSTCD
	}else if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)){
		//�����������
		memcpy(txData.msg, msg_fault  , 16);
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//RSTAB
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);//RSTCD		
	}else{
		//memcpy(txData.msg, msg_working, 16);		
		GPIO_SetBits(GPIOE,GPIO_Pin_3);//RSTAB
		GPIO_SetBits(GPIOE,GPIO_Pin_5);//RSTCD
		
		if(pwm0>=0){
			Set_PWM_DutyRatio(1, 0);
			Set_PWM_DutyRatio(2, pwm0);
		}else{
			Set_PWM_DutyRatio(2, 0);
			Set_PWM_DutyRatio(1, fabs(pwm0));	
		}
		
		if(pwm1>=0){
			Set_PWM_DutyRatio(4,0);
			Set_PWM_DutyRatio(3,pwm1);	
		}else{
			Set_PWM_DutyRatio(3,0);
			Set_PWM_DutyRatio(4,fabs(pwm1));	
		}
	}
}
