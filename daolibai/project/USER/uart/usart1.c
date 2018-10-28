#include <string.h>
#include <stdio.h>
#include "usart1.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
// #include "delay.h"

u8 RX_BUFFER[RX_BUFFER_LEN];//接受缓冲区
u8 TX_BUFFER[TX_BUFFER_LEN];//发送缓冲区

volatile u16 RX_data_len=0;  //接收缓冲区已存储的数据长度
volatile u16 RX_pos=0;  //接收缓冲区已读出的字节位置

volatile u16 TX_pos=0; //发送缓冲区指针
volatile u16 TX_len=0;//要发送的数据长度

u8 *TX_in = TX_BUFFER , *TX_out = TX_BUFFER ;

u16 RX_Availiable = 0;


//串口1初始化
void usart1Init(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	//GPIO初始化
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//USART1初始化
	USART_InitStruct.USART_BaudRate=baud;//波特率
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//输入输出
	USART_InitStruct.USART_Parity=USART_Parity_No;//无奇偶校验
	USART_InitStruct.USART_StopBits=USART_StopBits_1;//停止位1
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;//数据帧8位
	USART_Init(USART1,&USART_InitStruct);
	
		
	//GPIOA复用USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
	
	USART1->CR1|=(0x0001<<5);//RXNEIE置1，启动中断
	USART1->CR1|=(0x0001<<3);//TE发送器使能
	USART1->CR1|=(0x0001<<13);//使能串口

	//Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	
}


//串口1中断服务程序
void USART1_IRQHandler(void)
{
	u8 Rec;

	if(USART1->SR&(0x0001<<5))  //RXNE位为1，接收中断
	{
		Rec = USART1->DR;//(USART1->DR);	//读取接收到的数据
		
		RX_BUFFER[(RX_pos+RX_data_len)%RX_BUFFER_LEN] = Rec;
		RX_data_len++;		
		RX_Availiable  = 1;//缓冲区中有数据
  }
	else if(USART1->SR&(0x0001<<7))  //发送数据寄存器为空，可以发下一个字节
	{	
//		if(TX_pos == TX_len)
//		{
//			USART1->CR1&=(~(0x0001<<7));//传输完毕，关闭TXE中断
//			TX_pos=0;
//			TX_len=0;
//			return;
//		}				
//		USART1->DR = TX_BUFFER[TX_pos];//将发送缓冲区的下一个字节写入
//		//注：TX_pos的范围是0~TX_len-1，因此TX_pos==TX_len时就说明最后一个字节发完了
//		TX_pos++;
		
		if(TX_in - TX_out == 0)
		{
			USART1->CR1&=(~(0x0001<<7));//传输完毕，关闭TXE中断
			TX_in =  TX_BUFFER;
			TX_out = TX_BUFFER;
			return;
		}
		USART1->DR = *TX_out;
		TX_out++;
	}
	return;
} 

//发送一个字节
void usart1SendByte(u8 data)
{
//	TX_BUFFER[TX_len]=data;
//	TX_len++;
	*TX_in = data;
	TX_in ++;
	USART1->CR1|=(0x0001<<7);//传输开始，打开TXE中断
	return;
}

//发送一个字符串，以\0结尾
void usart1SendStr(char* str)
{
	u16 len;
	len=strlen(str);
	//memcpy(&TX_BUFFER[TX_len], str, len);
	memcpy(TX_in, str ,len);
	//TX_len+=len;
	TX_in += len;
	USART1->CR1|=(0x0001<<7);//传输开始，打开TXE中断
	return;
}

//发送一个一定长度的数据包，不超过256字节。
void usart1SendData(u8* data,u8 N)
{
	//memcpy(&TX_BUFFER[TX_len],data,N);//将数据拷贝到发送缓冲区
	memcpy(TX_in,data,N);
	//TX_len+=N;
	TX_in += N;
	USART1->CR1|=(0x0001<<7);//传输开始，打开TXE中断
	return;
}

//读一个字节，RX_Availiable为1时再读取
u8 usart1ReadByte(void)
{
	u8 data = RX_BUFFER[RX_pos];
	
	if(RX_data_len>0)
	{
		RX_data_len--;//被读走一个数据
		RX_pos++;
		RX_pos%=RX_BUFFER_LEN;
	}
	
	if(RX_data_len==0)
	{
		RX_Availiable = 0;//缓冲区中无数据
	}	
	return data;
}


#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
int fputc(int ch, FILE *f)//输出重定向，以适用printf
{ 	
	usart1SendByte(ch);
	return ch;
}
#endif
