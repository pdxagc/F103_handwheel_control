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



//////COMMAND TABLE//////////
#define CMD_X_AXIS	0                  //X轴选中
#define CMD_Y_AXIS	1                  //Y轴选中
#define CMD_Z_AXIS 2                   //Z轴选中
#define CMD_A_AXIS 3                   //A轴选中
#define CMD_B_AXIS 100                 //B轴选中
#define CMD_Clear 4                     //清零
#define CMD_Return_WorkPiece_Zero 5     //回工件零
#define CMD_Divided	6                   //分中
#define CMD_Override_Change	7           //倍率切换
#define CMD_Spin_On_Off	8               //主轴开关
#define CMD_Auto_knife	9               //自动对刀
#define CMD_Return_Machine_Zero 10      //回机械零
#define CMD_ 11
#define CMD_Start 12                    //开始加工
#define CMD_1	13
#define CMD_2	14
#define CMD_EStop	15                    //紧急停止
#define CMD_All_Spin_Clear	16          //全轴清零
#define CMD_Coordinate_Change 17        //坐标切换
#define CMD_Soft_Limit	18              //软限位
#define CMD_Jump_Work	19                //跳行加工
#define CMD_Safe_Z	20                   //安全Z
#define CMD_Reset	21                     //复位

#define CMD_ASK_SLAVE 26                 //主机请求数据
#define CMD_UPDATE_MACH3_NUMBER	41       //主机发送坐标

// 串口1初始化
void Usart1_Init(uint32 Baudrate);


//串口2，时钟初始化函数    A2,A3    
void Usart2_Init(uint32 BaudRate);

//RS485 模式控制.en:0,接收;1,发送.
void RS485_TX_Set(uint8 en);


/***************************************************************************
*   \name   SendChar()（本程序USART1专用）
*   \brief  发送1个字节 
*   \param  t 发送的字节
*****************************************************************************/
void  SendChar(uchar t);


// 发送的字节
 void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);

// 发送的字符串
 void Usart_SendString( USART_TypeDef * pUSARTx, char *str);

// 发送数据前，生成一个校验参数	
char SetXor	(char length, char start);

//接收数据后，生成一个检验参数，前后参数对比，一致的话则接收的数据正确
unsigned char CheckXor (char data, unsigned char len);

//地址校验
uint8_t Check_Address (char data);

//串口2中断执行函数  //place at ISR
void Usart2_Recieve_ISR_Process (void);

//串口2接收数据处理函数
void Usart2_Data_handle (void);

// 串口2发送数据函数
void Usart2_Send_Data (uint8 length);

//创建指令和数据
void Create_CMD_and_Date(void);

#endif
