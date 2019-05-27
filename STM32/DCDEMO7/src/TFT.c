#include "tft.h"
#include "hmi_user_uart.h"
#include "stdio.h"
#include "hmi_driver.h"
#include "flash.h"
#include "ulitity.h"
#include "cJSON.h"
#include "stdlib.h"
#include "timer.h"


extern uint8  Work_Page_Status;
extern int32 Pulses_counter;              // 手轮脉冲数量
extern Override override;
extern Speed_Control Speed;
extern Pram_Status pram_status;
extern Control_Panel_Pram control_panel_pram;
extern Return_Workpiece_Zero return_workpiece_zero;
extern Devide_Set devide_set;  

uint8 Coordinate_Change_Counter=0;                                    //坐标切换按钮触发计数
uint8 Override_Change_Counter=1;                                      //倍率切换按钮触发计数

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

//对X轴工件坐标清零
void X_coordinate_clear(void)
{
  SetTextValue(2,17,"000000.00");
	SetTextValue(0,16,"000000.00");
}

//对Y轴工件坐标清零
void Y_coordinate_clear(void)
{
	SetTextValue(2,18,"000000.00");
	SetTextValue(0,17,"000000.00");
  
}

//对Z轴工件坐标清零
void Z_coordinate_clear(void)
{
  SetTextValue(2,19,"000000.00");
	SetTextValue(0,18,"000000.00");
}

//对A轴工件坐标清零
void A_coordinate_clear(void)
{
  SetTextValue(2,20,"000000.00");
	SetTextValue(0,19,"000000.00");
}

//对B轴工件坐标清零
void B_coordinate_clear(void)
{
  SetTextValue(2,21,"000000.00");
	SetTextValue(0,20,"000000.00");
}

//对所有轴工件坐标清零
void All_Workpiece_coordinate_clear(void)
{
	uint8 i=0;
  for(i=16;i<21;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //工件坐标设置文本值
	}
	for(i=17;i<22;i++)
	{
		SetTextValue(2,i,"000000.00"); 	   //工件坐标设置文本值
	}
}

//XYZAB坐标按钮复位
void XYZAB_button_reset(void)
{
	uint8 i;
	for(i=22;i<27;i++)
	{
		SetButtonValue(2,i,0);           
	}
}

//开机画面设置
void Power_On_Set(void)
{
	uint8 i=0;
	char buf[6];
	SetScreen(17);           //切换到开机画面
	delay_ms(10); 
	SetScreen(17);           //切换到开机画面
	delay_ms(10);	
	for(i=0;i<101;i++)
	{
		SetProgressValue(17,3,i);     //设置进度条的值
		delay_ms(5);                  //延时0.05秒
	}
	
	SetScreen(0);           //切换到加工页面
	delay_ms(10);
	
  All_Workpiece_coordinate_clear();//对所有工件坐标清零
	
	for(i=34;i<39;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //	机械坐标设置文本值
		delay_ms(5);
	}
	for(i=38;i<43;i++)
	{
		SetTextValue(2,i,"000000.00"); 	   //工件坐标设置文本值
		delay_ms(5);
	}
	for(i=22;i<27;i++)
	{
		SetButtonValue(2,i,0);            //XYZAB轴复位
		delay_ms(5);
	}
	
	SetTextValue(0,27,"G54");        //设置当前坐标值：G54
	SetTextValue(2,30,"G54");
	SetTextValue(0,28,"关");         //设置手轮引导“关”
	SetTextValue(2,31,"关");
	SetTextValue(0,29,"X1");         //设置倍率：X1
	SetTextValue(2,32,"X1");
	
	Speed.Initial_Spindle_Speed=20000;                     //主轴初始转速
	Speed.Initial_Spindle_Speed_Percent=100;               //主轴初始速度百分比
	Speed.Initial_Work_Speed=1000;                         //加工初始速度
	Speed.Initial_Work_Speed_Percent=100;                  //加工初始速度百分比
	Speed.Changed_Spindle_Speed=20000;                     //变化后的主轴转速              
	Speed.Changed_Work_Speed=1000;                         //变化后的加工速度
	
	SetProgressValue(0,1,Speed.Initial_Spindle_Speed_Percent);  //主轴速度进度条初始化：100%
	sprintf(buf,"%d",Speed.Initial_Spindle_Speed);
	SetTextValue(0,24,(uchar *)buf);                            //主轴速度文本框初始化：20000
	

	SetProgressValue(0,2,Speed.Initial_Work_Speed_Percent);	     //加工速度进度条初始化：100%
	sprintf(buf,"%d",Speed.Initial_Work_Speed_Percent);
	SetTextValue(0,39,(uchar *)buf);                             //加工速度进度条初始化：100%	      
	sprintf(buf,"%d",Speed.Initial_Work_Speed);
	SetTextValue(0,25,(uchar *)buf);                             //加工速度文本框初始化：100%
	   
	
	SetButtonValue(2,13,0);          //开始按钮是松开状态
	SetButtonValue(2,14,1);          //停止按钮是按下状态
	override.Override_num=0.01;       //开机默认倍率
	Work_Page_Status=Working_Page;    //开机进入加工页面
	


}

