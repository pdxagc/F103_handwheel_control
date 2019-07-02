
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



volatile uint32  timer_tick_count = 0;               //定时器节拍
uint8  cmd_buffer[CMD_MAX_SIZE];                     //指令缓存
uint8  USART3_RX_STA=0;                              //USART2接收状态标记

uint8  TX_Data [30]; //the sending package    存储串口2要发送的数据（主程序发送）
uint8  RX_Data [30]; //the receiving package  接收串口2发来的数据（在中断中接收）

uint8  Work_Page_Status = 0;             //工作页面标记
uint8  get_cmd_type;                     //获取指令类型
uint8  get_ctrl_msg;                     //获取消息的类型
uint8  get_button_state;                 //获取按钮状态
uint8  get_control_type;                 //获取控件类型
uint16 get_screen_id;                    //获取画面ID
uint16 get_control_id;                   //获取控件ID
uint32 get_value;                        //获取数值
uint8  input_buf[20];                    //键盘输入内容
uint8 last_time_work_state;              //记录上一次机器工作状态
int16 Pulses_counter;              // 手轮脉冲数量
uint8 Override_num;                //倍率
uint8 Send_cooddinate_status;      //发送坐标标记位,1:发送坐标，0：不发送
uint8 first_time_re_workpiece;     //首次进入回工件零页面
uint8 first_time_into_ISR=0;        //第一次进入中断标志位 
int32 Working_line;                //加工行数

uint8 file_name[20]="精雕佛像";    //文件名
char  Working_line_buf[10];        //保存加工行数

State state;                               //申明工作状态结构体变量
Speed_Control Speed;                       //声明主轴速度相关的结构体变量
Pram_Status pram_status;                   //声明设置页面相关参数的结构体变量
Control_Panel_Pram control_panel_pram;      //声明控制面板相关参数的结构体变量
Return_Workpiece_Zero return_workpiece_zero;//声明回工件零相关参数的结构体变量
Devide_Set devide_set;                     //声明分中设置相关参数的结构体变量
Jump_Work_Set jump_work_set;               //声明跳行加工相关参数的结构体变量

