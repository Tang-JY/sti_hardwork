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

//u8 RX2_BUFFER[RX2_BUFFER_LEN];//接受缓冲区
u8 TX2_BUFFER[TX2_BUFFER_LEN];//发送缓冲区

volatile u16 RX2_data_len=0;  //接收缓冲区已存储的数据长度
volatile u16 RX2_pos=0;  //接收缓冲区已读出的字节位置

volatile u16 TX2_pos=0; //发送缓冲区指针
volatile u16 TX2_len=0;//要发送的数据长度

u8 *TX2_in = TX2_BUFFER , *TX2_out = TX2_BUFFER ;
//u16 RX2_Availiable=0;

void usart2_Init(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	

	//GPIO初始化
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//USART2初始化
	USART_InitStruct.USART_BaudRate=baud;//波特率
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//输入输出
	USART_InitStruct.USART_Parity=USART_Parity_No;//无奇偶校验
	USART_InitStruct.USART_StopBits=USART_StopBits_1;//停止位1
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;//数据帧8位
	USART_Init(USART2,&USART_InitStruct);
	
		
	//GPIOA复用USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART3);
	
	USART2->CR1|=(0x0001<<5);//RXNEIE置1，启动中断
	USART2->CR1|=(0x0001<<3);//TE发送器使能
	USART2->CR1|=(0x0001<<13);//使能串口

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口2中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	
}

void usart2_SendSync(void)
{
	static DataSync_TX_TypeDef temp={0};
	const u8 head[2] = {0xde,0xad};//同步数据包数据头
	
	if(memcmp( (u8*)&temp , (u8*)&txData, sizeof(DataSync_TX_TypeDef) )){//若数据被修改过，才发送同步数据
	
		memcpy(TX2_in,head,2);
		TX2_in += 2;
		memcpy(TX2_in,(char*)&txData,sizeof(DataSync_TX_TypeDef));
		TX2_in += sizeof(DataSync_TX_TypeDef);
		USART2->CR1 |= (0x0001<<7);//传输开始，打开TXE中断
		
		memcpy((char*)&temp  ,(char*)&txData,sizeof(DataSync_TX_TypeDef));//保存本次数据以便下次比对
		PRINT(" usart2:已发送同步\r\n");
	}
}

void USART2_IRQHandler(void)//串口2中断服务程序
{
	static u8 state=0;//状态值
	static u8 temp[sizeof(DataSync_RX_TypeDef)];
	static u8 i=0;
	u8 Rec;
	
	if(USART2->SR&(0x0001<<5)){  //RXNE位为1，接收中断
	
		Rec = USART2->DR;//(USART2->DR);	//读取接收到的数据
		//数据包以0xdead开头
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
					if(i == sizeof(DataSync_RX_TypeDef)){//若数据同步完毕
						PRINT(" usart2:已接收同步\r\n");
						memcpy((char*)&rxData,temp,sizeof(DataSync_RX_TypeDef));
						i=0;
						state=0;
					}
				}
				break;
		}
  }
	
	if(USART2->SR&(0x0001<<7)){ //发送数据寄存器为空，可以发下一个字节		
		if(TX2_in - TX2_out == 0){
			USART2->CR1 &= (~(0x0001<<7));//传输完毕，关闭TXE中断
			TX2_in =  TX2_BUFFER;
			TX2_out = TX2_BUFFER;
			return;
		}
		USART2->DR = *TX2_out;//发送数据
		TX2_out++;
	}
} 
