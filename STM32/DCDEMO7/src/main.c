
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


#define Start 1
#define Stop  0
#define TIME_100MS 10                                //100毫秒(10个单位)
volatile uint32  timer_tick_count = 0;               //定时器节拍
uint8  cmd_buffer[CMD_MAX_SIZE];                     //指令缓存
uint8  USART2_RX_STA=0;                              //USART2接收状态标记


uint8  Work_Page_Status = 0;              //工作页面标记
uint8  get_cmd_type;                     //获取指令类型
uint8  get_ctrl_msg;                     //获取消息的类型
uint8  get_button_state;                 //获取按钮状态
uint8  get_control_type;                 //获取控件类型
uint16 get_screen_id;                    //获取画面ID
uint16 get_control_id;                   //获取控件ID
uint32 get_value;                        //获取数值
uint8  input_buf[20];                    //键盘输入内容

int32 Pulses_counter;              // 手轮脉冲数量
uint8 first_time_power_on=0;       //掉电后第一次开机标志位
uint8 Send_cooddinate_status;      //发送坐标标记位
int32 Working_line;                //加工行数

int32 Pulses_num_temp;             //临时保存脉冲数量
uint8 file_name[20];               //文件名
char  Working_line_buf[10];        //保存加工行数

State state;                             //申明工作状态结构体变量
Override override;                       //声明倍率相关结构体
Speed_Control Speed;                     //声明主轴速度相关的结构体变量
Pram_Status pram_status;                 //声明设置页面相关参数的结构体变量
Control_Panel_Pram control_panel_pram;   //声明控制面板相关参数的结构体变量