//设置页面几个参数值获取（断电保存在flash）
void Setting_page_pram_get(void)
{
	pram_status.Voice_last_status=FlashRead(START_ADDR1);
	pram_status.voice_button_status=pram_status.Voice_last_status;            //设置语音提醒模式
	pram_status.Safe_Z_last_status=FlashRead(START_ADDR2);
	pram_status.Safe_Z_button_satus=pram_status.Safe_Z_last_status;           //设置安全Z模式
	pram_status.Auto_Knife_last_status=FlashRead(START_ADDR3);
	pram_status.Auto_Knife_button_status=pram_status.Auto_Knife_last_status;  //设置自动对刀模式
	pram_status.Unit_Change_last_status=FlashRead(START_ADDR4);
	pram_status.Unit_Change_button_status=pram_status.Unit_Change_last_status; //设置单位模式
}

//加工中心主轴速度和加工速度按钮处理
void Spindle_and_Work_Speed_Key_Process(void)
{
	char buf[10];
  if(Speed.Spindle_Speed_Changed==Changed)
	{
		Speed.Spindle_Speed_Changed=Unchanged;
		sprintf(buf,"%d",Speed.Changed_Spindle_Speed);
		SetTextValue(0,24,(uchar *)buf);                                 //修改主轴速度文本框数值		               
		SetProgressValue(0,1,Speed.Initial_Spindle_Speed_Percent);       //修改主轴速度进度条数值
		
		sprintf(buf,"$xvm:%d",Speed.Changed_Spindle_Speed);              
		Usart_SendString(USART2,(char *)buf);                            //通过USART2向主机发送主轴速度
	}
	if(Speed.Work_Speed_Changed==Changed)
	{
		Speed.Work_Speed_Changed=Unchanged;
		sprintf(buf,"%d",Speed.Changed_Work_Speed);
		SetTextValue(0,25,(uchar *)buf);                                   //修改加工速度文本框数值
		SetProgressValue(0,2,Speed.Initial_Work_Speed_Percent);            //修改加工速度进度条数值	
		sprintf(buf,"%d",Speed.Initial_Work_Speed_Percent);
		SetTextValue(0,39,(uchar *)buf);
		
		sprintf(buf,"$xfr:%d",Speed.Changed_Work_Speed);
		Usart_SendString(USART2,(char *)buf);                             //通过USART2向主机发送加工速度           
		
	}
}

