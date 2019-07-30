#include "key.h"
#include "stm32f10x_gpio.h"
#include "hmi_user_uart.h"
#include "hmi_driver.h"
#include "tft.h"
#include "ulitity.h"
#include "stdio.h"
#include "iic.h"
#include "24c02.h"



extern uint8 Estop_button;         //紧急停止按钮
extern uint8 Press_button;         //记录哪个按钮触发(需要把按键发送给雕刻机)
extern Control_Panel_Pram control_panel_pram;      //声明控制面板相关参数的结构体变量
extern uint8 Estop_Press_time;

//按键初始化函数
void Key_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA,PORTC时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;     //PA4  X轴按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);         //初始化GPIOA4
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;    //PA5  Y轴按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);        //初始化GPIOA.5
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;    //PA6  Z轴按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);        //初始化GPIOA.6
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;    //PB0  A轴按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);        //初始化GPIOB.0
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;    //PB1  B轴按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);        //初始化GPIOB.1
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;    //PB12  急停按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PB12设置成上拉输入	  
	GPIO_Init(GPIOB, &GPIO_InitStructure);        //初始化GPIOB12

}


//按键扫描函数
void Key_scan(void)
{
	static uint8 public_button_unpressed=1;  //按键松开标志  
  uint8 Last_time_Axis_mode;          //记录上一时刻是那个轴处于选中状态
	
	if(public_button_unpressed && (key_X == 0||key_Y == 0||key_Z == 0||key_A == 0||key_B == 0)) //有按键触发
	{
		public_button_unpressed=0;
		Last_time_Axis_mode = Axis_Gets();   //获取上次选中轴
		delay_ms(10);     //去抖动 
		if(key_X == 0)    //X轴按钮
		{
			control_panel_pram.Axis_press = CMD_X_AXIS;
			if(Last_time_Axis_mode != X_mode)  //上一次不是X轴选中
			{
			  Show_X_Axis_State();   //显示X轴选中状态		
			}
		}
		else if(key_Y == 0)  //Y轴按钮
		{
			control_panel_pram.Axis_press = CMD_Y_AXIS;	
			if(Last_time_Axis_mode != Y_mode)
			{	 
         Show_Y_Axis_State();  //显示Y轴选中状态
			}
		}
		else if(key_Z == 0)  //Z轴按钮
		{
			control_panel_pram.Axis_press = CMD_Z_AXIS;	
			if(Last_time_Axis_mode != Z_mode)
			{
			   Show_Z_Axis_State();  //显示Z轴选中状态
			}
		}
		else if(key_A == 0)  //A轴按钮
		{
			control_panel_pram.Axis_press = CMD_A_AXIS;	
			if(Last_time_Axis_mode != A_mode)
			{
			  Show_A_Axis_State();  //显示A轴选中状态
			}
		}
		else if(key_B == 0)  //B轴按钮
		{
			control_panel_pram.Axis_press = CMD_B_AXIS;	
			if(Last_time_Axis_mode != B_mode)
			{
			  Show_B_Axis_State();  //显示B轴选中状态
			}
		}		
	}
	else if(key_X && key_Y && key_Z && key_A && key_B)
	{
	  public_button_unpressed=1;
	}

}


//紧急停止按钮扫描
void Estop_Button_Scan(void)
{
//	uint8 Readvalue;
//	char buf[20];
	static uint8 Estop_button_unpressed=1;         //紧急停止按钮松开标志位,1:松开，0：按下
  if(Estop_button_unpressed && key_Estop == 0)   //急停轴按钮触发
	{
		Estop_button_unpressed=0;
		delay_ms(10);               //去抖动 
		if(key_Estop == 0)
		{
			Press_button = CMD_EStop;
			if(Estop_button==Estop_Off)
				Estop_button=Estop_On;
			else
			{
				Estop_button=Estop_Off;
				ClearTextValue(0,26);
			}
			
			//测试24c02
//			Estop_Press_time++;
//			AT24CXX_WriteOneByte(0,Estop_Press_time);
//			delay_ms(10);  
//			Readvalue=AT24CXX_ReadOneByte(0);
//			sprintf(buf,"%u",Readvalue);	
//			SetTextValue(0,23,(uchar *)buf);
	  }
	}
	else if(key_Estop == 1)
	{
	  Estop_button_unpressed=1;
	}

}
