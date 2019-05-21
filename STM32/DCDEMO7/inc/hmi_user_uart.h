/*! 
*  \file hmi_driver.h
*  \brief 串口初始化
*  \version 1.0
*  \date 2012-2018
*  \copyright 广州大彩光电科技有限公司
*/
#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f10x_it.h"     //根据用户MCU进行修改

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long

/****************************************************************************
* 名    称： Usart1_Init()
* 功    能： 串口初始化
* 入口参数： 无
* 出口参数： 无
****************************************************************************/
void Usart1_Init(uint32 Baudrate);



/*******************************************************************************  
* 函 数 名         : Usart2_Init  
* 函数功能         : IO端口及串口1，时钟初始化函数    A2,A3    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/ 

void Usart2_Init(uint32 BaudRate);



/***************************************************************************
*   \name   SendChar()
*   \brief  发送1个字节（本程序USART1专用） 
*   \param  t 发送的字节
*****************************************************************************/
void  SendChar(uchar t);


/***************************************************************************
*   \name   Usart_SendByte( )
*   \brief  发送一个字符 
*   \param  ch 发送的字节
*****************************************************************************/
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);
 
/***************************************************************************
*   \name   Usart_SendString( )
*   \brief  发送一个字符 
*   \param  str 发送的字符串
*****************************************************************************/
 void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
 
/***************************************************************************
*   \name   uart2_command_handle( )
*   \brief  解析串口2接收到的数据
*   \param  无
*****************************************************************************/
 void uart2_command_handle(void);


#endif