//设置页面语音提示按钮触发后处理程序
void Speaker_Key_Process(uint8  state)
{
	if(state==voice_off)        //喇叭音量调到0；EE 93 00 FF FC FF FF
	{
		 SpeakerControl(0);
		 SetButtonValue(1,27,1);
		 pram_status.Voice_last_status=voice_off;
	}
	else                        //喇叭音量调到最大：EE 93 64 FF FC FF FF 
	{
		 SpeakerControl(100);
		 SetButtonValue(1,27,0);
     pram_status.Voice_last_status=voice_on;	
	}
	FLASH_WriteByte(START_ADDR1,(uint16)pram_status.Voice_last_status);
}
//设置页面安全Z按钮触发后处理程序
void Safe_Z_process(uint8 state)
{
	if(state)                  //模式2（按钮按下状态）
	{
		SetButtonValue(1,1,1);
	  pram_status.Safe_Z_last_status=1;
		Usart_SendString(USART2,"Safe_z:mode2"); 
	}
	else
	{
		SetButtonValue(1,1,0);      //模式1
	  pram_status.Safe_Z_last_status=0;
		Usart_SendString(USART2,"Safe_z:mode1"); 
	}
	FLASH_WriteByte(START_ADDR2,(uint16)pram_status.Safe_Z_last_status);
}
//设置页面自动对刀按钮触发后处理程序
void Auto_Knife_process(uint8 state)
{
  if(state)                  //模式2（按钮按下状态）
	{
		SetButtonValue(1,2,1);
	  pram_status.Auto_Knife_last_status=1;
		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	else
	{
		SetButtonValue(1,2,0);      //模式1
	  pram_status.Auto_Knife_last_status=0;
		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	FLASH_WriteByte(START_ADDR3,(uint16)pram_status.Auto_Knife_last_status);
}
//设置页面单位切换按钮触发后处理程序
void Unit_Change_process(uint8 state)
{
	if(state )                  //模式2（按钮按下状态:英制）
	{
		SetButtonValue(1,3,1);
	  pram_status.Unit_Change_last_status=1;
		Usart_SendString(USART2,"Unit:mode2");
	}
	else
	{
		SetButtonValue(1,3,0);      //模式1
	  pram_status.Unit_Change_last_status=0;
		Usart_SendString(USART2,"Unit:mode1");
	}
  FLASH_WriteByte(START_ADDR4,(uint16)pram_status.Unit_Change_last_status);

}

//恢复上一次设置状态
void Return_last_status(void)
{
  Speaker_Key_Process(pram_status.Voice_last_status);	        //设置页面语音提示按钮状态处理程序
	Safe_Z_process(pram_status.Safe_Z_last_status);             //设置页面安全Z按钮触发后处理程序
	Auto_Knife_process(pram_status.Auto_Knife_last_status);     //设置页面自动对刀按钮触发后处理程序
	Unit_Change_process(pram_status.Unit_Change_last_status);   //设置页面单位切换按钮触发后处理程序
}

//保存当前设置
void Save_Set(void)
{
  Speaker_Key_Process(pram_status.voice_button_status);     //语音提示按钮处理函数
	Safe_Z_process(pram_status.Safe_Z_button_satus);           //安全Z触发处理函数
	Auto_Knife_process(pram_status.Auto_Knife_button_status);  //自动对刀按钮触发后处理程序
	Unit_Change_process(pram_status.Unit_Change_button_status); //单位切换按钮触发后处理程序

}
//控制面板坐标切换控制
void Coordinate_Change_Process(void)
{
	Coordinate_Change_Counter++;
	switch(Coordinate_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,27,"G55");        //设置文本值
			SetTextValue(2,30,"G55");       
			break;  
		}
		case 2:
		{
			SetTextValue(0,27,"G56");
			SetTextValue(2,30,"G56");
			break;
		}
		case 3:
		{
			SetTextValue(0,27,"G57");
			SetTextValue(2,30,"G57");
			break;
		}
		case 4:
		{
			SetTextValue(0,27,"G58");
			SetTextValue(2,30,"G58");
			break;
		}
		case 5:
		{
			SetTextValue(0,27,"G59");
			SetTextValue(2,30,"G59");
			break;
		}
		case 6:
		{
			SetTextValue(0,27,"G54");
			SetTextValue(2,30,"G54");
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
			override.Override_num=0.01;
			break;  
		}
		case 2:
		{
			SetTextValue(0,29,"X2");                                                                                                                                 
			SetTextValue(2,32,"X2");
			override.Override_num=0.02;
			break;
		}
		case 3:
		{
			SetTextValue(0,29,"X5");
			SetTextValue(2,32,"X5");
			override.Override_num=0.05;
			break;
		}
		case 4:
		{
			SetTextValue(0,29,"X10");
			SetTextValue(2,32,"X10");
			override.Override_num=0.1;
			break;
		}
		case 5:
		{
			SetTextValue(0,29,"X20");
			SetTextValue(2,32,"X20");
			override.Override_num=0.2;
			Override_Change_Counter=0;
			break;
		}	
    default:break;		
	}	
  
}

