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
int16  last_time_Pulses_number;    //记录上一次脉冲
uint8  last_Override;  //记录倍率变化
uint8  CMD_last_button;       //记录按键变化


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
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
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
* 函 数 名         : Usart3_Init(与主机通讯)
* 函数功能         : IO端口及串口3，时钟初始化函数    PB10,PB11    
* 输    入         : 无  
* 输    出         : 无  
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
		
		//初始化485发送或接收使能引脚 PA1
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
 
   //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        //中断号；
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //响应优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //开启中断
		NVIC_Init(&NVIC_InitStructure);	                         //中断优先级初始化

 
		USART_InitStructure.USART_BaudRate =BaudRate;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;            //字长为9位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;                 //一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;                    //无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	       //收发模式
    USART_Init(USART3, &USART_InitStructure);       
		

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //接收中断使能
    USART_Cmd(USART3, ENABLE);    
    
    RS485_TX_Set(0);		//使能485接收模式
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


//RS485 模式控制.en:0,接收;1,发送.
void RS485_TX_Set(uint8 en)
{
	if(en)
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}
 
/***************************************************************************
*   \name   SendChar()（本函数是USART1专用）
*   \brief  发送1个字节 
*   \param  t 发送的字节
*****************************************************************************/
void  SendChar(uchar t)     
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //等待数据转移到移位寄存器
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//等待串口发送完毕
}


/***************************************************************************
*   \brief  发送一个字符 
*   \param  ch 发送的字节
*****************************************************************************/
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
 {
 
    USART_SendData(pUSARTx,ch);                                    /* 发送一个字节数据到 USART */
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* 等待发送数据寄存器为空 */
 }
 
 //Usart3_send_Str 发送数组内容
 void Usart3_send_Str(uint8 buf[])
 {
	 uint8 i=0;
	 while(i<10)
	 {
		 Usart_SendByte( USART3, buf[i] );
			i++;
	    while (USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); /* 等待发送完成 */	 
	 }
 
 }
// 发送的字符串
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
 

 // 发送数据前，生成一个校验参数	
char SetXor	(char length, char start)
{
	char i, buf;;

	buf=TX_Data[start];
	for (i=start+1; i<=(length); i++)
		buf=buf^TX_Data[i];
	return buf;
}

//接收数据后，生成一个检验参数，前后参数对比，一致的话则接收的数据正确
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

//地址校验
uint8_t Check_Address (char data)
{
	unsigned char buf;
	buf = data&(0x1f);//0b0001 1111;
	if (buf==My_Address)	
		return 1;
	else
		return	0;
}

   
// 串口3接收中断执行函数  //place at ISR，接收主机发送的数据
void Usart3_Recieve_ISR_Process (void)
{ 	
	 unsigned char RX_Buffer;
	 short dat; //16 bit
	 
	 dat=USART3->DR;
	 USART3->SR&=~(1<<5);  //SR寄存器的第五位清0
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
	  	RX_Data[rxcounter]=RX_Buffer;   //RX_Data[0]存储数据长度
	  	rxcounter++;
	  	if(RX_Data[0]>PACKAGELEN)  //数据闯到超过30个字节，接收异常
	  	{ 
	  	 	RX_Busy=0;//等待下一个起始指令
	  	}
	  	if(rxcounter>RX_Data[0])    //接收完成了
	  	{
	  		//check package xor
	  		RX_Busy=0;
	  		if(CheckXor(RX_Data[rxcounter-1],RX_Data[0]))
	  		{
	  			ready2read=1; //校验通过									
//					if(RX_Data[1] == CMD_ASK_SLAVE)
//					{
//						 Usart3_Send_Data(10);
//					}

	  		}
	 	   }
	  }

}


//串口2接收数据处理函数
void Usart3_Data_handle (void)
{

	uint8 command;
	uint16 Recdata1,Recdata2;
	if(ready2read)        //检验通过
	{
		command=RX_Data[1];
		switch(command)
		{
			case CMD_ASK_SLAVE:           //发送数据
				Usart3_Send_Data(10);
			break;			
			case CMD_UPDATE_MACH3_NUMBER: //接收到坐标  
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

// 串口2发送数据
void Usart3_Send_Data (uint8 length)
{ 
	uint8 i,x,y,z;
	x = Check_Pulses_change();
	y = Check_CMD_button_change();
	z = Check_Override_change();
	if(x || y || z)
	{
		Create_CMD_and_Date();  //创建发给主机的指令和数据
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
			};//循环发送,直到发送完毕   
			}
		RS485_TX_Set(0);
	}
}


//判断脉冲是否发生变化
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

//判断按键是否发生变化
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

//判断倍率是否发生变化
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

//确定是哪个轴选中
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

//创建发给主机的指令和数据
//  address  length command   mpgmsb  mpglsb  multiplierlsb   key   axis  div     xorcheck
// bit 0     1       2         3       4       5               6    7      8       9

//倍率：x1 x2 x5 x10 x20 有效
//0-16 按键 50-66（长按） 0xff无效
//控制模式 10:x轴 11:y轴 12:z 13:a轴
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

