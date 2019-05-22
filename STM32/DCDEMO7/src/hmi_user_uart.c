/************************************版权申明********************************************
--------------------------------------------------------------------------------------
使用必读
hmi_user_uart.c中的串口发送接收函数共3个函数：

串口初始化Uartinti()、
发送1个字节SendChar()、
发送字符串SendStrings().

若移植到其他平台，需要修改底层寄存器设置,但禁止修改函数名称，否则无法与HMI驱动库(hmi_driver.c)匹配。
---------------------------------------------------------------------------------------*/


#include "hmi_user_uart.h"
#include "timer.h"
#include "cJSON.h"
#include "tft.h"
#include "string.h"


#define USART2_REC_LEN 2048


uint8  USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART2_REC_LEN个字节.
uint8  USART2_Recdata_Len=0;              //接收字符个数            
uint16 USART2_Rx_num=0;                   //带解析数据个数          

extern uint8  USART2_RX_STA;
extern Control_Panel_Pram control_panel_pram;


/*******************************************************************************  
* 函 数 名         : Usart1_Init(与TFT屏通讯)  
* 函数功能         : IO端口及串口1，时钟初始化函数    A9,A10    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/  
void Usart1_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //声明一个结构体变量，用来初始化GPIO   
    USART_InitTypeDef USART_InitStructure;    //声明一个结构体变量，用来初始化串口
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;                     //USART1_TX PA.9
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;            
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //初始化 GPIOA.9

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                    //USART1_RX PA.10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	  //串口参数初始化 /* USART configuration */
    USART_DeInit(USART1);                                          //复位USART1
    USART_InitStructure.USART_BaudRate = BaudRate;                 //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //字长为 8 位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;            //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //收发模式 
    USART_Init(USART1, &USART_InitStructure);                      //初始化串口         
		
		
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                  //接收中断使能

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);                                     //使能串口
}


/*******************************************************************************  
* 函 数 名         : Usart2_Init(与主机通讯)
* 函数功能         : IO端口及串口1，时钟初始化函数    A2,A3    
* 输    入         : 无  
* 输    出         : 无  
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
 
   //Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;        //中断号；
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //响应优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //开启中断
		NVIC_Init(&NVIC_InitStructure);	                         //中断优先级初始化

 
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
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)   //读数据寄存器非空 接收中断（接收到的数据必须是0x0d 0x0a结尾）
	{ 
		 Res =USART_ReceiveData(USART2);                      //(USART1->DR); //读取接收到的数据	 
		 if((Res>32 && Res<=126)||Res=='\r'||Res=='\n')      //Ascii字符大于33是显示字符	或者是回车
			{	
         USART2_RX_BUF[USART2_Recdata_Len]= Res ;
				 USART2_Recdata_Len++;
				 if(USART2_Recdata_Len>(USART2_REC_LEN-1))
				 {
					 USART2_Recdata_Len=0;                              //接收数据错误，重新开始接收
				 }				
				 if(Res==0x0a)                                     //0x0d:'\r'  0x0a:'\n'
				 {
           USART2_RX_STA=1;                                 //接收到了数据
					// USART2_Rx_num++;                                 //有多少条指令等待解析								 
				 }
		}

	}

}

 
/***************************************************************************
*   \name   SendChar()
*   \brief  发送1个字节（本程序USART1专用） 
*   \param  t 发送的字节
*****************************************************************************/
void  SendChar(uchar t)     
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //等待数据转移到移位寄存器
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//等待串口发送完毕
}


/***************************************************************************
*   \name   Usart_SendByte( )
*   \brief  发送一个字符 
*   \param  ch 发送的字节
*****************************************************************************/
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
 {
 
    USART_SendData(pUSARTx,ch);                                    /* 发送一个字节数据到 USART */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* 等待发送数据寄存器为空 */
 }
 
/***************************************************************************
*   \name   Usart_SendString( )
*   \brief  发送一个字符 
*   \param  str 发送的字符串
*****************************************************************************/
 void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
 {
		//unsigned int k=0;
		while (*str !='\0')
		{
			Usart_SendByte( pUSARTx, *str );
			str++;
		} 
	  while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); /* 等待发送完成 */
 }
 
/***************************************************************************
*   \name   uart2_command_handle( )
*   \brief  解析串口2接收到的数据
*   \param  无
*****************************************************************************/
 
 void uart2_command_handle(void)
 {
	 cJSON *cjson,*item_r,*item_pos,*item;//*arrayItem;          //定义cJSON结构体变量
	 char *Rec_data=(char *)USART2_RX_BUF;   //定义一个指针指向USART2接收数据buf
	 cjson=cJSON_Parse(Rec_data);            //把接收到的数据打包JSON字符串,成功返回一个指向内存块中的cJSON的指针，失败返回NULL
	 if(cjson!= NULL)                         //判断cJSON_Parse函数返回值,确定是否打包成功
    {
			item_r=cJSON_GetObjectItem(cjson,"r");        //获取一个对象"r"
		  if(item_r!= NULL)                             //判断是否有对象“r”
			{
				//************************************************************************解析工件坐标*************************************
				item_pos=cJSON_GetObjectItem(item_r,"pos");
			  if(item_pos!= NULL)                          //判断是否有对象“pos”
				{
				   item=cJSON_GetObjectItem(item_pos,"x");      //获取一个对象"x"
					 if(item!=NULL)
					 {
						 control_panel_pram.X_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"y");      //获取一个对象"y"
					 if(item!=NULL)
					 {
						 control_panel_pram.Y_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"z");      //获取一个对象"z"
					 if(item!=NULL)
					 {
						 control_panel_pram.Z_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"a");      //获取一个对象"a"
					 if(item!=NULL)
					 {
						 control_panel_pram.A_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"b");      //获取一个对象"b"
					 if(item!=NULL)
					 {
						 control_panel_pram.B_value=item->valuedouble;
					 }
				}
				//*******************************************************************************解析解析坐标***********************
				item_pos=cJSON_GetObjectItem(item_r,"mpo");
			  if(item_pos!= NULL)                          //判断是否有对象“pos”
				{
					item=cJSON_GetObjectItem(item_pos,"x");      //获取一个对象"x"
					 if(item!=NULL)
					 {
						 control_panel_pram.X_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"y");      //获取一个对象"y"
					 if(item!=NULL)
					 {
						 control_panel_pram.Y_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"z");      //获取一个对象"z"
					 if(item!=NULL)
					 {
						 control_panel_pram.Z_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"a");      //获取一个对象"a"
					 if(item!=NULL)
					 {
						 control_panel_pram.A_Mac_value=item->valuedouble;
					 }
					 item=cJSON_GetObjectItem(item_pos,"b");      //获取一个对象"b"
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

