
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
uint8  cmd_buffer[CMD_MAX_SIZE];                     //LCD指令缓存

uint8  TX_Data [30]; //the sending package    存储串口3要发送的数据（主程序发送）
uint8  RX_Data [30]; //the receiving package  接收串口3发来的数据（在中断中接收）

uint8  Work_Page_Status = 0;             //工作页面标记
uint8  get_cmd_type;                     //获取指令类型
uint8  get_ctrl_msg;                     //获取消息的类型
uint8  get_button_state;                 //获取按钮状态
uint8  get_control_type;                 //获取控件类型
uint16 get_screen_id;                    //获取画面ID
uint16 get_control_id;                   //获取控件ID
uint32 get_value;                        //获取数值
uint8  input_buf[20];                    //键盘输入内容

uint8  Press_button=0XFF;          //记录哪个按钮触发
uint8 last_time_work_state=0;      //记录上一次机器工作状态,默认是关机状态
uint16 Pulses_counter;              // 手轮脉冲数量
uint8 Override_num;                //倍率
uint8 first_time_re_workpiece;     //首次进入回工件零页面
uint8 Pulses_check=1;              //脉冲同步标志位
uint8 Pulses_count_mark=1;           //脉冲计数是能标记位
int32 Working_line;                //加工行数
uint8 Mark_10ms=0;                 //10ms计时标记位
uint8 Mark_10ms_Count;             //10ms计时溢出统计位
uint8 Mark_20ms=0;                 //20ms计时标记位
uint8 Mark_20ms_Count=0;           //20ms计时
uint8 Mark_60ms=0;                //100ms计时标记位

uint8 Cloud_File_Button=0;        //云空间文件选中按钮
uint8 SD_File_Button=0;           //SD卡文件选中按钮
uint8 Download_Per=0;             //已经下载百分比
uint8 Start_Download=0;           //开始下载标志位，0：没下载，1：正在下载
uint8 Cloud_Page=1;               //云空间页面
uint8 Last_time_Cloud_Page=0;
uint8 SD_Page=1;                  //SD卡页面
uint8 Last_time_SD_Page=0;
uint8 WiFi_Page=1;
uint8 Last_time_WiFi_Page=0;

uint8 Network_Control=4;   //0:无效，1：显示连接网络控件，2：隐藏连接网络控件，3：显示断开连接控件，4：隐藏断开连接控件
uint8 Sign_Control=4;      //0:无效，1：显示立即登录控件，2：隐藏立即登录控件，3：显示退出登录控件，4：隐藏退出登录控件
uint8 WiFi_Password_Right=0;  //网络密码正确
uint8 Account_Password_Right=0;//账户密码正确

uint8 file_name[20]="精雕佛像";    //文件名
char  Working_line_buf[20];        //保存加工行数

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
  uint16 time_conuter=0;	
  uint16 Recdata1,Recdata2;	 
	uint8  check_time=0;      //脉冲同步检查次数
	Set_System();             //配置时钟                                                                                                                                                                                                                                                              
	systicket_init();         //配置时钟节拍
	Usart1_Init(115200);      //串口1初始化(与LCD屏通讯)	 	 
	queue_reset();            //清空串口接收缓冲区 
	TIME2_Init();             //定时器2初始化
	//TIME3_Init();             //定时器3初始化(向主机询问坐标)
  TIME4_Init();             //定时器4初始化(计算手轮脉冲)                                                                                              
	delay_ms(300);            //延时等待串口屏初始化完毕,必须等待300ms  
	Usart3_Init(115200);      //串口3初始化(与雕刻机通讯)
	
	 
	Power_On_Set();                 //串口屏开机动画和参数初始化设置	
	Setting_page_pram_get();        //获取串口屏设置页面相关参数值（保存在flash）	
	Return_last_status();           //恢复上一次串口屏设置页面相关参数值

		//    MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		//    1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		//    2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。
		
