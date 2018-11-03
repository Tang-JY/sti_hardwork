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
		
	//pwm使能
	PWM_Init(freq,min,max);
	//FH_MOtor_SetPWM(0,0);
	
	//GPIOE时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	//OTW和FAULT引脚
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;  //输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	//RESTAB和RESTCD引脚
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;  //输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;//上拉
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	//H桥使能
	GPIO_SetBits(GPIOE,GPIO_Pin_3);//RSTAB
	GPIO_SetBits(GPIOE,GPIO_Pin_5);//RSTCD	
	
	//发送信息
	memcpy(txData.msg,msg_working,16);
}



void FH_Motor_SetPWM(float pwm0,float pwm1)
{
	if(      !GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14)){
		//电机驱动过热
		memcpy(txData.msg, msg_otw, 16);
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//RSTAB
		GPIO_ResetBits(GPIOE,GPIO_Pin_5);//RSTCD
	}else if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)){
		//电机驱动错误
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
