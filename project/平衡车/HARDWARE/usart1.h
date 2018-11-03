#ifndef __USART1_H
#define __USART1_H

//#define USART_DEBUG//debug�꿪��

#ifdef USART_DEBUG
#define PRINT(info) {printf(info);}
#else
#define PRINT(info) {}
#endif

#include "stm32f4xx.h"

#include <stdio.h>

#define RX_BUFFER_LEN 256
#define TX_BUFFER_LEN 256

extern u8 RX_BUFFER[RX_BUFFER_LEN];//���ܻ�����
extern u8 TX_BUFFER[TX_BUFFER_LEN];//���ͻ�����

extern u16 RX_Availiable; //����������������ʱ���˱���Ϊ1

void usart1_Init(u32 baud);
//����1��ʼ��
void usart1_SendByte(u8 data);
//����һ���ֽ�
void usart1_SendStr(char* str);
//����һ���ַ�������\0��β
void usart1_SendData(u8* data,u8 N);
//����һ��һ�����ȵ����ݰ���������256�ֽڡ�
void usart1_SendNum(int data);
//����һ������
void usart1_SendFloat(float data);
//����һ��������

u8 usart1_ReadByte(void);
//��ȡ�������е�һ���ֽڣ���RX_AcceptableΪ1ʱ�ٵ��ô˺���
u8 usart1_ReadCmd(char*,u8);
//��ȡ��������ֱ������\r��\n���򳤶ȴﵽ����

#endif


