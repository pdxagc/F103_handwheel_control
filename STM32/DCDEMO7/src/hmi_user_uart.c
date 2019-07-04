/************************************��Ȩ����********************************************
--------------------------------------------------------------------------------------
ʹ�ñض�
hmi_user_uart.c�еĴ��ڷ��ͽ��պ�����3��������

���ڳ�ʼ��Uartinti()��
����1���ֽ�SendChar()��
�����ַ���SendStrings().

����ֲ������ƽ̨����Ҫ�޸ĵײ�Ĵ�������,����ֹ�޸ĺ������ƣ������޷���HMI������(hmi_driver.c)ƥ�䡣
---------------------------------------------------------------------------------------*/


#include "hmi_user_uart.h"
#include "timer.h"
#include "cJSON.h"
#include "tft.h"
#include "string.h"
#include "stm32f10x_gpio.h"
#include "ulitity.h"


#define USART3_REC_LEN 2048
#define Mac_address 0
#define PACKAGELEN 30
#define CMD_Length 8
#define PULSE_DIV 4

extern int16 Pulses_counter;
extern uint8 Override_num;
extern uint8  TX_Data [30]; //the sending package
extern uint8  RX_Data [30]; //the receiving package
extern uint8  USART3_RX_STA;
extern Control_Panel_Pram control_panel_pram;

#define My_Address 2
uint8  RX_Busy=0;
uint8  HC_Address, rxcounter, remaincounter; //this unit is this unit address, if change to master become 0000 
uint8  ready2send; // bit is 1 while there is command to send
uint8  ready2read;
int16  last_time_Pulses_number;    //��¼��һ������
uint8  last_Override;  //��¼���ʱ仯
uint8  CMD_last_button;       //��¼�����仯


/*******************************************************************************  
* �� �� ��         : Usart1_Init(��TFT��ͨѶ)  
* ��������         : IO�˿ڼ�����1��ʱ�ӳ�ʼ������    A9,A10    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/  
void Usart1_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //����һ���ṹ�������������ʼ��GPIO   
    USART_InitTypeDef USART_InitStructure;    //����һ���ṹ�������������ʼ������
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;                     //USART1_TX PA.9
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //�����������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;            
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //��ʼ�� GPIOA.9

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                    //USART1_RX PA.10
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	  //���ڲ�����ʼ�� /* USART configuration */
    USART_DeInit(USART1);                                          //��λUSART1
    USART_InitStructure.USART_BaudRate = BaudRate;                 //����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //�ֳ�Ϊ 8 λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;            //����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //�շ�ģʽ 
    USART_Init(USART1, &USART_InitStructure);                      //��ʼ������         
		
		
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                  //�����ж�ʹ��

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);                                     //ʹ�ܴ���
}


/*******************************************************************************  
* �� �� ��         : Usart3_Init(������ͨѶ)
* ��������         : IO�˿ڼ�����3��ʱ�ӳ�ʼ������    PB10,PB11    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 

void Usart3_Init(uint32 BaudRate)
	{
		
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3|RCC_APB2Periph_AFIO, ENABLE); 
		
		//USART3_TX   PB10      //TX-485
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);
 
    //USART3_RX	  PB11      //RX-485  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		//��ʼ��485���ͻ����ʹ������ PA1
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
   //Usart3 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        //�жϺţ�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //��Ӧ���ȼ�0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //�����ж�
		NVIC_Init(&NVIC_InitStructure);	                         //�ж����ȼ���ʼ��

 
		USART_InitStructure.USART_BaudRate =BaudRate;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;            //�ֳ�Ϊ9λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;                 //һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;                    //����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //��Ӳ������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	       //�շ�ģʽ
    USART_Init(USART3, &USART_InitStructure);       
		

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //�����ж�ʹ��
    USART_Cmd(USART3, ENABLE);    
    
    RS485_TX_Set(0);		//ʹ��485����ģʽ
}



void USART3_IRQHandler(void)
{
	if(USART3->SR &1<<3)
	{
		uint8_t i;
		i=USART3->SR;
		i=USART3->DR;
		return;
	}
	Usart3_Recieve_ISR_Process();
}


//RS485 ģʽ����.en:0,����;1,����.
void RS485_TX_Set(uint8 en)
{
	if(en)
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}
 
/***************************************************************************
*   \name   SendChar()����������USART1ר�ã�
*   \brief  ����1���ֽ� 
*   \param  t ���͵��ֽ�
*****************************************************************************/
void  SendChar(uchar t)     
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //�ȴ�����ת�Ƶ���λ�Ĵ���
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//�ȴ����ڷ������
}


