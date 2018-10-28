#ifndef __USART1_H
#define __USART1_H



//#define USART_DEBUG//debug宏开关
#ifdef USART_DEBUG
#define PRINT(info) {printf(info);}
#else
#define PRINT(info) {}
#endif /* USART_DEBUG */



#include "stm32f4xx.h"
#include <stdio.h>

#define RX_BUFFER_LEN 256
#define TX_BUFFER_LEN 256

extern u8 RX_BUFFER[RX_BUFFER_LEN];//接受缓冲区
extern u8 TX_BUFFER[TX_BUFFER_LEN];//发送缓冲区

extern u16 RX_Availiable; //当缓冲区内有数据时，此变量为1

extern void usart1Init(u32 baud);
extern void usart1SendByte(u8 data);
extern void usart1SendStr(char* str);
extern void usart1SendData(u8* data,u8 N);

u8 usart1ReadByte(void);
//读取缓冲区中的一个字节，在RX_Acceptable为1时再调用此函数

#endif