//	while(Pulses_check)        //开机同步主机脉冲
//	{
//		if(RX_Data[1] == CMD_UPDATE_MACH3_NUMBER)
//		{
//			check_time++;
//			Recdata1=RX_Data[18];
//			Recdata2=RX_Data[19];	
//			TIM4->CNT = (Recdata1<<8)+Recdata2;  //同步脉冲
//      if(check_time>5)
//        Pulses_check=0;
//			
//      sprintf(Working_line_buf,"%u",Pulses_counter);	
//			SetTextValue(0,22,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问			
//		}
//	}
	  
		
	while(1)                                                                        
	{
		//time_conuter++;
		
		Pulses_Count();                        //计算手轮脉冲
		Usart3_Receive_Data_handle();          //与雕刻机通讯，处理相关数据 
	
		if(Mark_20ms) 
		{
		  LCD_command_analyse();                 //分析LCD屏的命令 			  
			Mark_20ms=0;
		}
		if(Mark_60ms)                         //定时满60ms
		{
			LCD_Show_coordanate_value();        //显示工件坐标
			Mark_60ms=0;
			
			if(Start_Download)
			{
				time_conuter++;
				if(time_conuter==1)
				{
					time_conuter=0;
					Download_Per++;
				}
		 }
		}
		LCD_handle();                          //处理LCD屏数据
			
    	
//		sprintf(Working_line_buf,"%d",Pulses_counter);  
//		SetTextValue(0,23,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问
	}  
}





//对来自LCD屏的命令进行分析
void LCD_command_analyse(void)
{
	qsize  size = 0;                                             //指令长度 
  size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);              //接收到LCD屏的数据，从USART1的指令缓冲区cmd_buffer中获取一条指令，得到指令长度       
	if(size>0 && cmd_buffer[1]!=0x07)  //接收到指令 ，及判断是否为开机提示
	{                                                                           
		Usart1_Receive_data_handle((PCTRL_MSG)cmd_buffer, size);           //指令分析处理 ，标记应该进入哪个Work_Page_Status，标记相应的操作位
		memset(cmd_buffer, 0, CMD_MAX_SIZE);                       //对指令缓冲cmd_buffer清零
	}
}


