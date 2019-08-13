/*
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
#include "hw_config.h"
#include "stdio.h"
#include "hmi_driver.h"
#include "cmd_queue.h"


#define USART3_REC_LEN 2048
#define Mac_address 0
#define PACKAGELEN 30
#define CMD_Length 8
#define PULSE_DIV 4

#define My_Address 2
extern uint16 Pulses_counter;
extern uint8 Multiple_num;
extern uint8  TX_Data [30]; //the sending package
extern uint8  RX_Data [30]; //the receiving package
extern Control_Panel_Pram control_panel_pram;
extern uint8  Press_button;          //��¼�ĸ���ť����(��Ҫ�Ѱ������͸���̻�)
extern uint16 master_ask,master_send;


uint8  RX_Busy=0;
uint8  HC_Address, rxcounter, remaincounter; //this unit is this unit address, if change to master become 0000 
uint8  ready2send; // bit is 1 while there is command to send
uint8  ready2read;
uint16 last_time_Pulses_number=0;  //��¼��һ������
uint8  last_time_Multiple=0;       //��¼���ʱ仯
uint8  last_time_button=0;         //��¼�����仯


/*******************************************************************************  
* �� �� ��         : Usart1_Init(���̻�ͨѶ)  
* ��������         : IO�˿ڼ�����1��ʱ�ӳ�ʼ������    A9,A10    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/  
void Usart1_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //����һ���ṹ�������������ʼ��GPIO   
    USART_InitTypeDef USART_InitStructure;    //����һ���ṹ�������������ʼ������
	  NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;                     //USART1_TX PA.9   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //�����������	
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //��ʼ�� GPIOA.9

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                    //USART1_RX PA.10
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                        //��ʼ�� GPIOA.10
	  //���ڲ�����ʼ�� /* USART configuration */
	
	 //��ʼ��485ʹ������ PA7
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //�������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
    USART_DeInit(USART1);                                          //��λUSART1
    USART_InitStructure.USART_BaudRate = BaudRate;                 //����������
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;    //�ֳ�Ϊ9 λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;            //����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //�շ�ģʽ 
    USART_Init(USART1, &USART_InitStructure);                      //��ʼ������    

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;        //�жϺţ�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		   //��Ӧ���ȼ�0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //�����ж�
		NVIC_Init(&NVIC_InitStructure);	
		
		
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                  //�����ж�ʹ��

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);                                     //ʹ�ܴ���
		RS485_mode_control(0);		                                     //ʹ��485����ģʽ
}


/*******************************************************************************  
* �� �� ��         : Usart2_Init(��TFT��ͨѶ)
* ��������         : IO�˿ڼ����ڣ�ʱ�ӳ�ʼ������    PA2,PA3    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 
void Usart2_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //����һ���ṹ�������������ʼ��GPIO   
    USART_InitTypeDef USART_InitStructure;    //����һ���ṹ�������������ʼ������
	  NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;                     //USART1_TX PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //�����������	
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //��ʼ�� GPIOA.2

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                    //USART1_RX PA.3
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);                        //��ʼ�� GPIOA.3
	
	  //���ڲ�����ʼ�� /* USART configuration */
    USART_DeInit(USART2);                                          //��λUSART2
    USART_InitStructure.USART_BaudRate = BaudRate;                 //����������
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //�ֳ�Ϊ 8 λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;            //����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //�շ�ģʽ 
    USART_Init(USART2, &USART_InitStructure);                      //��ʼ������    

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;        //�жϺţ�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		   //��Ӧ���ȼ�0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //�����ж�
		NVIC_Init(&NVIC_InitStructure);	
		
		
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);                  //�����ж�ʹ��

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);                                     //ʹ�ܴ���
}

