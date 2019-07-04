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

#define	Working_Page  0                //加工页面
#define	Setting_page  1                //设置页面
#define	ControlPanel_Page  2           //控制面板页面
#define	Return_WorkPiece_Zero_Page  3  //回工件零页面
#define	Jump_Work_Page  5              //跳行加工
#define	File_Manage_Page 6            //文件管理页面
#define	Leading_In_Page 7             //导入页面
#define	Leading_Out_Pgae 8            //导出页面
#define	Delete_Page  9                 //删除页面
#define	Storage_View_Page 10           //内存空间预览页面
#define	Net_Account_Manage_Page 11     //网络账户管理
#define	Choose_WiFi_Page  12           //选择WIFi页面
#define	Disconnet_and_SignIn_Page  13   //断开连接与立即登录页面
#define	Disconnect_and_SignOut_Page 14 //断开连接与退出登录页面
#define	Disconnect_Remind_Page 15      //断开网络提醒页面
#define	SignOut_Remind_Page 16         //退出登录提醒页面
//#define Leading_Out_Pgae 20            //导出页面
#define	Save_Pram_Page 21              //保存参数设置提醒页面



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
	uint8  Press_button;                  //记录哪个按钮触发
  uint8  Axis_press;                    //记录哪个轴选中
//	uint8  Y_press;                       //选中Y轴
//	uint8  Z_press;                       //选中Z轴
//  uint8	 A_press;                       //选中A轴
//  uint8  B_press;                       //选中B轴
	uint8  Clear_Button;                  //清零按钮
	uint8  Return_Mac_Zero_button;        //回机械零按钮
	uint8  Override_Change_button;        //倍率切换按钮
	uint8  Spin_Switch_button;            //主轴开关
	uint8  All_Spindle_Clear_Button;      //全轴清零按钮
	uint8  Return_WorkPiece_Zero_button;  //回工件零按钮
	uint8  Coordinate_Change_button;      //坐标切换按钮、
	uint8  Soft_limit_button;            //软限位开关按钮
	uint8  Safe_Z_button;                 //安全Z开关按钮
	uint8  Jump_Work_button;              //跳行加工按钮
	uint8  Auto_knife_button;             //自动对刀按钮
	uint8  Start_Button;                  //开始按钮
	float  X_value;                       //X轴工件坐标值
	float  X_last_value; 
	float  Y_value;                       //Y轴工件坐标值
	float  Y_last_value; 
	float  Z_value;                       //Z轴工件坐标值
	float  Z_last_value; 
	float  A_value;                       //A轴工件坐标值
	float  A_last_value;
	float  B_value;                       //B轴工件坐标值
	float  B_last_value;
	float  X_Mac_value;                   //X轴机械坐标值
	float  Y_Mac_value;                   //Y轴机械坐标值
	float  Z_Mac_value;                   //Z轴机械坐标值
	float  A_Mac_value;                   //A轴机械坐标值
	float  B_Mac_value;                   //B轴机械坐标值
	float  Temp_save_Xvalue;              //临时保存X轴坐标值
	float  Temp_save_Yvalue;              //临时保存X轴坐标值            
	float  Temp_save_Zvalue;              //临时保存X轴坐标值
	float  Temp_save_Avalue;              //临时保存X轴坐标值
	float  Temp_save_Bvalue;              //临时保存X轴坐标值
	
}Control_Panel_Pram;



typedef struct State
{
	uint8 Work_state;                           //机器工作状态标记
	uint8 WiFi_state;                           //WiFI状态标记
	uint8 BT_state;                             //蓝牙状态标记
	uint8 Account_state;                        //账户登入状态标记
	uint8 Hand_wheel_state;                     //手轮开关状态	
}State;


typedef struct Return_Workpiece_Zero
{
	uint8 all_spindle_status;     //全轴按钮选中状态，1：松开，0:选中（所有轴都清零）
	uint8 X_clear_status;         //X轴选定清零状态，1：没选中，0:选中清零
	uint8 Y_clear_status;         //Y轴选定清零状态，1：没选中，0:选中清零
	uint8 Z_clear_status;
	uint8 A_clear_status;
	uint8 B_clear_status;	
	uint8 Sure;                  //确定按钮选中标志位，1：选中，0：未选中
	uint8 Cancel;                //取消按钮选中标志位，1：选中，0：未选中	
	float Re_X_Value;            //数字键盘传给X轴的坐标值
	float Re_Y_Value;            //数字键盘传给Y轴的坐标值
	float Re_Z_Value;
	float Re_A_Value;
	float Re_B_Value;
	uint8 X_get_value;           //X轴获取到新坐标值标志位，1：获取到新坐标值，0：没有获取到新坐标值
	uint8 Y_get_value;           //Y轴获取到新坐标值标志位，1：获取到新坐标值，0：没有获取到新坐标值
	uint8 Z_get_value;
	uint8 A_get_value;
	uint8 B_get_value;
}Return_Workpiece_Zero;




typedef struct Devide_Set
{
	uint8 Devide_contronl;       //分中控制标记位
 
}Devide_Set;

typedef struct Jump_Work_Set
{
	uint8 First_get_into;              //首次进入跳行加工页面标记位
	uint8 Jump_Work_Sure;              //跳行加工确定
	uint8 Jump_Work_cancel;            //跳行加工取消
	int32 New_work_line;               //跳转的加工行数
	
}Jump_Work_Set;


//显示机器已经停止加工
void Show_Stop_Working(uint8 state);

//显示机器正在加工
void Show_Start_Working(uint8 state);

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

//恢复上一次设置状态
void Return_last_status(void);

//保存当前设置
void Save_Set(void);

//控制面板坐标切换控制
void Coordinate_Change_Process(void);

//控制面板倍率控制
void Override_Change_Process(void);

//在加工页面和控制面板页面显示所有轴坐标
void TFT_Show_coordanate_value(uint8 state);


//在回工件零页面显示所有轴坐标值
void Show_coordinate_on_return_workpiece_zero_page(void);
////向主机发送坐标
//void Send_Coordinate_to_Host_Machine(void);



//显示X轴坐标
void Show_X_Coordinata(uint16 screen_id,uint16 control_id);

//显示X轴坐标
void Show_Y_Coordinata(uint16 screen_id,uint16 control_id);

//显示X轴坐标
void Show_Z_Coordinata(uint16 screen_id,uint16 control_id);

//显示A轴坐标
void Show_A_Coordinata(uint16 screen_id,uint16 control_id);

//显示B轴坐标
void Show_B_Coordinata(uint16 screen_id,uint16 control_id);


#endif

