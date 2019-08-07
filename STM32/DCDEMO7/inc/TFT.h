#ifndef _TFT_H
#define _TFT_H
#include "hmi_user_uart.h"
#include "hmi_driver.h"


#define voice_on   0
#define voice_off  1
#define Changed   1
#define Unchanged  0

#define Show_Disconnect_Network_Control 3
#define Hide_Disconnect_Network_Control 4
#define Show_Sign_Out_Control 3
#define Hide_Sign_Out_Control 4

#define Start 1
#define Stop  0

#define START_ADDR1 0x0800d000     //定义一个Flash操作的起始地址宏定义
#define START_ADDR2 0x0800d400
#define START_ADDR3 0x0800d800
#define START_ADDR4 0x0800dc00


//workpage table
#define	Working_Page  0                //加工页面
#define	Setting_page  1                //设置页面
#define	ControlPanel_Page  2           //控制面板页面
#define	Return_WorkPiece_Zero_Page  3  //回工件零页面
#define	Jump_Work_Page  5              //跳行加工
#define	File_Manage_Page 6            //文件管理页面
#define	Same_file_Error_Page 7         //同名文件报错页面
#define	Download_Fail_Pgae 8            //下载失败页面
#define	Delete_Page  9                 //删除页面
#define	Storage_View_Page 10           //内存空间预览页面
#define	Net_Account_Manage_Page 11     //网络账户管理
#define	Choose_WiFi_Page  12           //选择WIFi页面
//#define	Disconnet_and_SignIn_Page  13   //断开连接与立即登录页面
//#define	Disconnect_and_SignOut_Page 14 //断开连接与退出登录页面
#define	Disconnect_Remind_Page 15      //断开网络提醒页面
#define	SignOut_Remind_Page 16         //退出登录提醒页面
#define Cancel_Download_Page 20        //取消下载页面
#define	Save_Pram_Page 21              //保存参数设置提醒页面


//Working_Page  button table
#define Spindle_Speed_Subtract_Button  3   //主轴速度减按钮
#define Spindle_Speed_Add_Button  4        //主轴速度加按钮
#define Work_Speed_Subtract_Button 5       //加工速度减按钮
#define Work_Speed_Add_Buttoub  6          //加工速度加按钮
#define File_Manage_Button1 12             //文件管理按钮
#define Net_Account_Manage_Button1  13     //网络账户管理按钮
#define Setting_Button  14                 //设置按钮
#define Control_Panel_Button 15            //控制面板按钮
#define X_Press_Button1  42                //X轴按钮
#define Y_Press_Button1  43                //Y轴按钮
#define Z_Press_Button1  44                //Z轴按钮
#define A_Press_Button1  45                //A轴按钮
#define B_Press_Button1  46                //B轴按钮

//Setting_page  button table
#define Safe_Z_Button1 1               //安全Z模式切换
#define Auto_kinfe_Button1 2           //自动对刀模式切换
#define Uint_Button 3                  //单位切换
#define Sure_Button1  4                //确定按钮
#define Cancel_button1 5               //取消按钮
#define Voice_button 27                //声音控制按钮


//ControlPanel_Page  button table
#define Clear_Botton 1                  //清零按钮
#define Return_Machine_Zero_Button 2    //回机械零按钮
#define Multiple_Button 3               //倍率切换按钮
#define Spin_Button 4                   //主轴开关按钮
#define All_Clear_Button 5              //全轴清零按钮
#define Return_Workpiece_Zero_Button 6  //回工件零按钮
#define Coordinate_Button  7            //坐标切换按钮
#define Soft_Limit_button  8            //软限位开关按钮
#define Safe_Z_Button2 9                //安全Z开关按钮
#define Jump_Work_Button 10             //跳行加工按钮
#define Auto_kinfe_Button2 11           //自动对刀按钮
#define Divided_Button 12               //分中按钮
#define Start_Button1 13                //开始按钮
#define Stop_Button 14                  //停止按钮
#define Reset_Button 15                 //复位按钮
#define Exit_Contronl_Panel_Button 16   //退出控制面板按钮
#define X_Press_Button2  22             //X轴按钮
#define Y_Press_Button2  23             //Y轴按钮
#define Z_Press_Button2  24             //Z轴按钮
#define A_Press_Button2  25             //A轴按钮
#define B_Press_Button2  26             //B轴按钮
#define File_Manage_Button2 33          //文件管理按钮
#define Net_Account_Manage_Button2  34  //网络账户管理按钮

//Return_WorkPiece_Zero_Page  button table
#define All_Spindle_Button 1         //全轴选中按钮
#define Cancel_Button2  2            //取消按钮
#define Sure_Button2  3              //确定按钮
#define X_Press_Button3  4           //X轴按钮
#define Y_Press_Button3  5           //Y轴按钮
#define Z_Press_Button3  6           //Z轴按钮
#define A_Press_Button3  7           //A轴按钮
#define B_Press_Button3  8           //B轴按钮

//Jump_Work_Page button table
#define Sure_Button3 1             //确定按钮
#define Cancel_Button3  2          //取消按钮


