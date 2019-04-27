/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   ulitity.c
** 修改时间:   2018-05-18
** 文件说明:   用户MCU串口驱动函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------*/
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "ulitity.h"
#include "string.h"


#define TIME_100MS 10                                                 //100毫秒(10个单位)
#define voice_on   0
#define voice_off  1

uint8 get_cmd_type;                      //获取指令类型
uint8 get_ctrl_msg;                      //获取消息的类型
uint8 get_button_state;                  //获取按钮状态
uint8 get_control_type;                  //获取控件类型
uint16 get_screen_id;                    //获取画面ID
uint16 get_control_id;                   //获取控件ID
uint32 get_value;                        //获取数值
uint8  input_buf[20];                    //键盘输入内容

uint8 Voice_Control=0;                   //语音控制（保存之前的控制状态），0：使能，1：禁止
uint8 voice_button;                      //语音按钮
uint8 Setting_Sure;                      //设置页面确定按钮状态
uint8 Setting_concel;                    //设置页面取消按钮状态

volatile uint32  timer_tick_count = 0;                                //定时器节拍

uint8  cmd_buffer[CMD_MAX_SIZE];                                      //指令缓存
static uint16 current_screen_id = 0;                                  //当前画面ID
static uint8 Work_state=0;                                            //机器工作状态标记
static uint8 WiFi_state=0;                                            //WiFI状态标记
static uint8 BT_state=0;                                              //蓝牙状态标记
static uint8 Account_state=0;                                         //账户登入状态标记
uint8 Work_Page_Status = 0;                                           //工作页面标记
uint8 Coordinate_Change_Counter=0;                                    //坐标切换按钮触发计数
uint8 Override_Change_Counter=0;                                      //倍率切换按钮触发计数
uint32 Progress_Value = 0;                                            //进度条的值 

float  X1=0,X2=0;                                                     //X轴坐标，X1:工件坐标，X2:机械坐标
float  Y1=0,Y2=0;
float  Z1=0,Z2=0;
float  A1=111.1,A2=111.1;
float  B1=111.1,B2=111.1;
uint8 X1buf[9],X2buf[8],Y1buf[9],Y2buf[9],Z1buf[9],Z2buf[9],A1buf[9],A2buf[9],B1buf[9],B2buf[9];
uint16 Spindle_Speed=20000;                                            //主轴最大转速
int Spindle_Speed_Value=100;                                           //主轴初始速度百分比
int Work_Speed=1000;                                                     //加工初始速度
int Work_Speed_Value=100;                                              //加工初始速度百分比



/*! 
*  \brief  加工页面坐标数据更新
*/ 
void UpdateWorkPageTextWord(void)
{
	//加工页面工件坐标
	SetTextValue(0,16,X1buf);
	SetTextValue(0,17,Y1buf);
	SetTextValue(0,18,Z1buf);
	SetTextValue(0,19,A1buf);
	SetTextValue(0,20,B1buf);
	//加工页面机械坐标
	SetTextValue(0,31,X2buf);
	SetTextValue(0,35,Y2buf);
	SetTextValue(0,36,Z2buf);
	SetTextValue(0,37,A2buf);
	SetTextValue(0,38,B2buf);	
	}

	//控制面板坐标数据更新
void UpdateControlPanelTextWord(void)
{	
	//控制面板工件坐标
	SetTextValue(2,38,X1buf);
	SetTextValue(2,39,Y1buf);
	SetTextValue(2,40,Z1buf);
	SetTextValue(2,41,A1buf);
	SetTextValue(2,42,B1buf);
	//控制面板工件坐标
	SetTextValue(2,17,X2buf);
	SetTextValue(2,18,Y2buf);
	SetTextValue(2,19,Z2buf);
	SetTextValue(2,20,A2buf);
	SetTextValue(2,21,B2buf); 			                                                                                                                                                          
}