/******************************************************************************************************/
/*!                                                                                 
*  \brief  程序入口                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
{  
	qsize  size = 0;          //指令长度 

	Set_System();             //配置时钟                                                                                                                                 
	Interrupts_Config();      //配置串口中断                                                                                                                             
	systicket_init();         //配置时钟节拍	
	Usart1_Init(115200);      //串口1初始化(与TFT屏通讯)
  Usart2_Init(115200);      //串口2初始化(与雕刻机通讯)
	queue_reset();            //清空串口接收缓冲区 
	TIME2_Init();             //定时器2初始化(向主机发送坐标)
	TIME3_Init();             //定时器3初始化(向主机询问坐标)
  TIME4_Init();             //定时器4初始化(计算脉冲)
	TIM_Cmd(TIM4, DISABLE);   //关闭TIM4定时器
	                                                                                               
	delay_ms(300);            //延时等待串口屏初始化完毕,必须等待300ms  
	
	Power_On_Set();                 //开机动画和参数初始化设置
		
	Setting_page_pram_get();        //设置页面相关参数值恢复（断电保存在flash）
	
  if(first_time_power_on==0)      //断电后第一次开机执行这个操作
  {
		Return_last_status();         //恢复上一次设置状态
		first_time_power_on=1;
  }
	
		
		//    特别注意
		//    MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		//    1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		//    2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
	
	while(1)                                                                        
	{
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);                  //从USART1缓冲区中获取一条指令，得到指令长度       
		if(size>0 && cmd_buffer[1]!=0x07)                                  //接收到指令 ，及判断是否为开机提示
		{                                                                           
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);                   //指令分析处理 ，判断主程序进入哪个Work_Page_Status
			memset(cmd_buffer, 0, CMD_MAX_SIZE);
		} 
		
		
    if(USART2_RX_STA)                                               //串口2接收到了数据
		{
		  uart2_command_handle();                                       //数据分析		
		}


		if(state.Work_state==Start)          //显示当前处于加工状态
		{
			WorkingStatus_Starting();       
		}
		else                                 //显示当前已经停止加工
		{
			WorkingStatus_Stoped();  
		}


		switch(Work_Page_Status)
		{
		
			case Working_Page: //*********************************************************加工页面***************************************************************************************************
			{	
        				
				Spindle_and_Work_Speed_Key_Process();	           	//加工中心主轴速度和加工速度按钮处理
				if(state.Work_state==Start)                        //机器处于加工状态中
				{
					  TFT_Show_coordanate_value();			            //显示工件和机械坐标
						SetTextValue(0,21,(uchar *)file_name);        //显示正在加载的文件名			
						sprintf(Working_line_buf,"%d",Working_line);  
						SetTextValue(0,22,(uchar *)Working_line_buf); //显示加工行数，需要向主机询问
				}						    							       			
			}break;
		
		 case Setting_page:  //***********************************************************设置页面*************************************************************************************************
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
				if(state.Work_state==Stop)      //停止加工
				{
					if(pram_status.Screen_ID21_Setting_Sure)                    //“确定按钮”按下
					{
						Speaker_Key_Process(pram_status.voice_button_status);     //语音提示按钮处理函数
						Safe_Z_process(pram_status.Safe_Z_button_satus);           //安全Z触发处理函数
						Auto_Knife_process(pram_status.Auto_Knife_button_status);  //自动对刀按钮触发后处理程序
						Unit_Change_process(pram_status.Unit_Change_button_status); //单位切换按钮触发后处理程序
						Work_Page_Status=Working_Page;
					}
					if(pram_status.Screen_ID21_Setting_concel)                    //取消按钮按下
					{
						Return_last_status();                                      //恢复上一次设置状态
						Work_Page_Status=Working_Page;					
					}	
			  }				
			}
			break;	
			case ControlPanel_Page:  //******************************************************控制面板页面*****************************************************************************************
			{
				char buf2[20];
        if(state.Work_state==Stop)	                          //停止加工
				{					
					if(control_panel_pram.X_press)                       //X轴选定状态***************************************************************                   
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_X != override.Override_num)   //倍率切换
							{	 
							   control_panel_pram.X_value_temp = control_panel_pram.X_value;
								 override.Override_num_temp_X = override.Override_num;
								 TIM4->CNT = 0;
								 Pulses_counter = 0;
							   Pulses_num_temp = 0;
							}
              Get_Pulses_num();   //计算脉冲个数							
							control_panel_pram.X_Pulses_counter=Pulses_counter;						
							control_panel_pram.X_value=control_panel_pram.X_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.X_value);
							SetTextValue(2,17,(uchar *)buf2);	        //在手轮上显示工件坐标
							SetTextValue(0,16,(uchar *)buf2);         //在手轮上显示工件坐标
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //清零按钮触发
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
							Pulses_num_temp=0;
							control_panel_pram.X_value_temp=0;
							control_panel_pram.X_Pulses_counter=0;							
              X_coordinate_clear();                    //对X轴工件坐标清零	
							control_panel_pram.Clear_Button=0;						
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
						{
							
							Pulses_num_Clear();                    //脉冲数量清零
						  All_Workpiece_coordinate_clear();      //对所有轴工件坐标清零							
							XYZAB_button_reset();                  //XYZAB坐标按钮复位
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
						
					}
					else if(control_panel_pram.Y_press)                   //Y轴选定状态*******************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_Y!= override.Override_num)   //倍率切换
							{	 
							   control_panel_pram.Y_value_temp=control_panel_pram.Y_value;
								 override.Override_num_temp_Y=override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.Y_Pulses_counter=Pulses_counter;
							control_panel_pram.Y_value=control_panel_pram.Y_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.Y_value);
							SetTextValue(2,18,(uchar *)buf2);
							SetTextValue(0,17,(uchar *)buf2);         //在手轮上显示工件坐标
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //清零按钮触发
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.Y_value_temp=0;
							control_panel_pram.Y_Pulses_counter=0;
							delay_ms(20);							
              Y_coordinate_clear();                   //对Y轴工件坐标清零
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
						{
							Pulses_num_Clear();                    //脉冲数量清零
						  All_Workpiece_coordinate_clear();      //对所有工件坐标清零														
							XYZAB_button_reset();                   //XYZAB坐标按钮复位
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
						
					}
					else if(control_panel_pram.Z_press)                   //Z轴选定状态**********************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_Z != override.Override_num)   //倍率切换
							{	 
							   control_panel_pram.Z_value_temp=control_panel_pram.Z_value;
								 override.Override_num_temp_Z = override.Override_num;
								 TIM4->CNT=0;
							 	 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.Z_Pulses_counter=Pulses_counter;
							control_panel_pram.Z_value=control_panel_pram.Z_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.Z_value);
							SetTextValue(2,19,(uchar *)buf2);
							SetTextValue(0,18,(uchar *)buf2);        //在手轮上显示工件坐标
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //清零按钮触发
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.Z_value_temp=0;
							control_panel_pram.Z_Pulses_counter=0;
							delay_ms(20);						
              Z_coordinate_clear();         //对Z轴工件坐标清零
							control_panel_pram.Clear_Button=0;						
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
						{
							Pulses_num_Clear();                    //脉冲数量清零
						  All_Workpiece_coordinate_clear();      //对所有工件坐标清零								
							XYZAB_button_reset();                  //XYZAB坐标按钮复位
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
					
					}
					else if(control_panel_pram.A_press)                   //A轴选定状态************************************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_A != override.Override_num)   //倍率切换
							{	 
							   control_panel_pram.A_value_temp = control_panel_pram.A_value;
								 override.Override_num_temp_A = override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.A_Pulses_counter=Pulses_counter;
							control_panel_pram.A_value=control_panel_pram.A_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.A_value);
							SetTextValue(2,20,(uchar *)buf2);
							SetTextValue(0,19,(uchar *)buf2);         //在手轮上显示工件坐标
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //清零按钮触发
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.A_value_temp=0;
							control_panel_pram.A_Pulses_counter=0;
							delay_ms(20);
							
              A_coordinate_clear();                  //对A轴工件坐标清零
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
						{
							Pulses_num_Clear();                    //脉冲数量清零
						  All_Workpiece_coordinate_clear();      //对所有工件坐标清零								
							XYZAB_button_reset();                  //XYZAB坐标按钮复位
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
																							
					}
					else if(control_panel_pram.B_press)                   //B轴选定状态*****************************************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_B!= override.Override_num)   //倍率切换
							{	 
							   control_panel_pram.B_value_temp=control_panel_pram.B_value;
								 override.Override_num_temp_B= override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.B_Pulses_counter=Pulses_counter;
							control_panel_pram.B_value=control_panel_pram.B_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.B_value);
							SetTextValue(2,21,(uchar *)buf2);
							SetTextValue(0,20,(uchar *)buf2);         //在手轮上显示工件坐标
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //清零按钮触发
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.B_value_temp=0;
							control_panel_pram.B_Pulses_counter=0;
							delay_ms(20);		
              B_coordinate_clear();                      	//对B轴工件坐标清零
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //全轴清零触发
						{
							Pulses_num_Clear();                  //脉冲数量清零
						  All_Workpiece_coordinate_clear();    //对所有工件坐标清零								
							XYZAB_button_reset();                //XYZAB坐标按钮复位
							control_panel_pram.All_Spindle_Clear_Button=0;
						}				
					}
					
					if(control_panel_pram.X_press || control_panel_pram.Y_press || control_panel_pram.Z_press || control_panel_pram.A_press || control_panel_pram.B_press)
					{
						TIM_Cmd(TIM2, ENABLE);                           //使能TIM2，定时向主机发送坐标
						if(Send_cooddinate_status)
						{
					     Send_Coordinate_to_Host_Machine();            //向主机发送坐标	
							 Send_cooddinate_status=0;
						}							
					}
					else 
					{
					   TIM_Cmd(TIM2, DISABLE);
					}
          					
			  }
				else                                                 //机器处于加工状态中
				{
					 TIM_Cmd(TIM3, ENABLE);                        //使能 TIM3
					// TIM_Cmd(TIM4, DISABLE);                       //禁止TIM4
					 TFT_Show_coordanate_value();			             //显示工件和机械坐标
					
					 SetTextValue(0,21,(uchar *)file_name);        //显示正在加载的文件名	         				
					 sprintf(Working_line_buf,"%d",Working_line);  
					 SetTextValue(0,22,(uchar *)Working_line_buf); //显示加工行数，需要向主机询问
				
				}
				
			}break;
			case Return_WorkPiece_Zero_Page: 
				break;
			case Jump_Work_Page:
				break;
			case File_Manage_Page: 
				break;
			case Leading_In_Page: 
				break;
			case Delete_Page: 
				break;
			case Storage_View_Page: 
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
			case Choose_WiFi_Page: 
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
}




/***
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
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
					case 0:                                       //画面0
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
									default:break;
						  }
					}break;
					case 1:                                       //画面1
					{
							Work_Page_Status=Setting_page;
						if(state.Work_state==Stop)    //停止加工状态
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
									case 6:
									{
										char buf[20];
										pram_status.Safe_Z_num=NotifyText(msg->param);										
										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 7:
									{
										char buf[20];
										pram_status.Knife_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8: 
									{
										char buf[20];
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12: 
									{
										char buf[20];
										pram_status.Soft_limit_X=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13: 
									{
										char buf[20];
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14: 
									{
										char buf[20];
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15: 
									{
										char buf[20];
										pram_status.Soft_limit_A=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16: 
									{
										char buf[20];
										pram_status.Soft_limit_B=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
		                Usart_SendString(USART2,(char *)buf);       
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
					case 2:                                       //画面2
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //清零按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                          //停止加工
										{
											control_panel_pram.Clear_Button=1;     
										}
									}break;    
                  case 2:                                             //回机械零按钮触发	
                  {
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
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
													Override_Change_Process();//倍率切换控制											
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
												TIM_Cmd(TIM4, DISABLE);       //关闭TIM4定时器
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
										}
									}break;
									case 9:                                            //安全Z按钮触发
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
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 12:                                         //分中按钮触发
									{
										if(state.Work_state==Start)    //开始加工
										{									
										}
										else                           //停止加工
										{
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 13:                                           //开始按钮触发
									{
										state.Work_state=Start;
										TIM_Cmd(TIM3, ENABLE);      //使能 TIM3
										TIM_Cmd(TIM4, DISABLE);      //禁止 TIM4
										SetButtonValue(2,14,0);     //停止按钮松开状态
										SetButtonValue(2,13,1);     //开始按钮按下状态										
									}break;
									case 14:                                           //停止按钮触发
									{
										state.Work_state=Stop;
										TIM_Cmd(TIM3, DISABLE);     //禁止 TIM3
										SetButtonValue(2,13,0);     //开始按钮松开状态
										SetButtonValue(2,14,1);     //停止按钮按下状态
                    if(control_panel_pram.X_press || control_panel_pram.Y_press || control_panel_pram.Z_press || control_panel_pram.A_press || control_panel_pram.B_press)
										{
										  TIM_Cmd(TIM4, ENABLE);      //使能 TIM4
										}											
									}break;
									case 15:                                            //复位按钮触发
									{
										state.Work_state=Stop;
										TIM_Cmd(TIM3, DISABLE);      //禁止 TIM3
										SetButtonValue(2,13,0);      //开始按钮松开状态
										SetButtonValue(2,14,1);      //停止按钮按下状态						
									}break;
									case 16:Work_Page_Status=Working_Page;break;
									case 22:                                            //X轴按钮触发
									{	
										SetButtonValue(2,22,1);     //X轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器									
										if(control_panel_pram.X_press==0)
										{
											TIM4->CNT = control_panel_pram.X_Pulses_counter*4;
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=1;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;									
									}break;
									case 23:                                            //Y轴按钮触发
									{
										SetButtonValue(2,23,1);        //Y轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);         //启动TIM4定时器
										if(control_panel_pram.Y_press==0)
										{
											 TIM4->CNT = control_panel_pram.Y_Pulses_counter*4;
                       Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=1;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;
							
									}break;
									case 24:                                              //Z轴按钮触发
									{
										SetButtonValue(2,24,1);         //Z轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器
										if(control_panel_pram.Z_press==0)
										{
											TIM4->CNT = control_panel_pram.Z_Pulses_counter*4;
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=1;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;						
									}break;
									case 25:                                             //A轴按钮触发
									{
										SetButtonValue(2,25,1);         //A轴保持选中状态
										TIM_Cmd(TIM4, ENABLE);          //启动TIM4定时器
										if(control_panel_pram.A_press==0)
										{
											TIM4->CNT = control_panel_pram.A_Pulses_counter*4;	
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=1;
										control_panel_pram.B_press=0;						
									}break;
									case 26:                                           //B轴按钮触发
									{
										  SetButtonValue(2,26,1);     //B轴保持选中状态
											TIM_Cmd(TIM4, ENABLE);      //启动TIM4定时器
											if(control_panel_pram.B_press==0)
											{
												TIM4->CNT = control_panel_pram.B_Pulses_counter*4;
                        Pulses_counter=0;												
											}
											control_panel_pram.X_press=0;
											control_panel_pram.Y_press=0;
											control_panel_pram.Z_press=0;
											control_panel_pram.A_press=0;
											control_panel_pram.B_press=1;
									}break;
									default:break;				
							}
					}break;
					case 3: Work_Page_Status=Return_WorkPiece_Zero_Page;break;  //画面3
					case 5: Work_Page_Status=Jump_Work_Page;break;              //画面5
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
					case 21:                                                  //画面21
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
		sscanf((char *)str,"%f",&value);                                            //把字符串转换为整数 
     return  value;                                                                      
}                                                                                

