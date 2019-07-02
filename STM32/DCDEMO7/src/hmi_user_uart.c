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


#define USART2_REC_LEN 2048


uint8  USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART2_REC_LEN���ֽ�.
uint8  USART2_Recdata_Len=0;              //�����ַ�����            
uint16 USART2_Rx_num=0;                   //���������ݸ���          

extern uint8  USART2_RX_STA;
extern Control_Panel_Pram control_panel_pram;


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
* �� �� ��         : Usart2_Init(������ͨѶ)
* ��������         : IO�˿ڼ�����1��ʱ�ӳ�ʼ������    A2,A3    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 

void Usart2_Init(uint32 BaudRate)
	{
		
    GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_AFIO, ENABLE); 
		
		//USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
   //Usart2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;        //�жϺţ�
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //��Ӧ���ȼ�0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //�����ж�
		NVIC_Init(&NVIC_InitStructure);	                         //�ж����ȼ���ʼ��

 
		USART_InitStructure.USART_BaudRate =BaudRate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	 
    USART_Init(USART2, &USART_InitStructure);
		

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  //
    USART_Cmd(USART2, ENABLE);                    
}



void USART2_IRQHandler(void)
{
	uint8 Res;
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)   //�����ݼĴ����ǿ� �����жϣ����յ������ݱ�����0x0d 0x0a��β��
	{ 
		 Res =USART_ReceiveData(USART2);                      //(USART1->DR); //��ȡ���յ�������	 
		 if((Res>32 && Res<=126)||Res=='\r'||Res=='\n')      //Ascii�ַ�����33����ʾ�ַ�	�����ǻس�
			{	
         USART2_RX_BUF[USART2_Recdata_Len]= Res ;
				 USART2_Recdata_Len++;
				 if(USART2_Recdata_Len>(USART2_REC_LEN-1))
				 {
					 USART2_Recdata_Len=0;                              //�������ݴ������¿�ʼ����
				 }				
				 if(Res==0x0a)                                     //0x0d:'\r'  0x0a:'\n'
				 {
           USART2_RX_STA=1;                                 //���յ�������
					// USART2_Rx_num++;                                 //�ж�����ָ��ȴ�����								 
				 }
		}

	}

}

 
/***************************************************************************
*   \name   SendChar()
*   \brief  ����1���ֽڣ�������USART1ר�ã� 
*   \param  t ���͵��ֽ�
*****************************************************************************/
void  SendChar(uchar t)     
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //�ȴ�����ת�Ƶ���λ�Ĵ���
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//�ȴ����ڷ������
}


/***************************************************************************
*   \name   Usart_SendByte( )
*   \brief  ����һ���ַ� 
*   \param  ch ���͵��ֽ�
*****************************************************************************/
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
 {
 
    USART_SendData(pUSARTx,ch);                                    /* ����һ���ֽ����ݵ� USART */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* �ȴ��������ݼĴ���Ϊ�� */
 }
 
/***************************************************************************
*   \name   Usart_SendString( )
*   \brief  ����һ���ַ� 
*   \param  str ���͵��ַ���
*****************************************************************************/
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
 
/***************************************************************************
*   \name   uart2_command_handle( )
*   \brief  ��������2���յ�������
*   \param  ��
*****************************************************************************/
 
 void uart2_command_handle(void)
 {
	 cJSON *cjson,*item_r,*item_pos,*item;//*arrayItem;          //����cJSON�ṹ�����
	 char *Rec_data=(char *)USART2_RX_BUF;   //����һ��ָ��ָ��USART2��������buf
	 cjson=cJSON_Parse(Rec_data);            //�ѽ��յ������ݴ��JSON�ַ���,�ɹ�����һ��ָ���ڴ���е�cJSON��ָ�룬ʧ�ܷ���NULL
	 if(cjson!= NULL)                         //�ж�cJSON_Parse��������ֵ,ȷ���Ƿ����ɹ�
    {
			item_r=cJSON_GetObjectItem(cjson,"r");        //��ȡһ������"r"
		  if(item_r!= NULL)                             //�ж��Ƿ��ж���r��
			{
				//************************************************************************������������*************************************
				item_pos=cJSON_GetObjectItem(item_r,"pos");
			  if(item_pos!= NULL)                          //�ж��Ƿ��ж���pos��
				{
				   item=cJSON_GetObjectItem(item_pos,"x");      //��ȡһ������"x"
					 if(item!=NULL)
					 {
						 control_panel_pram.X_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"y");      //��ȡһ������"y"
					 if(item!=NULL)
					 {
						 control_panel_pram.Y_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"z");      //��ȡһ������"z"
					 if(item!=NULL)
					 {
						 control_panel_pram.Z_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"a");      //��ȡһ������"a"
					 if(item!=NULL)
					 {
						 control_panel_pram.A_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"b");      //��ȡһ������"b"
					 if(item!=NULL)
					 {
						 control_panel_pram.B_value=item->valuedouble;
					 }
				}
				//*******************************************************************************������������***********************
				item_pos=cJSON_GetObjectItem(item_r,"mpo");
			  if(item_pos!= NULL)                          //�ж��Ƿ��ж���pos��
				{
					item=cJSON_GetObjectItem(item_pos,"x");      //��ȡһ������"x"
					 if(item!=NULL)
					 {
						 control_panel_pram.X_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"y");      //��ȡһ������"y"
					 if(item!=NULL)
					 {
						 control_panel_pram.Y_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"z");      //��ȡһ������"z"
					 if(item!=NULL)
					 {
						 control_panel_pram.Z_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"a");      //��ȡһ������"a"
					 if(item!=NULL)
					 {
						 control_panel_pram.A_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"b");      //��ȡһ������"b"
					 if(item!=NULL)
					 {
						 control_panel_pram.B_Mac_value=item->valuedouble;
					 }				
				}
			}		
		}
		cJSON_Delete(cjson);
		memset(USART2_RX_BUF,0,USART2_Recdata_Len);
		USART2_RX_STA=0;
		USART2_Recdata_Len=0;
	 
 
 
 
 
 }