/***************************************************************************
*   \brief  ����һ���ַ� 
*   \param  ch ���͵��ֽ�
*****************************************************************************/
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
 {
 
    USART_SendData(pUSARTx,ch);                                    /* ����һ���ֽ����ݵ� USART */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* �ȴ��������ݼĴ���Ϊ�� */
 }
 
 //Usart3_send_Str ������������
 void Usart3_send_Str(uint8 buf[])
 {
	 uint8 i=0;
	 while(i<10)
	 {
		 Usart_SendByte( USART3, buf[i] );
			i++;
	    while (USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); /* �ȴ�������� */	 
	 }
 
 }
// ���͵��ַ���
 void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
 {
		//unsigned int k=0;
		while (*str !='\0')
		{
			Usart_SendByte( pUSARTx, *str );
			str++;
		} 
	  while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); /* �ȴ�������� */
 }
 

 // ��������ǰ������һ��У�����	
char SetXor	(char length, char start)
{
	char i, buf;;

	buf=TX_Data[start];
	for (i=start+1; i<=(length); i++)
		buf=buf^TX_Data[i];
	return buf;
}

//�������ݺ�����һ�����������ǰ������Աȣ�һ�µĻ�����յ�������ȷ
unsigned char CheckXor (char data, unsigned char len)
{
	char buf1, buf;
	char i;

	buf1=RX_Data[len];
	buf=RX_Data[0];
	for (i=1; i<(len); i++)
	{
		buf= buf^RX_Data[i];
	}
	if (buf1==buf)
		return 1;
	else
		return 0;
}

//��ַУ��
uint8_t Check_Address (char data)
{
	unsigned char buf;
	buf = data&(0x1f);//0b0001 1111;
	if (buf==My_Address)	
		return 1;
	else
		return	0;
}

   
// ����3�����ж�ִ�к���  //place at ISR�������������͵�����
void Usart3_Recieve_ISR_Process (void)
{ 	
	 unsigned char RX_Buffer;
	 short dat; //16 bit
	 
	 dat=USART3->DR;
	 USART3->SR&=~(1<<5);  //SR�Ĵ����ĵ���λ��0
//	 i=USART3->DR;
//	 i=USART3->SR;
	 RX_Buffer=dat;
	 if(dat&(1<<8))
	 {
		 if (Check_Address(RX_Buffer))
	   {
	  		rxcounter=0;
	  		RX_Busy=1;
	   }
	 }
   else if(RX_Busy==1)
	 {
			if(rxcounter >= PACKAGELEN ) 
			{
				RX_Busy=0;
				return; 
			}
	  	RX_Data[rxcounter]=RX_Buffer;   //RX_Data[0]�洢���ݳ���
	  	rxcounter++;
	  	if(RX_Data[0]>PACKAGELEN)  //���ݴ�������30���ֽڣ������쳣
	  	{ 
	  	 	RX_Busy=0;//�ȴ���һ����ʼָ��
	  	}
	  	if(rxcounter>RX_Data[0])    //���������
	  	{
	  		//check package xor
	  		RX_Busy=0;
	  		if(CheckXor(RX_Data[rxcounter-1],RX_Data[0]))
	  		{
	  			ready2read=1; //У��ͨ��									
//					if(RX_Data[1] == CMD_ASK_SLAVE)
//					{
//						 Usart3_Send_Data(10);
//					}

	  		}
	 	   }
	  }

}