//计算首轮脉冲个数
void Pulses_Count(void)
{
	if( Work_Page_Status == Working_Page || Work_Page_Status == ControlPanel_Page )
	{
		if(Pulses_count_mark)
		{
		  TIM_Cmd(TIM4, ENABLE);
			Pulses_count_mark=0;
		}
		Get_Pulses_num();   
		sprintf(Working_line_buf,"%u",Pulses_counter);  
		SetTextValue(0,22,(uchar *)Working_line_buf);     //显示脉冲个数 
	}
	else
	{
	  TIM_Cmd(TIM4, DISABLE);          //禁止 TIM4，脉冲不计数
		Pulses_count_mark=1;
	}
	

}
/************************************************************
*
*程序进入标记的工作页面，处理相关任务
*
*************************************************************/
void LCD_handle(void)
{
	
  Work_state_control();   //主机工作状态显示
	
	switch(Work_Page_Status)
	{	
		case Working_Page: //*********************************************************加工页面***************************************************************************************************
		{	
			
			//LCD_Show_coordanate_value();		                  //串口屏显示工件和机械坐标	
			Spindle_and_Work_Speed_Key_Process();	           	//加工中心主轴速度和加工速度按钮处理
	
			//SetTextValue(0,21,(uchar *)file_name);          //显示正在加载的文件名	

//			sprintf(Working_line_buf,"%d",Pulses_counter);  
//			SetTextValue(0,22,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问
			
		}break;
		case Setting_page:  //********************************************************设置页面*************************************************************************************************
		{
		 if(pram_status.Screen_ID1_Setting_concel)          //取消按钮按下
		 {
				Return_last_status();                           //恢复上一次设置状态 
				pram_status.Screen_ID1_Setting_concel=0;
				Work_Page_Status=Working_Page;				 
		 }		
		}break;
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
		} break;	
		case ControlPanel_Page:  //******************************************************控制面板页面*****************************************************************************************
		{	
			//Get_Pulses_num();                                   //计算脉冲个数 
		  //LCD_Show_coordanate_value();		                    //串口屏显示工件和机械坐标
			
			//SetTextValue(2,27,(uchar *)file_name);            //显示正在加载的文件名	         				
			//sprintf(Working_line_buf,"%d",Pulses_counter);  
			//SetTextValue(2,28,(uchar *)Working_line_buf);     //显示加工行数，需要向主机询问	
			
		}break;
		case Return_WorkPiece_Zero_Page:   //*******************************************回工件零页面***************************************************************************************
		{ 
			
			if(first_time_re_workpiece)      //首次进入这个页面
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
				SetButtonValue(3,1,0);              //全轴按钮显示选中状态
			}
			else
			{
				SetButtonValue(3,1,1);              //全轴按钮显示未选中状态
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
		{
		  char buf[20];
		  if(Press_button==CMD_Download)   //下载按钮触发
			{
				Start_Download=1;               //下载标志位置1
				SetButtonValue(6,1,1);    //设值按钮为下载状态
			  SetControState(6,1,0);		 //禁止控件	
				Press_button=0xFF;
			}		
			if(Start_Download)
			{
			   sprintf(buf,"已下载%d%%",Download_Per);
				 SetTextValue(6,38,(uchar *)buf);     //显示下载百分比
				 if(Download_Per==100)
				 {
					 Download_Per=0;
					 Start_Download=0;
					 ClearTextValue(6,38);     //清除文本
					 SetControState(6,1,1);		 //使能控件	
					 SetButtonValue(6,1,0);    //设置值按钮位初始状态
				 }
			}
			if(Press_button==CMD_Cancel_Download)  //发取消下载按钮
			{
				Start_Download=0;
				Download_Per=0;
				ClearTextValue(6,38);     //清除文本
				SetControState(6,1,1);		 //使能控件	
				SetButtonValue(6,1,0);    //设置按钮为初始状态
				Press_button=0xFF;
			
			}
			if(Last_time_Cloud_Page!=Cloud_Page)
			{
			  sprintf(buf,"%u",Cloud_Page);
				SetTextValue(6,20,(uchar *)buf);     //显示第几页
				Last_time_Cloud_Page=Cloud_Page;
			}
			if(Last_time_SD_Page!=SD_Page)
			{
			  sprintf(buf,"%u",SD_Page);
				SetTextValue(6,21,(uchar *)buf);     //显示第几页
				Last_time_SD_Page=SD_Page;
			}
			if(Press_button==CMD_Delete)
			{
			  //sprintf(buf,"%u",SD_Page);
			  SetTextValue(9,3,(uchar *)file_name);     //显示文件名呢
				Press_button=0xFF;
			}
			if(Press_button==CMD_File_Delete_Sure)
			{
			
			}
		
		}
			break;
		case Same_file_Error_Page:    //**************************************************同名文件报错页面****************************************************************************************
			break;
		case Download_Fail_Pgae:    //***************************************************下载失败提醒页面
			break;
		case Delete_Page:         //*****************************************************删除页面****************************************************************************************
		{		}	
		break;
		case Storage_View_Page:    //*****************************************************内存预览页面****************************************************************************************
			break;	
		case Net_Account_Manage_Page:  //**************************************************网络与账户登录管理**********************************************************************************
			{
				if(WiFi_Password_Right==5)   //WiFi账号和密码正确
				{
				  Network_Control=3;
					WiFi_Password_Right=0;
				}
				if(Account_Password_Right==5) //用户账户和密码正确
				{
					Sign_Control=3;
					Account_Password_Right=0;
				}
				switch(Network_Control)     //网络连接控件显示、隐藏控制
				{
					case 0: break;
					case Show_Disconnect_Network_Control: 
					{
						ShowControl(11,18,1);
						Network_Control=0;
					}break;
					case Hide_Disconnect_Network_Control: 
					{
						ShowControl(11,18,0);
						Network_Control=0;
					}break;				
				}
				//ShowControl(11,4,1);
				switch(Sign_Control)     //账户登录退出控件显示、隐藏控制
				{
					case 0: break;
					case Show_Sign_Out_Control: 
					{
						ShowControl(11,19,1);
						Network_Control=0;
					}break;
					case Hide_Sign_Out_Control: 
					{
						ShowControl(11,19,0);
						Network_Control=0;
					}break;					
				}									
			}
			break;
		case Choose_WiFi_Page:        //*****************************************************选择WiFi页面****************************************************************************************
		{
		  char buf[20];
			if(Last_time_WiFi_Page!=WiFi_Page)
			{
			  sprintf(buf,"%u",WiFi_Page);
				SetTextValue(12,11,(uchar *)buf);     //显示第几页
				Last_time_WiFi_Page=WiFi_Page;
			}	
		}break;
		case Disconnect_Remind_Page: 
			break;
		case SignOut_Remind_Page: 
			break;
		case Cancel_Download_Pgae:
			break;
		default:
			break;
	}

}
	