/*******************************************************************************  
* �� �� ��         : Usart3_Init()
* ��������         : IO�˿ڼ�����3��ʱ�ӳ�ʼ������    PB10,PB11    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 

void Usart3_Init(uint32 BaudRate)
	{
		
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
		
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
		
		
 
   //Usart3 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        //�жϺţ�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //��Ӧ���ȼ�0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //�����ж�
		NVIC_Init(&NVIC_InitStructure);	                         //�ж����ȼ���ʼ��

 
		USART_InitStructure.USART_BaudRate =BaudRate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;            //�ֳ�Ϊ8λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;                 //һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;                    //����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //��Ӳ������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	       //�շ�ģʽ
    USART_Init(USART3, &USART_InitStructure);       
		

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);     //�����ж�ʹ��
    USART_Cmd(USART3, ENABLE);    
    
    
}

//����1�жϺ���
void USART1_IRQHandler(void)
{
	if(USART1->SR &1<<3)
	{
		uint8_t i;
		i=USART1->SR;
		i=USART1->DR;
		return;
	}
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
	     Usart1_Recieve_Process();  //��������
  }
}

//����2�жϺ���
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART2);
        queue_push(data);
    }

}




//RS485 ģʽ����.en:0,����;1,����.
void RS485_mode_control(uint8 en)
{
	if(en)
		GPIO_SetBits(GPIOA, GPIO_Pin_7);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_7);
}
 
/*
*   \name   SendChar()����������USART2ר�ã�
*   \brief  ����1���ֽ� 
*   \param  t ���͵��ֽ�
*/
void  SendChar(uchar t)     
{
    USART_SendData(USART2,t);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); //�ȴ�����ת�Ƶ���λ�Ĵ���
    while((USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET));//�ȴ����ڷ������
}



 
//Usart3_send_Str ������������
//void Usart3_send_Str(uint8 buf[])
// {
//	 uint8 i=0;
//	 while(i<10)
//	 {
//		 Usart_SendByte( USART3, buf[i] );
//			i++;
//	    while (USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); /* �ȴ�������� */	 
//	 }
// 
// }
///***************************************************************************
//*   \brief  ����һ���ֽ�
//*   \param  ch ���͵��ֽ�
//*****************************************************************************/
// void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
// {
// 
//    USART_SendData(pUSARTx,ch);                                    /* ����һ���ֽ����ݵ� USART */
//    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* �ȴ��������ݼĴ���Ϊ�� */
// }
////���͵��ַ���
// void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
// {
//		//unsigned int k=0;
//		while (*str !='\0')
//		{
//			Usart_SendByte( pUSARTx, *str );
//			str++;
//		} 
//	  while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); /* �ȴ�������� */
// }
// 

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
	buf = data&(0x1f);    //0b0001 1111;
	if (buf==My_Address)	
		return 1;
	else
		return	0;
}

   
// ����1�����������͵����� //place at ISR
void Usart1_Recieve_Process (void)
{ 	
	 //uint16 master_ask=0,master_send=0;
	 uint8 rec_command; 
	 char buf4[20];
	 uchar RX_Buffer;
	 short dat; //16 bit	 
	 dat=USART1->DR;
	 USART1->SR&=~(1<<5);  //SR�Ĵ����ĵ���λ��0
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
					rec_command=RX_Data[1];
					if(rec_command==CMD_ASK_SLAVE)
					{
						master_ask++;
						sprintf(buf4,"master_ask:%u",master_ask);	
		        SetTextValue(3,29,(uchar *)buf4);	
						
					}
//					else if(rec_command==CMD_UPDATE_MACH3_NUMBER)
//					{
//						master_send++;	
//            sprintf(buf4,"master_send:%u",master_send);	
//			  	  SetTextValue(3,49,(uchar *)buf4);
//					}						

	  		}
	 	   }
	  }

}
/*
// COMMAND TABLE
#define CMD_ASK_SLAVE 29                 //������������
#define CMD_RPY_HC_MPG1	36               //���ַ�������
#define CMD_UPDATE_MACH3_NUMBER	41       //����������������
#define CMD_Working_File_Name 50         //�ӹ��ļ���
#define CMD_Work_line 51                 //�ӹ�����
#define CMD_Working_Code 52              //�ӹ�����
#define CMD_SPin_Speed 53                //�����ٶ�
#define CMD_Working_Speed 54             //�ӹ��ٶ�
#define CMD_Warn_Massage  55             //������Ϣ
#define CMD_Clond_File_Name 56           //�ƿռ��ļ���
#define CMD_SD_Card_File_Name 57         //SD���ļ���
#define CMD_Storage_Data  58             //�ڴ�����
#define CMD_Wifi_Name  59                //wifi����
#define CMD_Wifi_Password 60             //wif����
#define CMD_Account_Name  61             //�û��˻�
#define CMD_Account_Password 62          //�û�����
*/

