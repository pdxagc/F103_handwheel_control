/*
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
extern uint8  Press_button;          //记录哪个按钮触发(需要把按键发送给雕刻机)
extern uint16 master_ask,master_send;


uint8  RX_Busy=0;
uint8  HC_Address, rxcounter, remaincounter; //this unit is this unit address, if change to master become 0000 
uint8  ready2send; // bit is 1 while there is command to send
uint8  ready2read;
uint16 last_time_Pulses_number=0;  //记录上一次脉冲
uint8  last_time_Multiple=0;       //记录倍率变化
uint8  last_time_button=0;         //记录按键变化


/*******************************************************************************  
* 函 数 名         : Usart1_Init(与雕刻机通讯)  
* 函数功能         : IO端口及串口1，时钟初始化函数    A9,A10    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/  
void Usart1_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //声明一个结构体变量，用来初始化GPIO   
    USART_InitTypeDef USART_InitStructure;    //声明一个结构体变量，用来初始化串口
	  NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;                     //USART1_TX PA.9   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //复用推挽输出	
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //初始化 GPIOA.9

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                    //USART1_RX PA.10
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                        //初始化 GPIOA.10
	  //串口参数初始化 /* USART configuration */
	
	 //初始化485使能引脚 PA7
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
    USART_DeInit(USART1);                                          //复位USART1
    USART_InitStructure.USART_BaudRate = BaudRate;                 //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_9b;    //字长为9 位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;            //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //收发模式 
    USART_Init(USART1, &USART_InitStructure);                      //初始化串口    

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;        //中断号；
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		   //响应优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //开启中断
		NVIC_Init(&NVIC_InitStructure);	
		
		
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                  //接收中断使能

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);                                     //使能串口
		RS485_mode_control(0);		                                     //使能485接收模式
}


/*******************************************************************************  
* 函 数 名         : Usart2_Init(与TFT屏通讯)
* 函数功能         : IO端口及串口，时钟初始化函数    PA2,PA3    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/ 
void Usart2_Init(uint32 BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;      //声明一个结构体变量，用来初始化GPIO   
    USART_InitTypeDef USART_InitStructure;    //声明一个结构体变量，用来初始化串口
	  NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;                     //USART1_TX PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;              //复用推挽输出	
    GPIO_Init(GPIOA, &GPIO_InitStructure);                       //初始化 GPIOA.2

    //  /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                    //USART1_RX PA.3
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;         //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);                        //初始化 GPIOA.3
	
	  //串口参数初始化 /* USART configuration */
    USART_DeInit(USART2);                                          //复位USART2
    USART_InitStructure.USART_BaudRate = BaudRate;                 //波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;    //字长为 8 位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;         //一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;            //无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
    USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx; //收发模式 
    USART_Init(USART2, &USART_InitStructure);                      //初始化串口    

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;        //中断号；
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		   //响应优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //开启中断
		NVIC_Init(&NVIC_InitStructure);	
		
		
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);                  //接收中断使能

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);                                     //使能串口
}

