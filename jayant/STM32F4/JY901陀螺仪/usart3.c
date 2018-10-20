#include "usart3.h"
#include "usart1.h"


volatile u8 USART3_RX_BUFFER[USART3_RX_BUFFER_LEN];//接受缓冲区
volatile u8 USART3_TX_BUFFER[USART3_TX_BUFFER_LEN];//发送缓冲区

volatile u16 USART3_RX_data_len=0;  //接收缓冲区已存储的数据长度
volatile u16 USART3_RX_pos=0;  //接收缓冲区已读出的字节位置

volatile u16 USART3_TX_pos=0; //发送缓冲区指针
volatile u16 USART3_TX_len=0;//要发送的数据长度
volatile u8 USART3_RX_Acceptable=1;

void (*usart3_IRQHandlerPtr) (u8)  =  0;//中断服务函数指针，初始为空


void usart3_Init(u32 baud)//串口1初始化
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	

	//GPIO初始化
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//USART3初始化
	USART_InitStruct.USART_BaudRate=baud;//波特率
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//输入输出
	USART_InitStruct.USART_Parity=USART_Parity_No;//无奇偶校验
	USART_InitStruct.USART_StopBits=USART_StopBits_1;//停止位1
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;//数据帧8位
	USART_Init(USART3,&USART_InitStruct);
	
		
	//GPIOD复用USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
	
	USART3->CR1|=(0x0001<<5);//RXNEIE置1，启动中断
	USART3->CR1|=(0x0001<<3);//TE发送器使能
	USART3->CR1|=(0x0001<<13);//使能串口
	
	
	//USART_Cmd(USART3,ENABLE);//使能串口
	
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启RXNEIE中断

	//Usart3 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口3中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
	return;	
}


void usart3_IRQHandlerRegister(FuncPtrTypeDef fp)//串口3中断注册函数
{
	usart3_IRQHandlerPtr=fp;
	return;
}


void USART3_IRQHandler(void)//串口3中断服务程序
{
	u8 Rec;

	if(USART3->SR&(0x0001<<5))  //RXNE位为1，接收中断
	{
		Rec = USART3->DR;//(USART3->DR);	//读取接收到的数据
		printf("usart3收到数据");
		if(usart3_IRQHandlerPtr != 0 )
		{
			usart3_IRQHandlerPtr(Rec);//调用上层文件给的函数来处理数据
		}		
//		USART3_RX_BUFFER[(USART3_RX_pos+USART3_RX_data_len)%USART3_RX_BUFFER_LEN] = Rec;
//		USART3_RX_data_len++;		
//		
//		USART3_RX_Acceptable  = 1;//缓冲区中有数据
  }
	if(USART3->SR&(0x0001<<7))  //发送数据寄存器为空，可以发下一个字节
	{
		USART3->DR = USART3_TX_BUFFER[USART3_TX_pos];//将发送缓冲区的下一个字节写入
		if(USART3_TX_pos==USART3_TX_len)
		{
			USART3->CR1&=(~(0x0001<<7));//传输完毕，关闭TXE中断
			USART3_TX_pos=0;
			USART3_TX_len=0;
			return;
		}
		USART3_TX_pos++;
	}
	return;
} 

void usart3_SendBit(u8 data)
{
	USART3->DR=data;
	return;
}

void usart3_SendStr(char str[])
{
	for(USART3_TX_len=0 ; USART3_TX_len<USART3_TX_BUFFER_LEN ; USART3_TX_len++ , str++)
	{
		USART3_TX_BUFFER[USART3_TX_len]=*str;
		if(*str=='\0')
		{
			break;//字符串已写入发送缓冲区
		}
	}
	USART3->CR1|=(0x0001<<7);//传输开始，打开TXE中断
	return;
}

void usart3_SendDatas(u8* data,u16 n)
{
	for(USART3_TX_len=0 ; (USART3_TX_len<USART3_TX_BUFFER_LEN)&&(USART3_TX_len<n) ; USART3_TX_len++ , data++)
	{
		USART3_TX_BUFFER[USART3_TX_len]=*data;
	}
	USART3->CR1|=(0x0001<<7);//传输开始，打开TXE中断
	return;

}


u8 usart3_ReadBit(void)
{
	u8 data = USART3_RX_BUFFER[USART3_RX_pos];
	
	if(USART3_RX_data_len>0)
	{
		USART3_RX_data_len--;//被读走一个数据
		USART3_RX_pos++;
		USART3_RX_pos%=USART3_RX_BUFFER_LEN;
	}
	if(USART3_RX_data_len==0)
	{
		USART3_RX_Acceptable = 0;//缓冲区中无数据
	}	
	return data;
}