/******************************************************************************************************/
/*!                                                                                 
*  \brief  程序入口                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
 {  
	qsize  size = 0;          //指令长度 
	Set_System();             //配置时钟                                                                                                                                                                                                                                                              
	systicket_init();         //配置时钟节拍
  Interrupts_Config();      //配置串口中断	 
	Usart1_Init(115200);      //串口1初始化(与串口屏通讯)
  Usart3_Init(115200);      //串口3初始化(与雕刻机通讯)
	queue_reset();            //清空串口接收缓冲区 
	//TIME2_Init();             //定时器2初始化(向主机发送坐标)
	//TIME3_Init();             //定时器3初始化(向主机询问坐标)
  TIME4_Init();             //定时器4初始化(计算手轮脉冲)
	//TIM_Cmd(TIM4, DISABLE);   //关闭TIM4定时器                                                                                               
	delay_ms(300);            //延时等待串口屏初始化完毕,必须等待300ms  
	
	Power_On_Set();                 //串口屏开机动画和参数初始化设置	
	Setting_page_pram_get();        //获取串口屏设置页面相关参数值（保存在flash）	
	Return_last_status();           //恢复上一次串口屏设置页面相关参数值

		//    特别注意
		//    MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		//    1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		//    2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
	
	while(1)                                                                        
	{
  	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);              //接收到串口屏的数据，从USART1的指令缓冲区cmd_buffer中获取一条指令，得到指令长度       
		if(size>0 && cmd_buffer[1]!=0x07)                            //接收到指令 ，及判断是否为开机提示
		{                                                                           
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);               //指令分析处理 ，标记应该进入哪个Work_Page_Status，标记相应的操作位
			memset(cmd_buffer, 0, CMD_MAX_SIZE);                       //对指令缓冲cmd_buffer清零
		}
		
		Work_Page_Process();                                         //进入标记的工作页面，处理相关任务
		USART3->CR1 &=(~(1<<2));
		if(USART3->SR &1<<3)
		{
			uint8_t i;
			i=USART3->SR;
			i=USART3->DR;
		}
		Usart3_Data_handle();                                       //串口3接收数据后，对数据进行处理
		USART3->CR1|=1<<2;

	}  
}




/************************************************************
*
*程序进入标记的工作页面，处理相关任务
*
*************************************************************/
void Work_Page_Process(void)
{
	if(last_time_work_state!=state.Work_state)
	{
		if(state.Work_state==Start)         
		{
			WorkingStatus_Starting();      //串口屏显示当前处于加工状态
		}
		else                                 
		{
			WorkingStatus_Stoped();        //串口屏显示当前已经停止加工
		}
	}
	
	switch(Work_Page_Status)
	{	
		case Working_Page: //*********************************************************加工页面***************************************************************************************************
		{	
			Pulses_counter=Get_Pulses_num();       //计算脉冲个数 
			TFT_Show_coordanate_value(Work_Page_Status);		   //串口屏显示工件和机械坐标	
			Spindle_and_Work_Speed_Key_Process();	           	//加工中心主轴速度和加工速度按钮处理
			if(state.Work_state==Start)                        //机器处于加工状态中
			{
				
			}		
			SetTextValue(0,21,(uchar *)file_name);        //显示正在加载的文件名	
			sprintf(Working_line_buf,"%d",Pulses_counter);  
			SetTextValue(0,22,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问
			
		}break;
		case Setting_page:  //********************************************************设置页面*************************************************************************************************
		{
		 if(pram_status.Screen_ID1_Setting_concel)          //取消按钮按下
		 {
				Return_last_status();                           //恢复上一次设置状态 
				pram_status.Screen_ID1_Setting_concel=0;
				Work_Page_Status=Working_Page;				 
		 }		
		}
		break;
		case Save_Pram_Page:  //****************************************************提示是否保存参数设置****************************************************************************************
		{
			if(state.Work_state==Stop)                                   //停止加工
			{
				if(pram_status.Screen_ID21_Setting_Sure)                   //“确定按钮”按下
				{
					Save_Set();                                              //保存设置
					Work_Page_Status=Working_Page;
				}
				if(pram_status.Screen_ID21_Setting_concel)                 //取消按钮按下
				{
					Return_last_status();                                    //恢复上一次设置状态
					Work_Page_Status=Working_Page;					
				}	
			}				
		}
		break;	
		case ControlPanel_Page:  //******************************************************控制面板页面*****************************************************************************************
		{			
//			if(state.Work_state==Stop)	                        //停止加工
//			{
				Pulses_counter=Get_Pulses_num();       //计算脉冲个数 
        			
//				switch (control_panel_pram.Axis_press)   			
//				{
//					case CMD_X_AXIS:            //***************X轴选定状态**************	
//					{				
//							if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
//							{				
//								control_panel_pram.All_Spindle_Clear_Button=0;
//							}
//							if(devide_set.Devide_contronl)         //分中设置
//							{
//								devide_set.Devide_contronl = 0;									
//							}													
//					}break;
//					case CMD_Y_AXIS:                //****************Y轴选定状态*************
//					{
//	
//						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
//						{										
//							control_panel_pram.All_Spindle_Clear_Button=0;
//						}
//						if(devide_set.Devide_contronl)         //分中设置
//						{
//							devide_set.Devide_contronl = 0;
//							
//						}
//					
//					}break;
//					case CMD_Z_AXIS:                 //*****************Z轴选定状态********
//					{
//						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
//						{						
//							control_panel_pram.All_Spindle_Clear_Button=0;
//						}
//					
//					}break;
//					case CMD_A_AXIS:                //***************A轴选定状态*************
//					{
//						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
//						{						
//							control_panel_pram.All_Spindle_Clear_Button=0;
//						}
//						
//					}break;
//					case CMD_B_AXIS:                //****************B轴选定状态*************
//					{
//						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
//						{							
//							control_panel_pram.All_Spindle_Clear_Button=0;
//						}	
//					}break;
										
//				}					
//			}
			TFT_Show_coordanate_value(ControlPanel_Page);		  //串口屏显示工件和机械坐标
			SetTextValue(2,27,(uchar *)file_name);            //显示正在加载的文件名	         				
			sprintf(Working_line_buf,"%d",Pulses_counter);  
			SetTextValue(2,28,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问	
			
		}break;
		case Return_WorkPiece_Zero_Page:   //*******************************************回工件零页面***************************************************************************************
		{ 
			TIM_Cmd(TIM4, DISABLE);         //禁止 TIM4，脉冲不计数
			if(first_time_re_workpiece)     //首次进入这个页面
			{
				SetButtonValue(3,1,0);         //所有轴显示为选中状态()
				SetButtonValue(3,4,0);
				SetButtonValue(3,5,0);
				SetButtonValue(3,6,0);
				SetButtonValue(3,7,0);
				SetButtonValue(3,8,0);
				
				return_workpiece_zero.all_spindle_status=0;   //所有轴显示为选中状态
				return_workpiece_zero.X_clear_status=0;
				return_workpiece_zero.Y_clear_status=0;
				return_workpiece_zero.Z_clear_status=0;
				return_workpiece_zero.A_clear_status=0;
				return_workpiece_zero.B_clear_status=0;
				
				control_panel_pram.Temp_save_Xvalue=control_panel_pram.X_value;   //把相应坐标值保存起来
				control_panel_pram.Temp_save_Yvalue=control_panel_pram.Y_value;
				control_panel_pram.Temp_save_Zvalue=control_panel_pram.Z_value;
				control_panel_pram.Temp_save_Avalue=control_panel_pram.A_value;
				control_panel_pram.Temp_save_Bvalue=control_panel_pram.B_value;
				first_time_re_workpiece=0;					
			}
			
			if(return_workpiece_zero.X_clear_status==0 && return_workpiece_zero.Y_clear_status==0 && return_workpiece_zero.Z_clear_status==0 && return_workpiece_zero.A_clear_status==0 && return_workpiece_zero.B_clear_status==0)   //所有轴都选中
			{
				SetButtonValue(3,1,0);            //全轴按钮显示选中状态
			}
			else
			{
				SetButtonValue(3,1,1);            //全轴按钮显示未选中状态
			}
			
			if(return_workpiece_zero.Sure)        //确定按钮按下
			{
				//*********************************************************************************************
				if(return_workpiece_zero.X_clear_status==0)  //对X轴清零
				{
					Puless_count_clear();
					control_panel_pram.X_value=0;
					control_panel_pram.Temp_save_Xvalue=0;													
				}
				else                                         //X轴不清零
				{
					if(return_workpiece_zero.X_get_value)     //获取到新坐标值
					{
						Puless_count_clear();
						control_panel_pram.X_value = return_workpiece_zero.Re_X_Value;
						control_panel_pram.Temp_save_Xvalue = return_workpiece_zero.Re_X_Value;						
						return_workpiece_zero.X_get_value=0;     //标志位清零
					}
					else                                      //没有新坐标值
					{
						Puless_count_clear();                //脉冲计数寄存器清零	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.Y_clear_status==0) //Y轴清零
				{
					Puless_count_clear();
					control_panel_pram.Y_value=0;
					control_panel_pram.Temp_save_Yvalue=0;						
										
				}
				else                                        //Y轴不清零
				{
					if(return_workpiece_zero.Y_get_value)     //获取到新坐标值
					{
						Puless_count_clear();
						control_panel_pram.Y_value = return_workpiece_zero.Re_Y_Value;
						control_panel_pram.Temp_save_Yvalue = return_workpiece_zero.Re_Y_Value;													
						return_workpiece_zero.Y_get_value=0;
					}
					else
					{
						 Puless_count_clear();                //脉冲计数寄存器清零	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.Z_clear_status==0)  //Z轴清零
				{
					Puless_count_clear();
					control_panel_pram.Z_value=0;
					control_panel_pram.Temp_save_Zvalue=0;												
				}
				else
				{
					if(return_workpiece_zero.Z_get_value)     //获取到新坐标值
					{
						Puless_count_clear();
						control_panel_pram.Z_value = return_workpiece_zero.Re_Z_Value;
						control_panel_pram.Temp_save_Zvalue = return_workpiece_zero.Re_Z_Value;						
			
						return_workpiece_zero.Z_get_value=0;
					}
					else
					{
						Puless_count_clear();                //脉冲计数寄存器清零	
					
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.A_clear_status==0)  //A轴清零
				{
					Puless_count_clear();
					control_panel_pram.A_value=0;
					control_panel_pram.Temp_save_Avalue=0;					
							
				}
				else
				{
					if(return_workpiece_zero.A_get_value)     //获取到新坐标值
					{
						Puless_count_clear();
						control_panel_pram.A_value = return_workpiece_zero.Re_A_Value;
						control_panel_pram.Temp_save_Avalue = return_workpiece_zero.Re_A_Value;								
						return_workpiece_zero.A_get_value=0;
					}
					else
					{
						Puless_count_clear();                //脉冲计数寄存器清零	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.B_clear_status==0)  //B轴清零
				{
					Puless_count_clear();
					control_panel_pram.B_value = 0;
					control_panel_pram.Temp_save_Bvalue = 0;					
							
				}
				else 
				{
					if(return_workpiece_zero.B_get_value)     //获取到新坐标值
					{
						Puless_count_clear();
						control_panel_pram.B_value = return_workpiece_zero.Re_B_Value;
						control_panel_pram.Temp_save_Bvalue = return_workpiece_zero.Re_B_Value;						
						return_workpiece_zero.B_get_value=0;
					}
					else
					{
						Puless_count_clear();                //脉冲计数寄存器清零	
					}
				}
				
				return_workpiece_zero.Sure=0;				
				Work_Page_Status=ControlPanel_Page;
			} 
			if(return_workpiece_zero.Cancel)        //取消按钮按下
			{
				control_panel_pram.X_value=control_panel_pram.Temp_save_Xvalue;
				control_panel_pram.Y_value=control_panel_pram.Temp_save_Yvalue;
				control_panel_pram.Z_value=control_panel_pram.Temp_save_Zvalue;
				control_panel_pram.A_value=control_panel_pram.Temp_save_Avalue;
				control_panel_pram.B_value=control_panel_pram.Temp_save_Bvalue;					
				Puless_count_clear();                //脉冲计数寄存器清零							
				return_workpiece_zero.Re_X_Value=0;
				return_workpiece_zero.Re_Y_Value=0;
				return_workpiece_zero.Re_Z_Value=0;
				return_workpiece_zero.Re_A_Value=0;
				return_workpiece_zero.Re_B_Value=0;	
				return_workpiece_zero.Cancel=0;
				
			}
			Show_coordinate_on_return_workpiece_zero_page();  //在回工件零页面显示所有轴坐标值				
			
		}
		break;
		case Jump_Work_Page:     //*****************************************************跳行加工页面****************************************************************************************
		{
			if(jump_work_set.First_get_into)
			{
				sprintf(Working_line_buf,"%d",Working_line);  
				SetTextValue(5,4,(uchar *)Working_line_buf);     //显示上次加工行数 
				SetTextValue(5,3,(uchar *)Working_line_buf);     //显示上次加工行数
				jump_work_set.First_get_into=0;
			}
			
		}
			break;
		case File_Manage_Page:    //*****************************************************文件管理页面****************************************************************************************
			break;
		case Leading_In_Page:    //***************************************************** 导入页面****************************************************************************************
			break;
		case Delete_Page:         //*****************************************************删除页面****************************************************************************************
			break;
		case Storage_View_Page:    //*****************************************************内存预览页面****************************************************************************************
			break;	
		case Net_Account_Manage_Page:  //**************************************************网络与账户登录管理**********************************************************************************
			{
				switch(get_control_type)                              //控件类型判断
				{
					case kCtrlButton:                                   //按钮控件
					{							
												
						
					}break;						
					case kCtrlText:                                     //文本控件
					{
						switch(get_control_id)
						{
							case 7:
							{
								SetTextValue(11,8,(uchar *)input_buf);
							
							}
							case 8:
							{
							
							}
							case 9:
							{
							
							}
						
						
						}
						 
					
					}break;						                    
																																																																																																
					default:
							 break;
				}
			
			
			}
			break;
		case Choose_WiFi_Page:        //*****************************************************选择WiFi页面****************************************************************************************
			break;
		case Disconnet_and_SignIn_Page: 
			break;
		case Disconnect_and_SignOut_Page: 
			break;
		case Disconnect_Remind_Page: 
			break;
		case SignOut_Remind_Page: 
			break;
			case Leading_Out_Pgae:
			break;
		default:
			break;
	}

}
	

/****************************************************
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
******************************************************/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
    get_cmd_type = msg->cmd_type;                                         //获取指令类型
    get_ctrl_msg = msg->ctrl_msg;                                         //获取消息的类型	  
    get_control_type = msg->control_type;                                 //获取控件类型
    get_screen_id = PTR2U16(&msg->screen_id);                             //获取画面ID
    get_control_id = PTR2U16(&msg->control_id);                           //获取控件ID
    get_value = PTR2U32(msg->param);                                      //获取数值
	
	  if(get_control_type==kCtrlButton)                                      //按钮控件
			get_button_state= msg->param[1];                                     //获取按钮状态


	  //sprintf((char *)input_buf,"%s",msg->param);
	   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //控件更新通知
    {  
				switch(get_screen_id)                                             //画面ID
				{
					case 0:                                       //画面0：加工页面
					{
							Work_Page_Status=Working_Page;
							switch(get_control_id)
							{
									case 3:                  //主轴速度减
									{
										if(get_button_state)
										{
											  Speed.Spindle_Speed_Changed=Changed;
												Speed.Initial_Spindle_Speed_Percent -= 10; 
												if(Speed.Initial_Spindle_Speed_Percent <= 0)
												{
													Speed.Initial_Spindle_Speed_Percent = 0;
												}
												Speed.Changed_Spindle_Speed=Speed.Initial_Spindle_Speed*Speed.Initial_Spindle_Speed_Percent/100;
												get_button_state=0;
									  }
									}break;	
									case 4:                  //主轴速度加
									{
										if(get_button_state)
										{   
											  Speed.Spindle_Speed_Changed=Changed;
												Speed.Initial_Spindle_Speed_Percent += 10;
												if(Speed.Initial_Spindle_Speed_Percent >= 100)
												{
													Speed.Initial_Spindle_Speed_Percent = 100;
												}
												Speed.Changed_Spindle_Speed=Speed.Initial_Spindle_Speed*Speed.Initial_Spindle_Speed_Percent/100;
												get_button_state=0;
									  }
									}break;
									case 5:                  //加工速度减
									{
										if(get_button_state)
										{
											  Speed.Work_Speed_Changed=Changed;
												if(Speed.Initial_Work_Speed_Percent <= 100)
												{
													Speed.Initial_Work_Speed_Percent -=10;
													if(Speed.Initial_Work_Speed_Percent <= 0)
													{
														Speed.Initial_Work_Speed_Percent=0;
													}
												}
												else if(100 < Speed.Initial_Work_Speed_Percent && Speed.Initial_Work_Speed_Percent <=300)
												{
													Speed.Initial_Work_Speed_Percent  -=25;						
												}
												Speed.Changed_Work_Speed=Speed.Initial_Work_Speed*Speed.Initial_Work_Speed_Percent/100;
		                    get_button_state=0;
									  }												
									}break;
									case 6:                  //加工速度加
									{
										if(get_button_state)
										{
											  Speed.Work_Speed_Changed=Changed;
												if(Speed.Initial_Work_Speed_Percent < 100)
												{
													Speed.Initial_Work_Speed_Percent +=10;
												}
												else if(100 <= Speed.Initial_Work_Speed_Percent && Speed.Initial_Work_Speed_Percent <=300)
												{
													Speed.Initial_Work_Speed_Percent +=25;
													if(Speed.Initial_Work_Speed_Percent >= 300)
													{
														Speed.Initial_Work_Speed_Percent=300;
													}						
												}
												Speed.Changed_Work_Speed=Speed.Initial_Work_Speed*Speed.Initial_Work_Speed_Percent/100;
												get_button_state=0;
									  }	
									}break;
									case 12:Work_Page_Status=File_Manage_Page;break;
									case 13:Work_Page_Status=Net_Account_Manage_Page;break;
									case 14:Work_Page_Status=Setting_page;break;
									case 15:Work_Page_Status=ControlPanel_Page;break;
									case 42:                                           //X轴按钮触发
									{	
//										SetButtonValue(0,42,1);     //X轴保持选中状态
//										SetButtonValue(2,22,1);     //X轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器																		
										control_panel_pram.Axis_press= CMD_X_AXIS;								
									}break;
									case 43:                                           //Y轴按钮触发
									{
//										SetButtonValue(0,43,1);        //Y轴保持选中状态
//										SetButtonValue(2,23,1);        //Y轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);         //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 44:                                              //Z轴按钮触发
									{
//										SetButtonValue(0,44,1);         //Z轴保持选中状态
//										SetButtonValue(2,24,1);         //Z轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 45:                                             //A轴按钮触发
									{
//										SetButtonValue(0,45,1);         //A轴保持选中状态
//										SetButtonValue(2,25,1);         //A轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 46:                                           //B轴按钮触发
									{
//										SetButtonValue(0,46,1);     //B轴保持选中状态
//										SetButtonValue(2,26,1);     //B轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;								
									default:break;
						  }
					}break;
					case 1:                                       //画面1：设置页面
					{
							Work_Page_Status=Setting_page;
						if(state.Work_state==Stop)             //停止加工状态
						{
							switch(get_control_id)                                
							{
									case 1:                           //安全Z按钮触发
									{
										if(get_button_state)
											pram_status.Safe_Z_button_satus=1;
										else
											pram_status.Safe_Z_button_satus=0;													
									}break;
									case 2:                          //自动对刀按钮触发
									{
										if(get_button_state)
											pram_status.Auto_Knife_button_status=1;
										else
											pram_status.Auto_Knife_button_status=0;						
									}break;
									case 3:                          //单位选择按钮触发
									{
										if(get_button_state)
											pram_status.Unit_Change_button_status=1;
										else
											pram_status.Unit_Change_button_status=0;						
									}break;
									case 6:                          //获取安全Z高度
									{
										char buf[20];
										pram_status.Safe_Z_num=NotifyText(msg->param);										
										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 7:                   //获取对刀高度
									{
										char buf[20];
										pram_status.Knife_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8:                     //获取对刀块高度
									{
										char buf[20];
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9:                  //获取对刀块X轴位置
									{
										char buf[20];
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10:                 //获取对刀块Y轴位置
									{
										char buf[20];
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11:                 //获取对刀块Z 轴位置
									{
										char buf[20];
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12:                  //获取软限位X轴位置
									{
										char buf[20];
										pram_status.Soft_limit_X=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13:                   //获取软限位Y轴位置
									{
										char buf[20];
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14:                 //获取软限位Z轴位置
									{
										char buf[20];
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15:                 //获取软限位A轴位置
									{ 
										char buf[20];
										pram_status.Soft_limit_A=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16:                  //获取软限位B轴位置
									{
										char buf[20];
										pram_status.Soft_limit_B=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 27:                           //语音提示按钮触发 
									{
										if(get_button_state)                                
											pram_status.voice_button_status=voice_off; //按钮按下，语音提醒关
										else
											pram_status.voice_button_status=voice_on;  //按钮松开，语音提醒开													
									}break;
									case 4:                             //确定按钮按下
									{
		//								pram_status.Screen_ID1_Setting_Sure=1;	
		//                pram_status.Screen_ID1_Setting_concel=0;								
									}break;
									case 5:                             //取消按钮按下
									{
		//								pram_status.Screen_ID1_Setting_Sure=0;
										pram_status.Screen_ID1_Setting_concel=1;						
									}break;
								
									default: break;		
                 }									
						}					
					}break;   
					case 2:                                       //画面2：控制面板
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //清零按钮触发
									{
										if(state.Work_state == Start)    //开始加工
										{									
										}
										else                          //停止加工
										{
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Clear;								
										  }											
										}
									}break;    
                  case 2:                                             //回机械零按钮触发	
                  {
										if(state.Work_state == Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Return_Machine_Zero;
                          												
											}
											get_button_state=0;	
										}
									}break;                                     									
									case 3:								                              //倍率切换按钮触发
									{
										if(state.Work_state==Start)      //开始加工
										{
										}
										else                             //停止加工
										{
											if(get_button_state)
											{
													control_panel_pram.Override_Change_button=1;
													Override_Change_Process();     //倍率切换控制											
											}
											get_button_state=0;	
										}											
									}break;
								  case 4:                                             //主轴开关按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Spin_On_Off;						
											}
											get_button_state=0;	
										}
									}break;
			            case 5:                                              //全轴清零按钮触发
									{
                    if(state.Work_state==Start)     //开始加工
										{
										}
										else                            //停止加工
										{										
											control_panel_pram.All_Spindle_Clear_Button=1;
										}
									}break;
									case 6:                                            //回工件零按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											Work_Page_Status=Return_WorkPiece_Zero_Page;
											first_time_re_workpiece=1;
										}
									}break;
									case 7:                                            //坐标切换按钮触发
									{
										if(state.Work_state==Start)     //开始加工
										{
										}
										else                           //停止加工
										{
											if(get_button_state)
											{										
												  Coordinate_Change_Process();//坐标切换控制
												  get_button_state=0;
											 }
										}											
									}break;
									case 8:                                            //软限位开关按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Soft_Limit;						
											}
											get_button_state=0;	
										}
									}break;
									case 9:                                            // 
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
										}
									}break;
									case 10:                                         //跳行加工按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											jump_work_set.First_get_into = 1;
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 11:                                          //对刀按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{											
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Auto_knife;								
											}
											get_button_state=0;	
										}
									}break;
									case 12:                                         //分中按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{	
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Divided;								
											}
											get_button_state=0;	
										}
									}break;
									case 13:                                           //开始按钮触发
									{
										state.Work_state=Start;
										TIM_Cmd(TIM4, DISABLE);      //禁止 TIM4
										SetButtonValue(2,14,0);     //停止按钮松开状态
										SetButtonValue(2,13,1);     //开始按钮按下状态										
									}break;
									case 14:                                           //停止按钮触发
									{
										state.Work_state=Stop;

										SetButtonValue(2,13,0);     //开始按钮松开状态
										SetButtonValue(2,14,1);     //停止按钮按下状态
                    
									  TIM_Cmd(TIM4, ENABLE);      //使能 TIM4
																	
									}break;
									case 15:                                            //复位按钮触发
									{
										state.Work_state=Stop;
										SetButtonValue(2,13,0);      //开始按钮松开状态
										SetButtonValue(2,14,1);      //停止按钮按下状态						
									}break;
									case 16:                                          //退出控制面板按钮触发
									{
										Work_Page_Status=Working_Page;
									}
									break;
									case 22:                                            //X轴按钮触发
									{	
//										SetButtonValue(2,22,1);     //X轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器																		
										control_panel_pram.Axis_press= CMD_X_AXIS;								
									}break;
									case 23:                                            //Y轴按钮触发
									{
//										SetButtonValue(2,23,1);        //Y轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);         //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 24:                                              //Z轴按钮触发
									{
//										SetButtonValue(2,24,1);         //Z轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 25:                                             //A轴按钮触发
									{
//										SetButtonValue(2,25,1);         //A轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 26:                                           //B轴按钮触发
									{
//										SetButtonValue(2,26,1);     //B轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器										
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;
									default:break;				
							}
					}break;
					case 3:                                        //画面3：回工件零页面
					{
						Work_Page_Status=Return_WorkPiece_Zero_Page;
						if(state.Work_state==Stop)    //停止加工状态
						{
							switch(get_control_id)                                
							{
									case 1:                  //全轴按钮触发
									{
										if(get_button_state)            //按钮是按下状态  
										{
											return_workpiece_zero.all_spindle_status=1; //此时全轴按钮按下，XYZAB轴显示成没有选中状态
											SetButtonValue(3,4,1);
											SetButtonValue(3,5,1);
											SetButtonValue(3,6,1);
											SetButtonValue(3,7,1);
											SetButtonValue(3,8,1);
											return_workpiece_zero.X_clear_status=1;
											return_workpiece_zero.Y_clear_status=1;
											return_workpiece_zero.Z_clear_status=1;
											return_workpiece_zero.A_clear_status=1;
											return_workpiece_zero.B_clear_status=1;										
										}
										else 
										{
											return_workpiece_zero.all_spindle_status=0;
											return_workpiece_zero.X_clear_status=0;
											return_workpiece_zero.Y_clear_status=0;
											return_workpiece_zero.Z_clear_status=0;
											return_workpiece_zero.A_clear_status=0;
											return_workpiece_zero.B_clear_status=0;
											
										}
									
									}break;
									case 2:                     //取消按钮触发
									{
									  TIM_Cmd(TIM4, ENABLE);      //使能 TIM4
										if(get_button_state) 
										{
											return_workpiece_zero.Cancel=1;  //取消按钮按下											
										}
										else
										{
											return_workpiece_zero.Cancel=0;  //取消按钮松开
											Work_Page_Status=ControlPanel_Page;
											
										}										
									
									}break;
									case 3:                   //确定按钮触发
									{
										TIM_Cmd(TIM4, ENABLE);      //使能 TIM4
										if(get_button_state) 
										{
											return_workpiece_zero.Sure=1;  //确定按钮按下											
										}
										else
										{
											Work_Page_Status=ControlPanel_Page;
										}
																		
									}break;
									case 4:                           //X轴按钮触发
									{
										if(get_button_state) 
										{											
										  return_workpiece_zero.X_clear_status=1;
										}
										else
										{
											return_workpiece_zero.X_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 5:                           //Y轴按钮触发
									{
										if(get_button_state) 
										{											
										  return_workpiece_zero.Y_clear_status=1;
										}
										else
										{
											return_workpiece_zero.Y_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 6:                           //Z轴按钮触发
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.Z_clear_status=1;
										}
										else
										{
											return_workpiece_zero.Z_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 7:                           //A轴按钮触发
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.A_clear_status=1;
										}
										else
										{
											return_workpiece_zero.A_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 8:                           //B轴按钮触发
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.B_clear_status=1;
										}
										else
										{
											return_workpiece_zero.B_clear_status=0;
                      return_workpiece_zero.all_spindle_status=0;											
										}											
									}break;
									case 9:                    //通过数字键盘更改X轴坐标值
									{
									  
										return_workpiece_zero.Re_X_Value = NotifyText(msg->param);     //获取新坐标值
										return_workpiece_zero.X_get_value = 1;                         //获取到新坐标值，标志位置1
										control_panel_pram.X_value = return_workpiece_zero.Re_X_Value;
										
									}break;
									case 10:                 //通过数字键盘更改Y轴坐标值
									{
									  
										return_workpiece_zero.Re_Y_Value = NotifyText(msg->param);
										return_workpiece_zero.Y_get_value = 1;                         //获取到新坐标值，标志位置1
										control_panel_pram.Y_value = return_workpiece_zero.Re_Y_Value;
									}break;
									case 11:                  //通过数字键盘更改Z轴坐标值
									{
									 
										return_workpiece_zero.Re_Z_Value = NotifyText(msg->param);
										return_workpiece_zero.Z_get_value = 1;                         //获取到新坐标值，标志位置1
										control_panel_pram.Z_value = return_workpiece_zero.Re_Z_Value;	

									}break;
									case 12:                  //通过数字键盘更改A轴坐标值
									{
									  
										return_workpiece_zero.Re_A_Value = NotifyText(msg->param);
										return_workpiece_zero.A_get_value = 1;                         //获取到新坐标值，标志位置1
										control_panel_pram.A_value = return_workpiece_zero.Re_A_Value;
									}break;
									case 13:                  //通过数字键盘更改B轴坐标值
									{
									  
										return_workpiece_zero.Re_B_Value = NotifyText(msg->param);
										return_workpiece_zero.B_get_value = 1;                         //获取到新坐标值，标志位置1
										control_panel_pram.B_value = return_workpiece_zero.Re_B_Value;

									}break;
									default:break;
							}
						}
						else
						{
						
						}
					}break; 
					case 5:                                         //画面5:跳行加工页面
					{
						switch(get_control_id)
						{
							case 1:                         //确定按钮触发
							{
							  jump_work_set.Jump_Work_Sure = 1;
								jump_work_set.Jump_Work_cancel = 0;
								Working_line = jump_work_set.New_work_line;
								Work_Page_Status = ControlPanel_Page;
							}break;
							case 2:                        //取消按钮触发
							{
							  jump_work_set.Jump_Work_cancel = 1;
								jump_work_set.Jump_Work_Sure = 0;
								Work_Page_Status = ControlPanel_Page;
							}break;
							case 3:                        //获取新的行数
							{
							  jump_work_set.New_work_line=NotifyText(msg->param);
							}break;
						}
						
					}break;              
					case 6:                                                     //画面6
					{
						Work_Page_Status=File_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Leading_In_Page;break;
							case 2: Work_Page_Status=Leading_Out_Pgae;break;
							case 3: Work_Page_Status=Delete_Page;break;
							case 4: Work_Page_Status=Storage_View_Page;break;
							case 5: Work_Page_Status=Working_Page;break;
							case 18:Work_Page_Status=Working_Page;break;
							case 19:Work_Page_Status=Net_Account_Manage_Page;break;
							default:break;
						}
					}break;
					case 7: Work_Page_Status=Leading_In_Page;break;            //画面7
					case 9: Work_Page_Status=Delete_Page;break;                //画面8
					case 10: Work_Page_Status=Storage_View_Page;break;         //画面10
					case 11:                                                   //画面10
					{
						Work_Page_Status=Net_Account_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Working_Page;break;
							case 2: Work_Page_Status=File_Manage_Page;break;
							case 3: Work_Page_Status=Choose_WiFi_Page;break;
							default:break;
						}
					}break;
					case 12: Work_Page_Status=Choose_WiFi_Page;break;
					case 13: Work_Page_Status=Disconnet_and_SignIn_Page;break;
					case 14: Work_Page_Status=Disconnect_and_SignOut_Page;break;
					case 15: Work_Page_Status=Disconnect_Remind_Page;break;
					case 16: Work_Page_Status=SignOut_Remind_Page;break;
					case 20: Work_Page_Status=Leading_Out_Pgae;break;
					case 21:                                                  //画面21 保存参数设置提醒页面
					{
						Work_Page_Status=Save_Pram_Page;
						if(state.Work_state==Stop)    //停止加工状态
						{
								switch(get_control_id)
								{
									case 1:    //确定按钮触发
									{							
										pram_status.Screen_ID21_Setting_Sure=1;
										pram_status.Screen_ID21_Setting_concel=0;
									}break;
									case 2:   //取消按钮触发
									{
										pram_status.Screen_ID21_Setting_Sure=0;
										pram_status.Screen_ID21_Setting_concel=1;						
									}
									default: break;	
								}
					  }
					}break;
					default: break;						
  			}
        
     }
		 
}


/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param str 文本控件内容
*  \pram  value 存放文本内容
*/
float NotifyText(uint8 *str)
{                                                                          
		float value=0;                                                            
		sscanf((char *)str,"%f",&value);            
     return  value;                                                                      
}                                                                                

