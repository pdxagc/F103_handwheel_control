#include "key.h"
#include "stm32f10x_gpio.h"
#include "hmi_user_uart.h"
#include "tft.h"
#include "ulitity.h"


extern uint8 Press_button;
extern Control_Panel_Pram control_panel_pram;      //声明控制面板相关参数的结构体变量


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

}


//按键扫描函数
void Key_scan(void)
{
  uint8 Last_time_Axis_mode;
	
	if(key_X == 0||key_Y == 0||key_Z == 0||key_A == 0||key_B == 0)  //有按键触发
	{
		
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


}