//主机工作状态显示
void Work_state_control(void)
{
  if(last_time_work_state!=state.Work_state)     //判断工作状态是否发生变化，发生变化则进行处理
	{
		if(state.Work_state==Start)         
		{
			Show_Start_Working(Work_Page_Status);      //串口屏显示当前处于加工状态
		}
		else                                 
		{
			Show_Stop_Working(Work_Page_Status);        //串口屏显示当前已经停止加工
		}
		last_time_work_state=state.Work_state;
	}

}

/****************************************************
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
******************************************************/
void Usart1_Receive_data_handle( PCTRL_MSG msg, uint16 size )
{
    get_cmd_type = msg->cmd_type;                                         //获取指令类型
    get_ctrl_msg = msg->ctrl_msg;                                         //获取消息的类型	  
    get_control_type = msg->control_type;                                 //获取控件类型
    get_screen_id = PTR2U16(&msg->screen_id);                             //获取画面ID
    get_control_id = PTR2U16(&msg->control_id);                           //获取控件ID
    get_value = PTR2U32(msg->param);                                      //获取数值
	
	  if(get_control_type==kCtrlButton)                                      //按钮控件
			get_button_state= msg->param[1];                                     //获取按钮状态
   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //控件更新通知
    {  
				switch(get_screen_id)                                             //画面ID
				{
					case Working_Page:                                              //画面0：加工页面
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
													Speed.Initial_Work_Speed_Percent-=25;						
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
										control_panel_pram.Axis_press= CMD_X_AXIS;								
									}break;
									case 43:                                           //Y轴按钮触发
									{
																				
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 44:                                              //Z轴按钮触发
									{
																			
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 45:                                             //A轴按钮触发
									{
																				
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 46:                                           //B轴按钮触发
									{
																			
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;								
									default:break;
						  }
					}break;
					case Setting_page:                                              //画面1：设置页面
					{
						char buf[20];
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
										
										pram_status.Safe_Z_num=NotifyText(msg->param);										
//										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);    
									}										
									break;
									case 7:                   //获取对刀高度
									{
										
										pram_status.Knife_high_num=NotifyText(msg->param);										
//										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8:                     //获取对刀块高度
									{
										
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9:                  //获取对刀块X轴位置
									{
										
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10:                 //获取对刀块Y轴位置
									{
										
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11:                 //获取对刀块Z 轴位置
									{
										
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12:                  //获取软限位X轴位置
									{
										
										pram_status.Soft_limit_X=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13:                   //获取软限位Y轴位置
									{
										
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14:                 //获取软限位Z轴位置
									{
										
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15:                 //获取软限位A轴位置
									{ 
										
										pram_status.Soft_limit_A=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16:                  //获取软限位B轴位置
									{
										
										pram_status.Soft_limit_B=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
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
					case ControlPanel_Page:                                         //画面2：控制面板
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //清零按钮触发
									{
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Clear;								
										}																					
									}break;    
                  case 2:                                             //回机械零按钮触发	
                  {
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Return_Machine_Zero;
																								
										}
										get_button_state=0;	
									}break;                                     									
									case 3:								                              //倍率切换按钮触发
									{
										if(get_button_state)
										{
												control_panel_pram.Override_Change_button=1;
												Override_Change_Process();                    //倍率切换控制
												control_panel_pram.Press_button = CMD_Override_Change;												
										}
										get_button_state=0;										
									}break;
								  case 4:                                             //主轴开关按钮触发
									{
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Spin_On_Off;						
										}
										get_button_state=0;	
									}break;
			            case 5:                                              //全轴清零按钮触发
									{
                    if(get_button_state)   
										{										
											control_panel_pram.All_Spindle_Clear_Button=1;
											control_panel_pram.Press_button = CMD_All_Spin_Clear;
											get_button_state=0;
										}
									}break;
									case 6:                                            //回工件零按钮触发
									{
										if(get_button_state)    
										{
											Work_Page_Status=Return_WorkPiece_Zero_Page;
											first_time_re_workpiece=1;
											control_panel_pram.Press_button = CMD_Return_WorkPiece_Zero;
											get_button_state=0;
										}
									}break;
									case 7:                                            //坐标切换按钮触发
									{
											if(get_button_state)
											{										
												  Coordinate_Change_Process();//坐标切换控制
												  control_panel_pram.Press_button = CMD_Coordinate_Change;
												  get_button_state=0;
											 }										
									}break;
									case 8:                                            //软限位开关按钮触发
									{											
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Soft_Limit;						
											}
											get_button_state=0;	
									}break;
									case 9:                                            // 安全Z开关
									{
										if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Safe_Z;						
											}
											get_button_state=0;	
									}break;
									case 10:                                         //跳行加工按钮触发
									{
										if(get_button_state)
										{
											jump_work_set.First_get_into = 1;
											Work_Page_Status=Jump_Work_Page;
											control_panel_pram.Press_button = CMD_Jump_Work;
											get_button_state=0;
										}
									}break;
									case 11:                                          //对刀按钮触发
									{										
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Auto_knife;								
											}
											get_button_state=0;	
									}break;
									case 12:                                         //分中按钮触发
									{
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Divided;								
											}
											get_button_state=0;	
									}break;
									case 13:                                           //开始按钮触发
									{
										state.Work_state=Start;
										control_panel_pram.Press_button = CMD_Start;
										TIM_Cmd(TIM4, DISABLE);      //禁止 TIM4
//										SetButtonValue(2,14,0);     //停止按钮松开状态
										SetButtonValue(2,13,1);       //开始按钮按下状态										
									}break;
									case 14:                                           //停止按钮触发
									{
										state.Work_state=Stop;
//										SetButtonValue(2,13,0);     //开始按钮松开状态
										SetButtonValue(2,14,1);     //停止按钮按下状态
                    control_panel_pram.Press_button = CMD_Stop;
									  TIM_Cmd(TIM4, ENABLE);      //使能 TIM4
																	
									}break;
									case 15:                                            //复位按钮触发
									{
										state.Work_state=Stop;
										control_panel_pram.Press_button = CMD_Reset;  						
									}break;
									case 16:                                          //退出控制面板按钮触发
									{
										Work_Page_Status=Working_Page;
									}
									break;
									case 22:                                            //X轴按钮触发
									{	
																											
										control_panel_pram.Axis_press= CMD_X_AXIS;								
									}break;
									case 23:                                            //Y轴按钮触发
									{
																		
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 24:                                              //Z轴按钮触发
									{
															
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 25:                                             //A轴按钮触发
									{
																	
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 26:                                           //B轴按钮触发
									{  
																	
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;
									default:break;				
							}
					}break;
					case Return_WorkPiece_Zero_Page:                                //画面3：回工件零页面
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
					case Jump_Work_Page:                                            //画面5:跳行加工页面
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
					case File_Manage_Page:                                           //画面6:文件管理
					{
						Work_Page_Status=File_Manage_Page;
						switch(get_control_id)
						{
							case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_Download;								
								}
							}
							break;
							case 2:  //取消下载按钮触发
							{
							  
							}
							break;
							case 3: 
								{
							  if(get_button_state)
								{
									Press_button = CMD_Delete;								
								}
							}
							break;
							case 4:
              {
								Work_Page_Status=Storage_View_Page;
							  if(get_button_state)
								{
									Press_button = CMD_Storage_View;								
								}
							}								
							break;
							case 5: Work_Page_Status=ControlPanel_Page;break;
							case 6: 
							{
							  if(get_button_state)
								{
									if(Cloud_Page>1)
								    Cloud_Page--;
								}
							}	
							break;
							case 7: 
							{
							  if(get_button_state)
								{
								  Cloud_Page++;
								}
							}
								break;
							case 8: 
							{
							  if(get_button_state)
								{
								  if(SD_Page>1)
								    SD_Page--;
								}
							}
								break;
							case 9: 
							{
							  if(get_button_state)
								{
								  SD_Page++;
								}
							}
								break;
							case 10:Cloud_File_Button=0X01;break;
							case 11:Cloud_File_Button=0X02;break;
							case 12:Cloud_File_Button=0X03;break;
							case 13:Cloud_File_Button=0X04;break;
							case 14:SD_File_Button=0X01;break;
							case 15:SD_File_Button=0X02;break;
							case 16:SD_File_Button=0X03;break;
							case 17:SD_File_Button=0X04;break;
							
							case 18:Work_Page_Status=Working_Page;break;
							case 19:Work_Page_Status=Net_Account_Manage_Page;break;
							default:break;
						}
					}break;
					case Same_file_Error_Page: Work_Page_Status=Same_file_Error_Page;break;//画面7：同名文件报错页面
					case Download_Fail_Pgae: Work_Page_Status=Download_Fail_Pgae;break;    //画面8：下载失败页面
					case Delete_Page: Work_Page_Status=Delete_Page;                    //画面9：删除页面
					{
						switch(get_control_id)
						{
					    case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_File_Delete_Sure;								
								}
							}
						}
					}
					break;
					case Storage_View_Page: Work_Page_Status=Storage_View_Page;break;   //画面10：内存空间预览页面
					case Net_Account_Manage_Page:                                       //画面11：网络账户管理
					{
						Work_Page_Status=Net_Account_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Working_Page;break;
							case 2: Work_Page_Status=File_Manage_Page;break;
							case 3: Work_Page_Status=Choose_WiFi_Page;break;
							case 4: 
							{
							  if(get_button_state)
								{
									WiFi_Password_Right++;								
								}
							
							}break;
							case 5:
							{
							  if(get_button_state)
								{
									Account_Password_Right++;								
								}
							
							}
							default:break;
						}
					}break;
					case Choose_WiFi_Page:                                                 //画面12：选择WiFi
   				{
						Work_Page_Status=Choose_WiFi_Page;
						switch(get_control_id)
						{
							case 1: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 2: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 3: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 4: 
							{
								Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 5:
							{
							  if(get_button_state)
								{
								    WiFi_Page++;
								}
							}break;
							case 6: 
							{
								if(get_button_state)
								{
									if(WiFi_Page>1)
								    WiFi_Page--;
								}
							}break;
							default:break;
						}
					}break;          
					case Disconnect_Remind_Page:                                            //画面15：断开网络提醒页面
					{
						switch(get_control_id)
						{
							case 1: Network_Control=4;
						}
					}break;     
					case SignOut_Remind_Page:                                                //画面16：退出登录提醒页面
					{
					  switch(get_control_id)
						{
							case 1: Sign_Control=4;
						}
					}	
					break;         
					case Cancel_Download_Pgae:                                                //取消下载提醒
					{
            switch(get_control_id)
						{
							case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_Cancel_Download;								
								}
							}
							break;
							case 2: 
							{
							  if(get_button_state)
								{
									Press_button = 0xFF;								
								}
							}
							break;
					  }
					}break;
					case Save_Pram_Page:                                                    //画面21 保存参数设置提醒页面
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

