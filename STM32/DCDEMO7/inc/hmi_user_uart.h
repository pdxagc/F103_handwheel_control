/*! 
*  \file hmi_driver.h
*  \brief 串口初始化
*  
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

//控制面板
#define CMD_X_AXIS	0                  //X轴选中
#define CMD_Y_AXIS	1                  //Y轴选中
#define CMD_Z_AXIS  2                   //Z轴选中
#define CMD_A_AXIS  3                   //A轴选中
#define CMD_B_AXIS  100                 //B轴选中
#define CMD_Clear   4                     //清零
#define CMD_Return_WorkPiece_Zero 5     //回工件零
#define CMD_Divided	 6                   //分中
#define CMD_Override_Change	7           //倍率切换
#define CMD_Spin_On_Off	8               //主轴开关
#define CMD_Auto_knife	9               //自动对刀
#define CMD_Return_Machine_Zero 10      //回机械零
#define CMD_ 11
#define CMD_Start 12                    //开始加工
#define CMD_1	13
#define CMD_2	14
#define CMD_EStop	15                    //紧急停止
//#define CMD_Stop  16
#define CMD_All_Spin_Clear	17          //全轴清零
#define CMD_Coordinate_Change 18        //坐标切换
#define CMD_Soft_Limit	19              //软限位
#define CMD_Jump_Work	20                //跳行加工
#define CMD_Safe_Z	21                   //安全Z
#define CMD_Reset	22                     //复位
#define CMD_Stop  23                     //停止加工

//加工页面
#define CMD_Spindle_Speed_Add  24        //主轴速度加
#define CMD_Spindle_Speed_Subtract 25    //主轴速度减
#define CMD_Work_Speed_add  26            //加工速度加
#define CMD_Work_Speed_Suttract  27      //加工速度减

//设置页面
#define CMD_Safe_Z_Mode  28             //安全Z模式
#define CMD_Auto_Knife_Mode  30          //自动对刀模式
#define CMD_Uint_Mode  31               //单位模式
#define CMD_Voice_Switch  32            //语音开关
#define CMD_Sure   33                   //确定
#define CMD_Cancel 34                   //取消

//文件管理
#define CMD_Download 37                 //下载
#define CMD_Cancel_Download  38         //取消下载
#define CMD_Delete    39                //删除
#define CMD_Storage_View 40             //内存预览
#define CMD_Open_Loading 42             //打开加载
#define CMD_File_Delete_Sure 43         //文件删除
//#define CMD_Clode_Flie_next_page 44     //云空间文件下一页
//#define CMD_SD_File_last_page   45      //SD卡文件下一页


//网络连接与账户登录
//#define CMD_WiFi_page   46        //WiFi页码
#define CMD_Connect_WIFI   47     //连接WiFi
#define CMD_Sign_In       48     //登录账户


#define CMD_ASK_SLAVE 29                 //主机请求数据
#define CMD_RPY_HC_MPG1	36               //主机获取数据
#define CMD_UPDATE_MACH3_NUMBER	41       //主机发送坐标

// 串口1初始化
void Usart1_Init(uint32 Baudrate);


//串口2，时钟初始化函数    A2,A3    
void Usart2_Init(uint32 BaudRate);

//串口3，时钟初始化函数    A2,A3 
void Usart3_Init(uint32 BaudRate);

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
 
//Usart3_send_Str 发送数组内容
// void Usart3_send_Str(uint8 buf[]);

// 发送数据前，生成一个校验参数	
char SetXor	(char length, char start);

//接收数据后，生成一个检验参数，前后参数对比，一致的话则接收的数据正确
unsigned char CheckXor (char data, unsigned char len);

//地址校验
uint8_t Check_Address (char data);

//串口1中断，接收雕刻机数据  //place at ISR
void Usart1_Recieve_Process (void);

//串口2接收数据处理函数
void Usart1_Rec_Data_handle (void);

//给主机发送数据
void Send_data_to_Master(void);

// 串口2发送数据函数
void Usart1_Send_Data (uint8 length);

//判断脉冲是否发生变化
uint8 Check_Pulses_change(void);

//判断按键是否发生变化
uint8 Check_CMD_button_change(void);

//判断倍率是否发生变化
uint8 Check_Override_change(void);

//创建指令和数据
void Create_CMD_and_Date(void);

#endif