//����1�������ݺ󣬶����ݽ��д���
void Communication_Data_handle (void)
{

	uint8 command;
	uint16 Recdata1,Recdata2;
	if(ready2read)                    //����ͨ�����յ�������������
	{
		command=RX_Data[1];            //��ȡ���յ�����
		switch(command)
		{
			case CMD_ASK_SLAVE:           //�����������ݣ�����Ҫ��������������
				Usart1_Send_Data(10);
			break;			
			case CMD_UPDATE_MACH3_NUMBER: //���յ�������������  
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
			case CMD_Working_File_Name:    //�ӹ��ļ���
			{
			
			}break;
			case CMD_Work_line:             //�ӹ�����
			{
			
			}break;
			case CMD_Working_Code:             //�ӹ�����
			{
			
			}break;
			case CMD_SPin_Speed:               //�����ٶ�
			{
			
			}break;
			case CMD_Working_Speed:            //�ӹ��ٶ�
			{
			
			}break;
			case CMD_Warn_Massage:             //������Ϣ
			{  
			
			}break;	
			case CMD_Clond_File_Name:          //�ƿռ��ļ���
			{
			
			}break;
			case CMD_SD_Card_File_Name:         //SD���ļ���
			{
			
			}break;
		  case 	CMD_Storage_Data:            //�ڴ�����
			{  
			
			}break;
			case CMD_Wifi_Name:               //wifi����
			{
			
			}break;
			
		}
		ready2read=0;
	}


}

////��������������
//void Send_data_to_Master(void)
//{
//	if(USART1->SR &1<<3)
//	{
//		uint8_t i;
//		i=USART1->SR;
//		i=USART1->DR;
//		return;
//	}
//   Usart1_Send_Data(10);
//}





//�ж������Ƿ����仯
uint8 Check_Pulses_change(void)
{
	if(last_time_Pulses_number != Pulses_counter)
	{
		last_time_Pulses_number = Pulses_counter;
	  return 1;
	}
	else 
	{  
		return  0;	
	}
}

//�жϰ����Ƿ����仯
uint8 Check_CMD_button_change(void)
{
	if(last_time_button==Press_button)
	{
		Press_button = 0XFF;
	  return 0;
	}
	else 
	{
	  last_time_button=Press_button;
		return 1;	
	}
}

//�жϱ����Ƿ����仯
uint8 Check_Multiple_change(void)
{
	if(last_time_Multiple==Multiple_num)
	{
	  return 0;
	}
	else 
	{
	  last_time_Multiple=Multiple_num;
		return 1;	
	}

}

//ȷ�����ĸ���ѡ��
uint8 Axis_Gets(void)
{
	uint8 axis_mode;
	switch(control_panel_pram.Axis_press)
	{
		case CMD_X_AXIS: axis_mode=X_mode;break;
	  case CMD_Y_AXIS: axis_mode=Y_mode;break;
		case CMD_Z_AXIS: axis_mode=Z_mode;break;
		case CMD_A_AXIS: axis_mode=A_mode;break;
		case CMD_B_AXIS: axis_mode=B_mode;break;
	}
	
	return axis_mode;
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
			TX_Data[5] = Multiple_num;
			TX_Data[6] = Press_button;
			TX_Data[7] = Axis_Gets();
			TX_Data[8] = 4;
			TX_Data[9] = SetXor(TX_Data[1],1);	
		
}


// ����1��������
void Usart1_Send_Data (uint8 length)
{ 
	uint8 i,x,y,z;
  static uint16 send_times=0;
	char buf3[20];
	x = Check_Pulses_change();
	y = Check_CMD_button_change();
	z = Check_Multiple_change();
	if(x || y || z)
	{
		Create_CMD_and_Date();  //��������������ָ�������
		RS485_mode_control(1);
		for(i=0;i<length;i++)
		{
			if (i==0)
			{
				USART1->DR =(uint32_t)(1<<8)|TX_Data[0];
				while((USART1->SR&0X40)==0);
			}
			else
			{
				USART1->DR = TX_Data[i];
				while((USART1->SR&0X40)==0);  
				
			};//ѭ������,ֱ���������   		
		}
		send_times++;
		sprintf(buf3,"Slave_send:%u",send_times);	
		SetTextValue(3,50,(uchar *)buf3);
		
		RS485_mode_control(0);
	}
}
