#include "usart3.h"
#include "usart1.h"


volatile u8 USART3_RX_BUFFER[USART3_RX_BUFFER_LEN];//���ܻ�����
volatile u8 USART3_TX_BUFFER[USART3_TX_BUFFER_LEN];//���ͻ�����

volatile u16 USART3_RX_data_len=0;  //���ջ������Ѵ洢�����ݳ���
volatile u16 USART3_RX_pos=0;  //���ջ������Ѷ������ֽ�λ��

volatile u16 USART3_TX_pos=0; //���ͻ�����ָ��
volatile u16 USART3_TX_len=0;//Ҫ���͵����ݳ���
volatile u8 USART3_RX_Acceptable=1;

void (*usart3_IRQHandlerPtr) (u8)  =  0;//�жϷ�����ָ�룬��ʼΪ��


void usart3_Init(u32 baud)//����1��ʼ��
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//ʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	

	//GPIO��ʼ��
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//USART3��ʼ��
	USART_InitStruct.USART_BaudRate=baud;//������
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;//�������
	USART_InitStruct.USART_Parity=USART_Parity_No;//����żУ��
	USART_InitStruct.USART_StopBits=USART_StopBits_1;//ֹͣλ1
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;//����֡8λ
	USART_Init(USART3,&USART_InitStruct);
	
		
	//GPIOD����USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
	
	USART3->CR1|=(0x0001<<5);//RXNEIE��1�������ж�
	USART3->CR1|=(0x0001<<3);//TE������ʹ��
	USART3->CR1|=(0x0001<<13);//ʹ�ܴ���
	
	
	//USART_Cmd(USART3,ENABLE);//ʹ�ܴ���
	
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//����RXNEIE�ж�

	//Usart3 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//����3�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
	return;	
}


void usart3_IRQHandlerRegister(FuncPtrTypeDef fp)//����3�ж�ע�ắ��
{
	usart3_IRQHandlerPtr=fp;
	return;
}


void USART3_IRQHandler(void)//����3�жϷ������
{
	u8 Rec;

	if(USART3->SR&(0x0001<<5))  //RXNEλΪ1�������ж�
	{
		Rec = USART3->DR;//(USART3->DR);	//��ȡ���յ�������
		printf("usart3�յ�����");
		if(usart3_IRQHandlerPtr != 0 )
		{
			usart3_IRQHandlerPtr(Rec);//�����ϲ��ļ����ĺ�������������
		}		
//		USART3_RX_BUFFER[(USART3_RX_pos+USART3_RX_data_len)%USART3_RX_BUFFER_LEN] = Rec;
//		USART3_RX_data_len++;		
//		
//		USART3_RX_Acceptable  = 1;//��������������
  }
	if(USART3->SR&(0x0001<<7))  //�������ݼĴ���Ϊ�գ����Է���һ���ֽ�
	{
		USART3->DR = USART3_TX_BUFFER[USART3_TX_pos];//�����ͻ���������һ���ֽ�д��
		if(USART3_TX_pos==USART3_TX_len)
		{
			USART3->CR1&=(~(0x0001<<7));//������ϣ��ر�TXE�ж�
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
			break;//�ַ�����д�뷢�ͻ�����
		}
	}
	USART3->CR1|=(0x0001<<7);//���俪ʼ����TXE�ж�
	return;
}

void usart3_SendDatas(u8* data,u16 n)
{
	for(USART3_TX_len=0 ; (USART3_TX_len<USART3_TX_BUFFER_LEN)&&(USART3_TX_len<n) ; USART3_TX_len++ , data++)
	{
		USART3_TX_BUFFER[USART3_TX_len]=*data;
	}
	USART3->CR1|=(0x0001<<7);//���俪ʼ����TXE�ж�
	return;

}


u8 usart3_ReadBit(void)
{
	u8 data = USART3_RX_BUFFER[USART3_RX_pos];
	
	if(USART3_RX_data_len>0)
	{
		USART3_RX_data_len--;//������һ������
		USART3_RX_pos++;
		USART3_RX_pos%=USART3_RX_BUFFER_LEN;
	}
	if(USART3_RX_data_len==0)
	{
		USART3_RX_Acceptable = 0;//��������������
	}	
	return data;
}
