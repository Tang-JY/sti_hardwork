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
   
extern volatile u8 USART3_RX_BUFFER[USART3_RX_BUFFER_LEN];//接受缓冲区
extern volatile u8 USART3_TX_BUFFER[USART3_TX_BUFFER_LEN];//发送缓冲区
extern volatile u8 USART3_RX_Acceptable; //当缓冲区内有数据时，此变量为1

void usart3_Init(u32);//串口3初始化
void usart3_SendBit(u8);//发送一个字符
void usart3_SendStr(char[]);//发送一个字符串，以\0结尾
u8 usart3_ReadBit(void);//读取缓冲区中的一个字节，在RX_Acceptable为1时再调用此函数
void usart3_SendDatas(u8*,u16);

void usart3_IRQHandlerRegister(FuncPtrTypeDef fp);//中断注册函数


#endif