//加工中心主轴速度和加工速度按钮处理
void Spindle_and_Work_Speed_Key_Process(uint8 control_id )
{
	char buf[6];
	uint32 Speed1=0;
	int16 Speed2=0;
	if(control_id==3)                  //主轴速度减
	{
			Spindle_Speed_Value -= 10; 
			if(Spindle_Speed_Value <= 0)
			{
					Spindle_Speed_Value = 0;
			}
      Speed1=Spindle_Speed*Spindle_Speed_Value/100;
			sprintf(buf,"%d",Speed1);
		  SetTextValue(0,24,(uchar *)buf);                //修改文本框数值
      //SetTextInt32(0,24,Speed1,0,5);		               
			SetProgressValue(0,1,Spindle_Speed_Value);      //修改进度条数值
	}
	if(control_id==4)                 //主轴速度加
	{
			Spindle_Speed_Value += 10;
			if(Spindle_Speed_Value >= 100)
			{
					Spindle_Speed_Value = 100;
			}
			Speed1=Spindle_Speed*Spindle_Speed_Value/100;
			sprintf(buf,"%d",Speed1);
		  SetTextValue(0,24,(uchar *)buf);              //修改文本框数值
      //SetTextInt32(0,24,Speed1,0,5);
			SetProgressValue(0,1,Spindle_Speed_Value);    //修改进度条数值
	}
	if(control_id==5)                //加工速度减
	{
		if(Work_Speed_Value <= 100)
		{
			Work_Speed_Value -=10;
			if(Work_Speed_Value <= 0)
			{
				Work_Speed_Value=0;
			}
			SetTextInt32(0,39,Work_Speed_Value,0,2);          //修改文本框数值
		}
		else if(100 < Work_Speed_Value && Work_Speed_Value <=300)
		{
			Work_Speed_Value  -=25;	
			SetTextInt32(0,39,Work_Speed_Value,0,3);         //修改文本框数值						
		}
		Speed2=Work_Speed*Work_Speed_Value/100;
    sprintf(buf,"%d",Speed2);
		SetTextValue(0,25,(uchar *)buf);                    //修改文本框数值
		SetProgressValue(0,2,Work_Speed_Value);            //修改进度条数值
		
	}
	if(control_id==6)               //加工速度加
	{
		if(Work_Speed_Value < 100)
		{
			Work_Speed_Value +=10;
			SetTextInt32(0,39,Work_Speed_Value,0,2);          //修改文本框数值
		}
		else if(100 <= Work_Speed_Value && Work_Speed_Value <=300)
		{
			Work_Speed_Value +=25;
			if(Work_Speed_Value >= 300)
			{
				Work_Speed_Value=300;
			}	
			SetTextInt32(0,39,Work_Speed_Value,0,3);          //修改文本框数值						
		}
		Speed2=Work_Speed*Work_Speed_Value/100;
    sprintf(buf,"%d",Speed2);
		SetTextValue(0,25,(uchar *)buf);                    //修改文本框数值
		SetProgressValue(0,2,Work_Speed_Value);             //修改进度条数值
	}
}

//设置页面语音提示按钮处理
void Speaker_Key_Process(uint8  Button_state)
{
	if(Button_state==voice_off)        //喇叭音量调到最大：EE 93 64 FF FC FF FF 
	{
		 SpeakerControl(0);
		 Voice_Control=voice_off;
	}
	else                               //喇叭音量调到0；EE 93 00 FF FC FF FF 
	{
		 SpeakerControl(100);
     Voice_Control=voice_on;	
	}
}
//控制面板坐标切换控制
void Coordinate_Change_Process(void)
{
	Coordinate_Change_Counter++;
	switch(Coordinate_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,27,"G54");        //设置文本值
			SetTextValue(2,30,"G54");       
			break;  
		}
		case 2:
		{
			SetTextValue(0,27,"G55");
			SetTextValue(2,30,"G55");
			break;
		}
		case 3:
		{
			SetTextValue(0,27,"G56");
			SetTextValue(2,30,"G56");
			break;
		}
		case 4:
		{
			SetTextValue(0,27,"G57");
			SetTextValue(2,30,"G57");
			break;
		}
		case 5:
		{
			SetTextValue(0,27,"G58");
			SetTextValue(2,30,"G58");
			break;
		}
		case 6:
		{
			SetTextValue(0,27,"G59");
			SetTextValue(2,30,"G59");
			Coordinate_Change_Counter=0;
			break;
		}	
	}
}
//控制面板倍率控制
void Override_Change_Process(void)
{
	Override_Change_Counter++;
	switch(Override_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,29,"X1");    //设置文本值
			SetTextValue(2,32,"X1");
			break;  
		}
		case 2:
		{
			SetTextValue(0,29,"X2");
			SetTextValue(2,32,"X2");
			break;
		}
		case 3:
		{
			SetTextValue(0,29,"X3");
			SetTextValue(2,32,"X3");
			break;
		}
		case 4:
		{
			SetTextValue(0,29,"X5");
			SetTextValue(2,32,"X5");
			break;
		}
		case 5:
		{
			SetTextValue(0,29,"X10");
			SetTextValue(2,32,"X10");
			break;
		}
		case 6:
		{
			SetTextValue(0,29,"X20");
			SetTextValue(2,32,"X20");
			Override_Change_Counter=0;
			break;
		}			
	}		
}