//����2�������ݴ�����
void Usart3_Data_handle (void)
{

	uint8 command;
	uint16 Recdata1,Recdata2;
	if(ready2read)        //����ͨ��
	{
		command=RX_Data[1];
		switch(command)
		{
			case CMD_ASK_SLAVE:           //��������
				Usart3_Send_Data(10);
			break;			
			case CMD_UPDATE_MACH3_NUMBER: //���յ�����  
			{
				Recdata1=RX_Data[2];
				Recdata2=RX_Data[4];
				control_panel_pram.X_value = (int16)(Recdata1<<8)+RX_Data[3]+((int16)((Recdata2<<8)+RX_Data[5]))*0.001;
				Recdata1=RX_Data[6];
				Recdata2=RX_Data[8];
				control_panel_pram.Y_value = (int16)(Recdata1<<8)+RX_Data[7]+((int16)((Recdata2<<8)+RX_Data[9]))*0.001;
				Recdata1=RX_Data[10];
				Recdata2=RX_Data[12];
				control_panel_pram.Z_value = (int16)(Recdata1<<8)+RX_Data[11]+((int16)((Recdata2<<8)+RX_Data[13]))*0.001;
				Recdata1=RX_Data[14];
				Recdata2=RX_Data[16];
				control_panel_pram.A_value = (int16)(Recdata1<<8)+RX_Data[15]+((int16)((Recdata2<<8)+RX_Data[17]))*0.001;
				
        				
			}break;
			
		}
		ready2read=0;
	}


}

// ����2��������
void Usart3_Send_Data (uint8 length)
{ 
	uint8 i,x,y,z;
	x = Check_Pulses_change();
	y = Check_CMD_button_change();
	z = Check_Override_change();
	if(x || y || z)
	{
		Create_CMD_and_Date();  //��������������ָ�������
		RS485_TX_Set(1);
		//delay_ms(1);
		for(i=0;i<length;i++)
		{
			if (i==0)
			{
				USART3->DR =(uint32_t)(1<<8)|TX_Data[0];
				while((USART3->SR&0X40)==0);
			}
			else
			{
				USART3->DR = TX_Data[i];
				while((USART3->SR&0X40)==0);
			};//ѭ������,ֱ���������   
			}
		RS485_TX_Set(0);
	}
}


//�ж������Ƿ����仯
uint8 Check_Pulses_change(void)
{
	uint8 result;
	if(last_time_Pulses_number==Pulses_counter)
	{
	  result = 0;
	}
	else 
	{
	  last_time_Pulses_number=Pulses_counter;
		result = 1;	
	}
	return result;
}

//�жϰ����Ƿ����仯
uint8 Check_CMD_button_change(void)
{
	uint8 result;
	if(CMD_last_button==control_panel_pram.Press_button)
	{
		control_panel_pram.Press_button = 0XFF;
	  result = 0;
	}
	else 
	{
	  CMD_last_button=control_panel_pram.Press_button;
		result = 1;	
	}
	return result;
}

//�жϱ����Ƿ����仯
uint8 Check_Override_change(void)
{
	uint8 result;
	if(last_Override==Override_num)
	{
	  result = 0;
	}
	else 
	{
	  last_Override=Override_num;
		result = 1;	
	}
	return result;
}

//ȷ�����ĸ���ѡ��
uint8 Axis_Gets()
{
	uint8 axis;
	switch(control_panel_pram.Axis_press)
	{
		case CMD_X_AXIS: axis=10;break;
	  case CMD_Y_AXIS: axis=11;break;
		case CMD_Z_AXIS: axis=12;break;
		case CMD_A_AXIS: axis=13;break;
		case CMD_B_AXIS: axis=14;break;
	}
	
	return axis;
}

//��������������ָ�������
//  address  length command   mpgmsb  mpglsb  multiplierlsb   key   axis  div     xorcheck
// bit 0     1       2         3       4       5               6    7      8       9

//���ʣ�x1 x2 x5 x10 x20 ��Ч
//0-16 ���� 50-66�������� 0xff��Ч
//����ģʽ 10:x�� 11:y�� 12:z 13:a��
void Create_CMD_and_Date(void)
{
	
			TX_Data[0] = Mac_address;
			TX_Data[1] = CMD_Length;
			TX_Data[2] = CMD_RPY_HC_MPG1;
			TX_Data[3] = Pulses_counter>>8;
			TX_Data[4] = Pulses_counter;
			TX_Data[5] = Override_num;
			TX_Data[6] = control_panel_pram.Press_button;
			TX_Data[7] = Axis_Gets();
			TX_Data[8] = 4;
			TX_Data[9] = SetXor(TX_Data[1],1);	
		
}