/*******************************************************************************  
* 函 数 名         : Usart3_Init()
* 函数功能         : IO端口及串口3，时钟初始化函数    PB10,PB11    
* 输    入         : 无  
* 输    出         : 无  
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
		
		
 
   //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        //中断号；
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //响应优先级0
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			     //开启中断
		NVIC_Init(&NVIC_InitStructure);	                         //中断优先级初始化

 
		USART_InitStructure.USART_BaudRate =BaudRate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;            //字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;                 //一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;                    //无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	       //收发模式
    USART_Init(USART3, &USART_InitStructure);       
		

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);     //接收中断使能
    USART_Cmd(USART3, ENABLE);    
    
    
}

//串口1中断函数
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
	     Usart1_Recieve_Process();  //接收数据
  }
}

//串口2中断函数
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART2);
        queue_push(data);
    }

}




//RS485 模式控制.en:0,接收;1,发送.
void RS485_mode_control(uint8 en)
{
	if(en)
		GPIO_SetBits(GPIOA, GPIO_Pin_7);
	else
		GPIO_ResetBits(GPIOA, GPIO_Pin_7);
}
 
/*
*   \name   SendChar()（本函数是USART2专用）
*   \brief  发送1个字节 
*   \param  t 发送的字节
*/
void  SendChar(uchar t)     
{
    USART_SendData(USART2,t);
    while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); //等待数据转移到移位寄存器
    while((USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET));//等待串口发送完毕
}



 
//Usart3_send_Str 发送数组内容
//void Usart3_send_Str(uint8 buf[])
// {
//	 uint8 i=0;
//	 while(i<10)
//	 {
//		 Usart_SendByte( USART3, buf[i] );
//			i++;
//	    while (USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); /* 等待发送完成 */	 
//	 }
// 
// }
///***************************************************************************
//*   \brief  发送一个字节
//*   \param  ch 发送的字节
//*****************************************************************************/
// void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
// {
// 
//    USART_SendData(pUSARTx,ch);                                    /* 发送一个字节数据到 USART */
//    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET); /* 等待发送数据寄存器为空 */
// }
////发送的字符串
// void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
// {
//		//unsigned int k=0;
//		while (*str !='\0')
//		{
//			Usart_SendByte( pUSARTx, *str );
//			str++;
//		} 
//	  while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET); /* 等待发送完成 */
// }
// 

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
	buf = data&(0x1f);    //0b0001 1111;
	if (buf==My_Address)	
		return 1;
	else
		return	0;
}

   
// 串口1接收主机发送的数据 //place at ISR
void Usart1_Recieve_Process (void)
{ 	
	 //uint16 master_ask=0,master_send=0;
	 uint8 rec_command; 
	 char buf4[20];
	 uchar RX_Buffer;
	 short dat; //16 bit	 
	 dat=USART1->DR;
	 USART1->SR&=~(1<<5);  //SR寄存器的第五位清0
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
#define CMD_ASK_SLAVE 29                 //主机请求数据
#define CMD_RPY_HC_MPG1	36               //手轮发送数据
#define CMD_UPDATE_MACH3_NUMBER	41       //主机发送坐标数据
#define CMD_Working_File_Name 50         //加工文件名
#define CMD_Work_line 51                 //加工行数
#define CMD_Working_Code 52              //加工代码
#define CMD_SPin_Speed 53                //主轴速度
#define CMD_Working_Speed 54             //加工速度
#define CMD_Warn_Massage  55             //警报信息
#define CMD_Clond_File_Name 56           //云空间文件名
#define CMD_SD_Card_File_Name 57         //SD卡文件名
#define CMD_Storage_Data  58             //内存数据
#define CMD_Wifi_Name  59                //wifi名称
#define CMD_Wifi_Password 60             //wif密码
#define CMD_Account_Name  61             //用户账户
#define CMD_Account_Password 62          //用户密码
*/

//串口1接收数据后，对数据进行处理
void Communication_Data_handle (void)
{

	uint8 command;
	uint16 Recdata1,Recdata2;
	if(ready2read)                    //检验通过，收到了主机的数据
	{
		command=RX_Data[1];            //读取接收的命令
		switch(command)
		{
			case CMD_ASK_SLAVE:           //主机请求数据，手轮要给主机发送数据
				Usart1_Send_Data(10);
			break;			
			case CMD_UPDATE_MACH3_NUMBER: //接收到工件坐标数据  
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
			case CMD_Working_File_Name:    //加工文件名
			{
			
			}break;
			case CMD_Work_line:             //加工行数
			{
			
			}break;
			case CMD_Working_Code:             //加工代码
			{
			
			}break;
			case CMD_SPin_Speed:               //主轴速度
			{
			
			}break;
			case CMD_Working_Speed:            //加工速度
			{
			
			}break;
			case CMD_Warn_Massage:             //警报信息
			{  
			
			}break;	
			case CMD_Clond_File_Name:          //云空间文件名
			{
			
			}break;
			case CMD_SD_Card_File_Name:         //SD卡文件名
			{
			
			}break;
		  case 	CMD_Storage_Data:            //内存数据
			{  
			
			}break;
			case CMD_Wifi_Name:               //wifi名称
			{
			
			}break;
			
		}
		ready2read=0;
	}


}

////给主机发送数据
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





//判断脉冲是否发生变化
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

//判断按键是否发生变化
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

//判断倍率是否发生变化
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

//确定是哪个轴选中
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
			TX_Data[5] = Multiple_num;
			TX_Data[6] = Press_button;
			TX_Data[7] = Axis_Gets();
			TX_Data[8] = 4;
			TX_Data[9] = SetXor(TX_Data[1],1);	
		
}


// 串口1发送数据
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
		Create_CMD_and_Date();  //创建发给主机的指令和数据
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
				
			};//循环发送,直到发送完毕   		
		}
		send_times++;
		sprintf(buf3,"Slave_send:%u",send_times);	
		SetTextValue(3,50,(uchar *)buf3);
		
		RS485_mode_control(0);
	}
}
