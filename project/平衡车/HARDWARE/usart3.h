#ifndef __USART3_H
#define __USART3_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"


#define USART3_RX_BUFFER_LEN 1024
#define USART3_TX_BUFFER_LEN 1024


typedef void (*FuncPtrTypeDef) (u8);
   
extern volatile u8 USART3_RX_BUFFER[USART3_RX_BUFFER_LEN];//���ܻ�����
extern volatile u8 USART3_TX_BUFFER[USART3_TX_BUFFER_LEN];//���ͻ�����
extern volatile u8 USART3_RX_Acceptable; //����������������ʱ���˱���Ϊ1

void usart3_Init(u32);//����3��ʼ��
void usart3_SendBit(u8);//����һ���ַ�
void usart3_SendStr(char[]);//����һ���ַ�������\0��β
u8 usart3_ReadBit(void);//��ȡ�������е�һ���ֽڣ���RX_AcceptableΪ1ʱ�ٵ��ô˺���
void usart3_SendDatas(u8*,u16);

void usart3_IRQHandlerRegister(FuncPtrTypeDef fp);//�ж�ע�ắ��


#endif

