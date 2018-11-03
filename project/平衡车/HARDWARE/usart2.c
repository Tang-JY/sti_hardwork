#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "usart2.h"
#include <string.h>
#include "usart1.h"

//DataSync_TX_TypeDef *dataSync_TX;
//DataSync_RX_TypeDef *dataSync_RX;

DataSync_TX_TypeDef txData;
DataSync_RX_TypeDef rxData;

//u8 RX2_BUFFER[RX2_BUFFER_LEN];//���ܻ�����
u8 TX2_BUFFER[TX2_BUFFER_LEN];//���ͻ�����

volatile u16 RX2_data_len=0;  //���ջ������Ѵ洢�����ݳ���
volatile u16 RX2_pos=0;  //���ջ������Ѷ������ֽ�λ��

volatile u16 TX2_pos=0; //���ͻ�����ָ��
volatile u16 TX2_len=0;//Ҫ���͵����ݳ���

u8 *TX2_in = TX2_BUFFER , *TX2_out = TX2_BUFFER ;
//u16 RX2_Availiable=0;

void usart2_Init(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//ʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	

	//GPIO��ʼ��
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//USART2��ʼ��
	USART_InitStruct.USART_BaudRate=baud;//������
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//�������
	USART_InitStruct.USART_Parity=USART_Parity_No;//����żУ��
	USART_InitStruct.USART_StopBits=USART_StopBits_1;//ֹͣλ1
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;//����֡8λ
	USART_Init(USART2,&USART_InitStruct);
	
		
	//GPIOA����USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART3);
	
	USART2->CR1|=(0x0001<<5);//RXNEIE��1�������ж�
	USART2->CR1|=(0x0001<<3);//TE������ʹ��
	USART2->CR1|=(0x0001<<13);//ʹ�ܴ���

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����2�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	
}

void usart2_SendSync(void)
{
	static DataSync_TX_TypeDef temp={0};
	const u8 head[2] = {0xde,0xad};//ͬ�����ݰ�����ͷ
	
	if(memcmp( (u8*)&temp , (u8*)&txData, sizeof(DataSync_TX_TypeDef) )){//�����ݱ��޸Ĺ����ŷ���ͬ������
	
		memcpy(TX2_in,head,2);
		TX2_in += 2;
		memcpy(TX2_in,(char*)&txData,sizeof(DataSync_TX_TypeDef));
		TX2_in += sizeof(DataSync_TX_TypeDef);
		USART2->CR1 |= (0x0001<<7);//���俪ʼ����TXE�ж�
		
		memcpy((char*)&temp  ,(char*)&txData,sizeof(DataSync_TX_TypeDef));//���汾�������Ա��´αȶ�
		PRINT(" usart2:�ѷ���ͬ��\r\n");
	}
}

void USART2_IRQHandler(void)//����2�жϷ������
{
	static u8 state=0;//״ֵ̬
	static u8 temp[sizeof(DataSync_RX_TypeDef)];
	static u8 i=0;
	u8 Rec;
	
	if(USART2->SR&(0x0001<<5)){  //RXNEλΪ1�������ж�
	
		Rec = USART2->DR;//(USART2->DR);	//��ȡ���յ�������
		//���ݰ���0xdead��ͷ
		switch(Rec){
			case 0xde:
				
				if(state == 0){
					state++;
					break;
				}
				
			case 0xad:
				
				if((state==1) && (Rec==0xad)){
					state++;
					break;
				}
			
			default:
				
				if((i < sizeof(DataSync_RX_TypeDef))  &&  (state == 2)){
					temp[i++]=Rec;					
					if(i == sizeof(DataSync_RX_TypeDef)){//������ͬ�����
						PRINT(" usart2:�ѽ���ͬ��\r\n");
						memcpy((char*)&rxData,temp,sizeof(DataSync_RX_TypeDef));
						i=0;
						state=0;
					}
				}
				break;
		}
  }
	
	if(USART2->SR&(0x0001<<7)){ //�������ݼĴ���Ϊ�գ����Է���һ���ֽ�		
		if(TX2_in - TX2_out == 0){
			USART2->CR1 &= (~(0x0001<<7));//������ϣ��ر�TXE�ж�
			TX2_in =  TX2_BUFFER;
			TX2_out = TX2_BUFFER;
			return;
		}
		USART2->DR = *TX2_out;//��������
		TX2_out++;
	}
} 