// 在加工页面和控制面板页面显示所有轴坐标
void TFT_Show_coordanate_value(void)
{
	char buf1[20];
	sprintf((char *)buf1,"%09.2f",control_panel_pram.X_value); 
	SetTextValue(2,17,(uchar *)buf1);	                                //显示X轴工件坐标
	SetTextValue(0,16,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(2,18,(uchar *)buf1);                                //显示Y轴工件坐标
	SetTextValue(0,17,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(2,19,(uchar *)buf1);                                //显示Z轴工件坐标
	SetTextValue(0,18,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.A_value);
	SetTextValue(2,20,(uchar *)buf1);                                //显示A轴工件坐标
	SetTextValue(0,19,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.B_value);
	SetTextValue(2,21,(uchar *)buf1);                               //显示B轴工件坐标
	SetTextValue(0,20,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.X_Mac_value); 
	SetTextValue(2,38,(uchar *)buf1);	                               //显示X轴机械坐标
	SetTextValue(0,34,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_Mac_value);
	SetTextValue(2,39,(uchar *)buf1);                                //显示Y轴机械坐标
	SetTextValue(0,35,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_Mac_value);
	SetTextValue(2,40,(uchar *)buf1);                                //显示Z轴机械坐标
	SetTextValue(0,36,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.A_Mac_value);
	SetTextValue(2,41,(uchar *)buf1);                                //显示A轴机械坐标
	SetTextValue(0,37,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.B_Mac_value);
	SetTextValue(2,42,(uchar *)buf1);                               //显示B轴机械坐标
	SetTextValue(0,38,(uchar *)buf1);

}


//显示回工件零页面所有轴坐标值
void Show_coordinate_on_return_workpiece_zero_page(void)
{
	char buf[20];
  sprintf((char *)buf,"%09.2f",control_panel_pram.X_value);
	SetTextValue(3,9,(uchar *)buf);	          //在手轮上显示X轴工件坐标
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(3,10,(uchar *)buf);	        //在手轮上显示Y轴工件坐标     	
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(3,11,(uchar *)buf);	       //在手轮上显示Z轴工件坐标
	
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.A_value);
	SetTextValue(3,12,(uchar *)buf);	       //在手轮上显示A轴工件坐标
		
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.B_value);
	SetTextValue(3,13,(uchar *)buf);	        //在手轮上显示B轴工件坐标
}
//向主机发送X轴坐标
void Send_X_Coordinate_to_Host(void)
{
	char buf1[20];
	sprintf((char *)buf1,"{\"jogx\":%.2f%s",control_panel_pram.X_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //向主机发送X轴坐标
}

//向主机发送Y轴坐标
void Send_Y_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogy\":%.2f%s",control_panel_pram.Y_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //向主机发送Y轴坐标
}

//向主机发送Z轴坐标
void Send_Z_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogz\":%.2f%s",control_panel_pram.Z_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //向主机发送Z轴坐标
}

//向主机发送A轴坐标
void Send_A_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"joga\":%.2f%s",control_panel_pram.A_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //向主机发送A轴坐标
} 

//向主机发送B轴坐标
void Send_B_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogb\":%.2f%s",control_panel_pram.B_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //向主机发送B轴坐标
}




//计算脉冲并显示X轴坐标
void Show_X_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();   //计算脉冲个数												
	control_panel_pram.X_value=control_panel_pram.Temp_save_Xvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.X_value);
	SetTextValue(2,17,(uchar *)buf2);	        //在手轮上显示工件坐标
	SetTextValue(0,16,(uchar *)buf2);         //在手轮上显示工件坐标	
}


//计算脉冲并显示Y轴坐标
void Show_Y_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.Y_value=control_panel_pram.Temp_save_Yvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(2,18,(uchar *)buf2);
	SetTextValue(0,17,(uchar *)buf2);         //在手轮上显示工件坐标

}

//计算脉冲并显示Z轴坐标
void Show_Z_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.Z_value=control_panel_pram.Temp_save_Zvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(2,19,(uchar *)buf2);
	SetTextValue(0,18,(uchar *)buf2);        //在手轮上显示工件坐标
	

}

//计算脉冲并显示A轴坐标
void Show_A_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.A_value=control_panel_pram.Temp_save_Avalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.A_value);
	SetTextValue(2,20,(uchar *)buf2);
	SetTextValue(0,19,(uchar *)buf2);         //在手轮上显示工件坐标
	

}

//计算脉冲并显示B轴坐标
void Show_B_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.B_value=control_panel_pram.Temp_save_Bvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.B_value);
	SetTextValue(2,21,(uchar *)buf2);
	SetTextValue(0,20,(uchar *)buf2);         //在手轮上显示工件坐标

}

