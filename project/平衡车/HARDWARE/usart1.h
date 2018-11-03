#ifndef __USART1_H
#define __USART1_H

//#define USART_DEBUG//debug宏开关

#ifdef USART_DEBUG
#define PRINT(info) {printf(info);}
#else
#define PRINT(info) {}
#endif

#include "stm32f4xx.h"

#include <stdio.h>

#define RX_BUFFER_LEN 256
#define TX_BUFFER_LEN 256

extern u8 RX_BUFFER[RX_BUFFER_LEN];//接受缓冲区
extern u8 TX_BUFFER[TX_BUFFER_LEN];//发送缓冲区

extern u16 RX_Availiable; //当缓冲区内有数据时，此变量为1

void usart1_Init(u32 baud);
//串口1初始化
void usart1_SendByte(u8 data);
//发送一个字节
void usart1_SendStr(char* str);
//发送一个字符串，以\0结尾
void usart1_SendData(u8* data,u8 N);
//发送一个一定长度的数据包，不超过256字节。
void usart1_SendNum(int data);
//发送一个整数
void usart1_SendFloat(float data);
//发送一个浮点数

u8 usart1_ReadByte(void);
//读取缓冲区中的一个字节，在RX_Acceptable为1时再调用此函数
u8 usart1_ReadCmd(char*,u8);
//读取缓冲区，直到读到\r或\n，或长度达到限制

#endif


