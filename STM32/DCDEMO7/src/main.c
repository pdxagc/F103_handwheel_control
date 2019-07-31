
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "ulitity.h"
#include "string.h"
#include "flash.h"
#include "timer.h"
#include "exti.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "tft.h"
#include "malloc.h"
#include "key.h"
#include "iic.h"
#include "24c02.h"




volatile uint32  timer_tick_count = 0;               //定时器节拍

uint8 TX_Data[30]; //the sending package    存储串口3要发送的数据（主程序发送）
uint8 RX_Data[30]; //the receiving package  接收串口3发来的数据（在中断中接收）
uint8 Pulses_check=1;              //脉冲同步次数标志位


uint8 Mark_10ms=0;                 //10ms计时标记位
uint8 Mark_10ms_Count;
uint8 Mark_20ms=0;                 //20ms计时标记位
uint8 Mark_20ms_Count;
uint8 Mark_30ms=0;                 //30ms计时标记位
uint8 Mark_60ms=0;                 //60ms计时标记位
uint8 Mark_500ms=0;                //500ms计时标记位

uint8 Send_Estop_to_handwheel=1;   //给手轮发送紧急停止消息，1：发送，0：不发送
uint8 Clear_Estop_massage=0;       //清除手轮紧急停止消息；1：清除，0：不清除

uint8 Estop_button=Estop_Off;      //紧急停止按钮
uint8 Estop_Press_time=0;          //紧急按钮按下次数统计

/******************************************************************************************************/
/*!                                                                                 
*  \brief  程序入口                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
 { 
  uint16 time_conuter=0;	
  char bufrec[20],buf1[20];
  uint16 Recdata1,Recdata2,RecPulses;	 
	uint8  check_time=0;      //脉冲同步检查次数
	 
	Set_System();             //配置时钟                                                                                                                                                                                                                                                              
	systicket_init();         //配置时钟节拍
	Usart1_Init(115200);      //串口1初始化(与雕刻机通讯)
  Usart2_Init(115200);      //串口2初始化(与TFT屏通讯) 	 
	queue_reset();            //清空串口接收缓冲区 
	TIME2_Init();             //定时器2初始化
	Key_Init();               //按键初始化	
	//EXTIX_Init();             //中断初始化
	//TIME3_Init();             //定时器3初始化(向主机询问坐标)
  TIME4_Init();             //定时器4初始化(计算手轮脉冲)
  IIC_Init();               //IIC初始化
	delay_ms(300);            //延时等待串口屏初始化完毕,必须等待300ms
   
	//Usart3_Init(115200);      //串口3初始化
	
	 
	Power_On_Set();                 //串口屏开机动画和参数初始化设置	
	Get_Setting_page_pram();        //获取串口屏设置页面相关参数值（保存在flash）	
	Return_last_status();           //恢复上一次串口屏设置页面相关参数值

		//    MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		//    1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		//    2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
		
//	while(Pulses_check)        //开机同步主机脉冲
//	{
//		
//		if(RX_Data[1] == CMD_UPDATE_MACH3_NUMBER)  //接收到坐标数据
//		{	
//			Recdata1=RX_Data[18];
//			Recdata2=RX_Data[19];			 
//      RecPulses= (Recdata1<<8)+Recdata2;  //获取脉冲值
//      if(TIM4->CNT==RecPulses)
//				check_time++;
//			else 
//				TIM4->CNT=RecPulses;
//      if(check_time>5)
//        Pulses_check=0;
//			
//      sprintf(bufrec,"%u",RecPulses);	
//			SetTextValue(0,23,(uchar *)bufrec);     //显示脉冲值			
//		}
//	}

   

//   while(AT24CXX_Check()) 
//	 {
//	   SetTextValue(0,26,(uchar *)"检测失败");	 
//	 }
//	   SetTextValue(0,26,(uchar *)"检测成功");
		
	while(1)                                                                        
	{				
//		time_conuter++;	
		
		Pulses_Count_Process();               //计算手轮脉冲
		 
		TFT_Page_handle();                         //程序进入不同的工作页面,处理相关任务		
		
		
		if(Mark_10ms)
		{
			Estop_button_process();               //紧急按钮触发处理函数
			Usart1_Rec_Data_handle();             //串口1（雕刻机数据）处理接收的数据
		  Mark_10ms=0;
		}
		if(Mark_20ms) 
		{
			Key_scan();                         //手轮物理按键扫描
			TFT_command_analyse();              //分析TFT屏的命令，触发了什么按钮 			  
			Mark_20ms=0;
		}
		if(Mark_60ms)                         //定时满60ms
		{
			TFT_Show_coordanate_value();        //显示工件坐标（机械坐标待增加） 
			Work_state_control();               //雕刻机工作状态显示（未增加蓝牙，WiFi状态）
			Mark_60ms=0;
//			Usart1_Send_Data(10);
//			if(Start_Download)
//			{
//				time_conuter++;
//				if(time_conuter==1)
//				{
//					time_conuter=0;
//					Download_Per++;
//				}
//		  }
		}	
//			sprintf(buf1,"%d",time_conuter);  
//			SetTextValue(0,23,(uchar *)buf1);     //显示加工行数，需要向主机询问
				
		
	}
	
}