//显示机器已经停止加工
void WorkingStatus_Stoped(void)
{
	AnimationPlayFrame(0,11,0);
	AnimationPlayFrame(1,28,0);
	AnimationPlayFrame(2,43,0);
	AnimationPlayFrame(6,33,0);
	AnimationPlayFrame(10,5,0);
	AnimationPlayFrame(11,13,0);
	AnimationPlayFrame(13,9,0);
	AnimationPlayFrame(14,12,0);
}
//显示机器正在加工
void WorkingStatus_Starting(void)
{
	AnimationPlayFrame(0,11,1);
	AnimationPlayFrame(1,28,1);
	AnimationPlayFrame(2,43,1);
	AnimationPlayFrame(6,33,1);
	AnimationPlayFrame(10,5,1);
	AnimationPlayFrame(11,13,1);
	AnimationPlayFrame(13,9,1);
	AnimationPlayFrame(14,12,1);
	
}
//开机画面设置
void Power_On_Set(void)
{
	uint8 i=0;
	char buf[6];
	SetScreen(17);           //开机画面
	delay_ms(10); 
	SetScreen(17);           //开机画面
	delay_ms(10);	
	for(i=0;i<101;i++)
	{
		SetProgressValue(17,3,i);      //设置进度条的值
		delay_ms(20);                  //延时0.05秒
	}
	
	SetScreen(0);           //切换到加工页面
	delay_ms(10);
	
	//开机后，加工页面相关设置
	
	for(i=16;i<21;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //工件坐标设置文本值
	}
	for(i=34;i<39;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //	机械坐标设置文本值
	}
	
	SetTextValue(0,27,"G59");        //设置当前坐标值：G54
	SetTextValue(2,30,"G59");
	SetTextValue(0,28,"关");         //设置手轮引导“关”
	SetTextValue(2,31,"关");
	SetTextValue(0,29,"X0");         //设置倍率：X1
	SetTextValue(2,32,"X0");
	SetProgressValue(0,1,100);        //设置主轴初始化速度：100%
	sprintf(buf,"%d",Spindle_Speed);
	SetTextValue(0,24,(uchar *)buf);
	
	SetProgressValue(0,2,100);	      //设置加工初始化速度：100%	
	SetTextInt32(0,39,100,0,3);       //设置加工初始化速度文本值：100
	SetTextInt32(0,25,1000,0,2);      //显示加工初始化速度为100%时的速度
	
	SetButtonValue(2,13,0);          //开始按钮是松开状态
	SetButtonValue(2,14,1);          //停止按钮是按下状态

}
/******************************************************************************************************/
/*!                                                                                 
*  \brief  程序入口                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
{  
	qsize  size = 0;        //指令长度 
	
	Set_System();           //配置时钟                                                                                                                                 
	Interrupts_Config();    //配置串口中断                                                                                                                             
	systicket_init();       //配置时钟节拍                                                                                                          
	UartInit(115200);       //串口初始化                                                                                                                      
	queue_reset();          //清空串口接收缓冲区                                                                                                
	delay_ms(300);          //延时等待串口屏初始化完毕,必须等待300ms  
 
	Power_On_Set();         //开机画面设置
	Work_Page_Status=Working_Page;
	
	while(1)                                                                        
	{   			
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);                  //从缓冲区中获取一条指令，得到指令长度       

		if(size>0&&cmd_buffer[1]!=0x07)                                  //接收到指令 ，及判断是否为开机提示
		{                                                                           
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);                   //指令分析处理 ，判断主程序进入哪个Work_Page_Status
		}                                                                           
		else if(size>0&&cmd_buffer[1]==0x07)                             //如果为指令0x07就软重置STM32  
		{                                                                           
			__disable_fault_irq();                                                   
			NVIC_SystemReset();                                                                                                                                          
		}
	 
		
		switch(Work_Page_Status)
		{
			//********加工页面********
			case Working_Page:       
			{
				switch(get_control_type)                                     //控件类型判断
				{
					case kCtrlButton:                                          //按钮控件,kCtrlButton=0X10
					{
						if(get_button_state)
						{
							Spindle_and_Work_Speed_Key_Process(get_control_id);
							get_button_state=0;							
						}							                     
					}break;						
					case kCtrlText:                                           //文本控件
					{
					
					}	break;                        							                                                                                                                                                                                  
					default:						
							 break;
				}			
			}break;
			//********设置页面********
		  case Setting_page:  
			{
				switch(get_control_type)                                    //控件类型判断
				{
					case kCtrlButton:                                         //按钮控件
					{
						if(get_control_id==27)                                //语音提示按钮触发     
						{
							if(get_button_state)                                
								voice_button=voice_off;                           //按钮按下，语音提醒关
							else
								voice_button=voice_on;                            //按钮松开，语音提醒开													
						}
						if(get_control_id==4)                                //确定按钮按下
						{
												
						}
						if(get_control_id==5)                                //取消按钮按下
						{
							if(Voice_Control==voice_off)
								SetButtonValue(1,27,1);
							else 
								SetButtonValue(1,27,0);							
						}
						
					}break;						
					case kCtrlText:                                                       //文本控件
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				 }
			
			}
			break;
			//********提示是否保存参数设置*********
			case Save_Pram_Page:      
			{
				switch(get_control_type)                              //控件类型判断
				{
					case kCtrlButton:                                   //按钮控件
					{
						if(get_control_id==1)                           //确定按钮按下
						{
							Speaker_Key_Process(voice_button);            //语音提示按钮处理						
						}
						if(get_control_id==2)                           //取消按钮按下
						{
							if(Voice_Control==voice_off)
								SetButtonValue(1,27,1);
							else 
								SetButtonValue(1,27,0);						
						}
					}
					case kCtrlText:                                                       //文本控件
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				}	
			}break;
			//********控制面板页面***********
			case ControlPanel_Page: 
			{
				switch(get_control_type)                              //控件类型判断
				{
					case kCtrlButton:                                   //按钮控件
					{							
						if(get_button_state)                            //按钮按下
						{
							if(get_control_id==7)                         //坐标切换按钮触发
							{			
								Coordinate_Change_Process();		            //坐标切换控制		 
							}
							if(get_control_id==3)                         //倍率切换按钮触发
							{
								Override_Change_Process();                  //倍率切换控制			
							} 
														
						}	
						switch(get_control_id)
						{
							case 13:                        //开始按钮触发
							{
								Work_state=1;
								SetButtonValue(2,14,0);                    //停止按钮松开状态
								if(get_button_state==0)
								{
									delay_ms(200);
									SetButtonValue(2,13,1);                    //开始按钮按下状态
								}							
								
							}break;
							case 14:                       //停止按钮触发
							{
								Work_state=0;
								SetButtonValue(2,13,0);                    //开始按钮松开状态
								if(get_button_state==0)
								{
									delay_ms(200);
									SetButtonValue(2,14,1);                    //停止按钮按下状态
								}								
								
							}break;
							case 15:                       //复位按钮触发
							{
								Work_state=0;
								SetButtonValue(2,13,0);                    //开始按钮松开状态
								SetButtonValue(2,14,1);                    //停止按钮按下状态						
							}break;
					 }
					}break;						
					case kCtrlText:                                                       //文本控件
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				}
				
			}break;
			//
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
			//********网络与账户登录管理**********
			case Net_Account_Manage_Page: 
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
								// NotifyText(get_screen_id,get_control_id,get_value);                       
								                                                                                                                                                                                  
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

		
		
		if(Work_state)               //加工状态：开始
		{
			WorkingStatus_Starting();       
		}
		else                         //加工状态：停止
		{
			WorkingStatus_Stoped();  
		}
		
		//    特别注意
		//    MCU不要频繁向串口屏发送数据，否则串口屏的内部缓存区会满，从而导致数据丢失(缓冲区大小：标准型8K，基本型4.7K)
		//    1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
		//    2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。

		//这里为了演示方便，每20毫秒自动更新一次
		//进一次中断10ms，timer_tick_count值+1,100*timer_tick_count = 1s				
	}  
}




/***
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
//	  uint8 *Just_get_value;
	
	  char input_string;
    get_cmd_type = msg->cmd_type;                                         //获取指令类型
    get_ctrl_msg = msg->ctrl_msg;                                         //获取消息的类型
	  get_button_state= msg->param[1];                                       //获取按钮状态
    get_control_type = msg->control_type;                                 //获取控件类型
    get_screen_id = PTR2U16(&msg->screen_id);                             //获取画面ID
    get_control_id = PTR2U16(&msg->control_id);                           //获取控件ID
    //get_value = PTR2U32(msg->param);                                      //获取数值
//	  Just_get_value = msg->param; 	  
//	  SetTextValue(11,6,Just_get_value);

	  sprintf(input_buf,"%s",msg->param);
	   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //控件更新通知
    {  
//			if(get_ctrl_msg==MSG_GET_CURRENT_SCREEN)                            //画面ID变化通知,在这里切换Work_Page_Status
//			{
				switch(get_screen_id)                                             //画面切换
				{
					case 0: Work_Page_Status=Working_Page;break;
					case 1: Work_Page_Status=Setting_page;break;
					case 2: Work_Page_Status=ControlPanel_Page;break;
					case 3: Work_Page_Status=Return_WorkPiece_Zero_Page;break;
					case 5: Work_Page_Status=Jump_Work_Page;break;
					case 6: Work_Page_Status=File_Manage_Page;break;
					case 7: Work_Page_Status=Leading_In_Page;break;
					case 9: Work_Page_Status=Delete_Page;break;
					case 10: Work_Page_Status=Storage_View_Page;break;
					case 11: Work_Page_Status=Net_Account_Manage_Page;break;
					case 12: Work_Page_Status=Choose_WiFi_Page;break;
					case 13: Work_Page_Status=Disconnet_and_SignIn_Page;break;
					case 14: Work_Page_Status=Disconnect_and_SignOut_Page;break;
					case 15: Work_Page_Status=Disconnect_Remind_Page;break;
					case 16: Work_Page_Status=SignOut_Remind_Page;break;
					case 20: Work_Page_Status=Leading_Out_Pgae;break;
					case 21: Work_Page_Status=Save_Pram_Page;break;
					default: break;						
  			}
        
     }
}


/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param str 文本控件内容
*/
void NotifyText(uint16 screen_id, uint16 control_id, uint32 str)
{
    if(screen_id==11)                                                                 //画面ID2：文本设置和显示
    {                                                                            
        int32 value=0;                                                            
        //sscanf(str,"%ld",&value);                                                    //把字符串转换为整数 
			  value=str;
        if(control_id==7)                                                            //最高电压
        {                                                                         
//            //限定数值范围（也可以在文本控件属性中设置）                             
//            if(value<0)                                                              
//            {                                                                        
//                value = 0;                                                            
//            }                                                                        
//            else if(value>380)                                                       
//            {                                                                        
//                value = 380;                                                           
//            }                                                                        
            SetTextInt32(11,8,value,0,4);                                             //更新最高电压
            //SetTextInt32(4,5,value/2,1,1);                                           //更新最高电压/2
        }                                                                         
    }                                                                            
}                                                                                