//File_Manage_Page  button  table
#define Download_button 1             //下载按钮
#define Canael_Download_button 2      //取消下载按钮
#define Delete_Button 3               //删除按钮
#define Storage_button 4              //内存预览按钮
#define Open_and_Load_Button 5        //打开加载按钮
#define Cloud_Last_Page_button 6      //上一页
#define Cloud_Next_Page_button 7      //下一页
#define SD_Last_Page_button 8         //上一页
#define SD_Next_Page_button 9         //下一页
#define Cloud_file_1 10               //云空间文件1
#define Cloud_file_2 11               //云空间文件2
#define Cloud_file_3 12               //云空间文件3
#define Cloud_file_4 13               //云空间文件4
#define SD_file_1 14                  //SD卡文件1
#define SD_file_2 15                  //SD卡文件2
#define SD_file_3 16                  //SD卡文件3
#define SD_file_4 17                  //SD卡文件4
#define Working_Page_button3 18       //文件管理按钮
#define Net_Account_Manage_Button3 19     //网络账户管理按钮

//Delete_Page Button table
#define Sure_Button4 1             //确定按钮
#define Cancel_Button4 2           //取消按钮

//Storage_View_Page button table
#define Net_Account_Manage_Button4  1     //网络账户管理按钮
#define Working_Page_button1 7            //加工页面按钮
#define Return_File_Manage_Button 8       //返回文件管理


//Net_Account_Manage_Page button table
#define Working_Page_button2 1            //加工页面按钮
#define File_Manage_Button4  2            //文件管理按钮
#define Choose_WiFi_Button 3              //选择WiFi
#define Connect_WiFi_Button 4             //连接WiFi
#define Sign_In_Button 5                  //登录账户
#define Disconnect_WIFI_Button 18         //断开WiFi
#define Sign_Out_Button 19                //退出登录

//Choose_WiFi_Page button table
#define WiFi_1_Button 1                  //WiFi1
#define WiFi_2_Button 2                  //WiFi2
#define WiFi_3_Button 3                  //WiFi3
#define WiFi_4_Button 4                  //WiFi4
#define WiFi_Last_Page_Button 6          //上一页
#define WiFi_Next_Page_Button 5          //下一页

//Disconnect_Remind_Page button table
#define Sure_Button5 1               //确定按钮
#define Cancel_Button5 2             //取消按钮
 
//SignOut_Remind_Page button table
#define Sure_Button6 1             //确定按钮
#define Cancel_Button6 2           //取消按钮

//Cancel_Download_Page button table
#define Sure_Button7 1              //确定按钮
#define Cancel_Button7 2            //取消按钮

//Save_Pram_Page button table
#define Sure_Button8 1              //确定按钮
#define Cancel_Button8 2            //取消按钮

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
	uint8 Setting_Page_cancel_button;      //设置页面取消按钮状态
  uint8 Save_Pram_Page_Sure_button;      //保存参数页面确定按钮状态
	uint8 Save_Pram_Page_concel_button;    //保存参数页面取消按钮状态
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
//  uint8  Press_button;                  //记录哪个按钮触发
  uint8  Axis_press;                    //记录哪个轴选中
//	uint8  Y_press;                       //选中Y轴
//	uint8  Z_press;                       //选中Z轴
//  uint8	 A_press;                       //选中A轴
//  uint8  B_press;                       //选中B轴
	uint8  Clear_Button;                  //清零按钮
	uint8  Return_Mac_Zero_button;        //回机械零按钮
	uint8  Multiple_Change_button;        //倍率切换按钮
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


//LCD屏数据处理
void TFT_command_analyse(void);


void Get_Pulses_num(void);

//计算首轮脉冲个数
void Pulses_Count_Process(void);
//屏幕背光亮度控制
void light_Control_Process(void);

//程序进入标记的工作页面，处理相关任务
void TFT_Page_handle(void);

/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void Usart2_Receive_data_handle( PCTRL_MSG msg, uint16 size );

/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param str 文本控件内容
*/

float NotifyText(uint8 *str);

//主机工作状态显示
void Work_state_control(void);



//显示机器已经停止加工
void Show_Stop_Working(uint8 state);

//显示机器正在加工
void Show_Start_Working(uint8 state);

//XYZAB坐标按钮复位
void XYZAB_button_reset(void);

//开机画面设置
void Power_On_Set(void);

//设置页面几个参数值获取（断电保存在flash）
void Get_Setting_page_pram(void);

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
void Multiple_Change_Process(void);

//在加工页面和控制面板页面显示所有轴坐标
void TFT_Show_coordanate_value(void);


//在回工件零页面显示所有轴坐标值
void Show_coordinate_on_return_workpiece_zero_page(void);
////向主机发送坐标
//void Send_Coordinate_to_Host_Machine(void);



//显示X轴坐标
void Show_X_Coordinata(void);

//显示X轴坐标
void Show_Y_Coordinata(void);

//显示X轴坐标
void Show_Z_Coordinata(void);

//显示A轴坐标
void Show_A_Coordinata(void);

//显示B轴坐标
void Show_B_Coordinata(void);


//显示X轴选中状态
void Show_X_Axis_State(void);

//显示Y轴选中状态
void Show_Y_Axis_State(void);

//显示Z轴选中状态
void Show_Z_Axis_State(void);

//显示A轴选中状态
void Show_A_Axis_State(void);

//显示B轴选中状态
void Show_B_Axis_State(void);


#endif

