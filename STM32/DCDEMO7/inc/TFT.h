#ifndef _TFT_H
#define _TFT_H
#include "hmi_user_uart.h"


#define voice_on   0
#define voice_off  1
#define Changed   1
#define Unchanged  0

#define START_ADDR1 0x0800d000     //定义一个Flash操作的起始地址宏定义
#define START_ADDR2 0x0800d400
#define START_ADDR3 0x0800d800
#define START_ADDR4 0x0800dc00

typedef struct Speed_Control
{
	uint16 Initial_Spindle_Speed;                     //主轴初始转速
	int    Initial_Spindle_Speed_Percent;             //主轴初始速度百分比
	int    Initial_Work_Speed;                        //加工初始速度
	int    Initial_Work_Speed_Percent;                //加工初始速度百分比
	uint32 Changed_Spindle_Speed;                     //变化后的主轴转速              
	int16  Changed_Work_Speed;                        //变化后的加工速度
	uint8  Spindle_Speed_Changed;                     //主轴速度发生变化标志位
	uint8  Work_Speed_Changed;                        //加工速度发生变化标志位
}Speed_Control;



typedef struct Pram_Status
{
	uint8 Safe_Z_last_status;             //安全Z上一次状态
	uint8 Safe_Z_button_satus;            //安全Z按钮状态
	uint8 Auto_Knife_last_status;         //自动对刀上一次主状态
	uint8 Auto_Knife_button_status;       //自动对刀按钮状态
	uint8 Unit_Change_last_status;        //单位选择上一次状态
	uint8 Unit_Change_button_status;       //单位选择按钮状态
	uint8 Voice_last_status;               //语音控制（上一次的控制状态），0：使能，1：禁止
	uint8 voice_button_status;             //语音按钮状态    0：松开=使能 ，1：按下=禁止
	uint8 Screen_ID1_Setting_Sure;         //设置页面(screen id:1)确定按钮状态
	uint8 Screen_ID1_Setting_concel;       //设置页面(screen id:1)取消按钮状态
  uint8 Screen_ID21_Setting_Sure;         //设置页面(screen id:2)确定按钮状态
	uint8 Screen_ID21_Setting_concel;       //设置页面(screen id:2)取消按钮状态
	float Safe_Z_num;                       //安全Z高度
	float Knife_high_num;                   //对刀高度
	float Knife_block_high_num;             //对刀块高度
	float Auto_Knife_block_X;               //X轴对刀块位置
	float Auto_Knife_block_Y;               //Y轴对刀块位置
	float Auto_Knife_block_Z;               //Z轴对刀块位置
	float Soft_limit_X;                     //X轴软限位值
	float Soft_limit_Y;                     //Y轴软限位值
	float Soft_limit_Z;                     //Z轴软限位值
	float Soft_limit_A;                     //A轴软限位值
	float Soft_limit_B;                     //B轴软限位值
}Pram_Status;




typedef struct Control_Panel_Pram
{
  uint8  X_press;                      //选中X轴
	uint8  Y_press;                      //选中Y轴
	uint8  Z_press;                      //选中Z轴
  uint8	 A_press;                      //选中A轴
  uint8  B_press;                      //选中B轴
	uint8  Clear_Button;                 //清零按钮
	uint8  Override_Change_button;       //倍率切换按钮标志位
	uint8  All_Spindle_Clear_Button;     //全轴清零按钮
	uint8  Start_Button;                  //开始按钮
	int32  X_Pulses_counter;              //X轴脉冲数量保存
	int32  Y_Pulses_counter;              //Y轴脉冲数量保存
	int32  Z_Pulses_counter;              //Z轴脉冲数量保存
	int32  A_Pulses_counter;              //A轴脉冲数量保存
	int32  B_Pulses_counter;              //B轴脉冲数量保存
	float  X_value;                       //X轴工件坐标值
	float  X_value_temp;
	float  Y_value;                       //Y轴工件坐标值
	float  Y_value_temp;
	float  Z_value;                       //Z轴工件坐标值
	float  Z_value_temp;
	float  A_value;                       //A轴工件坐标值
	float  A_value_temp;
	float  B_value;                       //B轴工件坐标值
	float  B_value_temp;
	float  X_Mac_value;                   //X轴机械坐标值
	float  Y_Mac_value;                   //Y轴机械坐标值
	float  Z_Mac_value;                   //Z轴机械坐标值
	float  A_Mac_value;                   //A轴机械坐标值
	float  B_Mac_value;                   //B轴机械坐标值
}Control_Panel_Pram;



typedef struct State
{
	uint8 Work_state;                           //机器工作状态标记
	uint8 WiFi_state;                           //WiFI状态标记
	uint8 BT_state;                             //蓝牙状态标记
	uint8 Account_state;                        //账户登入状态标记
	uint8 Hand_wheel_state;                     //手轮开关状态	
}State;

typedef struct Override
{
	float Override_num;                           //默认倍率 
	float Override_num_temp_X;                    //保存X轴倍率
	float Override_num_temp_Y;                    //保存Y轴倍率
	float Override_num_temp_Z;                    //保存Z轴倍率
	float Override_num_temp_A;                    //保存A轴倍率
	float Override_num_temp_B;                    //保存B轴倍率	
}Override;




//显示机器已经停止加工
void WorkingStatus_Stoped(void);

//显示机器正在加工
void WorkingStatus_Starting(void);

//对X轴工件坐标清零
void X_coordinate_clear(void);

//对Y轴工件坐标清零
void Y_coordinate_clear(void);

//对Z轴工件坐标清零
void Z_coordinate_clear(void);

//对A轴工件坐标清零
void A_coordinate_clear(void);

//对B轴工件坐标清零
void B_coordinate_clear(void);

//对所有工件坐标清零
void All_Workpiece_coordinate_clear(void);

//XYZAB坐标按钮复位
void XYZAB_button_reset(void);

//开机画面设置
void Power_On_Set(void);

//设置页面几个参数值获取（断电保存在flash）
void Setting_page_pram_get(void);

//加工中心主轴速度和加工速度按钮处理
void Spindle_and_Work_Speed_Key_Process(void);

//设置页面语音提示按钮触发后处理程序
void Speaker_Key_Process(uint8  state);

//设置页面安全Z按钮触发后处理程序
void Safe_Z_process(uint8 state);

//设置页面自动对刀按钮触发后处理程序
void Auto_Knife_process(uint8 state);

//设置页面单位切换按钮触发后处理程序
void Unit_Change_process(uint8 state);

//控制面板坐标切换控制
void Coordinate_Change_Process(void);

//控制面板倍率控制
void Override_Change_Process(void);

//加工时坐标、文件名等显示
void TFT_Show_coordanate_value(void);

//向主机发送坐标
void Send_Coordinate_to_Host_Machine(void);

#endif

