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

#define START_ADDR1 0x0800d000     //����һ��Flash��������ʼ��ַ�궨��
#define START_ADDR2 0x0800d400
#define START_ADDR3 0x0800d800
#define START_ADDR4 0x0800dc00


//workpage table
#define	Working_Page  0                //�ӹ�ҳ��
#define	Setting_page  1                //����ҳ��
#define	ControlPanel_Page  2           //�������ҳ��
#define	Return_WorkPiece_Zero_Page  3  //�ع�����ҳ��
#define	Jump_Work_Page  5              //���мӹ�
#define	File_Manage_Page 6            //�ļ�����ҳ��
#define	Same_file_Error_Page 7         //ͬ���ļ�����ҳ��
#define	Download_Fail_Pgae 8            //����ʧ��ҳ��
#define	Delete_Page  9                 //ɾ��ҳ��
#define	Storage_View_Page 10           //�ڴ�ռ�Ԥ��ҳ��
#define	Net_Account_Manage_Page 11     //�����˻�����
#define	Choose_WiFi_Page  12           //ѡ��WIFiҳ��
//#define	Disconnet_and_SignIn_Page  13   //�Ͽ�������������¼ҳ��
//#define	Disconnect_and_SignOut_Page 14 //�Ͽ��������˳���¼ҳ��
#define	Disconnect_Remind_Page 15      //�Ͽ���������ҳ��
#define	SignOut_Remind_Page 16         //�˳���¼����ҳ��
#define Cancel_Download_Page 20        //ȡ������ҳ��
#define	Save_Pram_Page 21              //���������������ҳ��


//Working_Page  button table
#define Spindle_Speed_Subtract_Button  3   //�����ٶȼ���ť
#define Spindle_Speed_Add_Button  4        //�����ٶȼӰ�ť
#define Work_Speed_Subtract_Button 5       //�ӹ��ٶȼ���ť
#define Work_Speed_Add_Buttoub  6          //�ӹ��ٶȼӰ�ť
#define File_Manage_Button1 12             //�ļ�����ť
#define Net_Account_Manage_Button1  13     //�����˻�����ť
#define Setting_Button  14                 //���ð�ť
#define Control_Panel_Button 15            //������尴ť
#define X_Press_Button1  42                //X�ᰴť
#define Y_Press_Button1  43                //Y�ᰴť
#define Z_Press_Button1  44                //Z�ᰴť
#define A_Press_Button1  45                //A�ᰴť
#define B_Press_Button1  46                //B�ᰴť

//Setting_page  button table
#define Safe_Z_Button1 1               //��ȫZģʽ�л�
#define Auto_kinfe_Button1 2           //�Զ��Ե�ģʽ�л�
#define Uint_Button 3                  //��λ�л�
#define Sure_Button1  4                //ȷ����ť
#define Cancel_button1 5               //ȡ����ť
#define Voice_button 27                //�������ư�ť


//ControlPanel_Page  button table
#define Clear_Botton 1                  //���㰴ť
#define Return_Machine_Zero_Button 2    //�ػ�е�㰴ť
#define Multiple_Button 3               //�����л���ť
#define Spin_Button 4                   //���Ὺ�ذ�ť
#define All_Clear_Button 5              //ȫ�����㰴ť
#define Return_Workpiece_Zero_Button 6  //�ع����㰴ť
#define Coordinate_Button  7            //�����л���ť
#define Soft_Limit_button  8            //����λ���ذ�ť
#define Safe_Z_Button2 9                //��ȫZ���ذ�ť
#define Jump_Work_Button 10             //���мӹ���ť
#define Auto_kinfe_Button2 11           //�Զ��Ե���ť
#define Divided_Button 12               //���а�ť
#define Start_Button1 13                //��ʼ��ť
#define Stop_Button 14                  //ֹͣ��ť
#define Reset_Button 15                 //��λ��ť
#define Exit_Contronl_Panel_Button 16   //�˳�������尴ť
#define X_Press_Button2  22             //X�ᰴť
#define Y_Press_Button2  23             //Y�ᰴť
#define Z_Press_Button2  24             //Z�ᰴť
#define A_Press_Button2  25             //A�ᰴť
#define B_Press_Button2  26             //B�ᰴť
#define File_Manage_Button2 33          //�ļ�����ť
#define Net_Account_Manage_Button2  34  //�����˻�����ť

//Return_WorkPiece_Zero_Page  button table
#define All_Spindle_Button 1         //ȫ��ѡ�а�ť
#define Cancel_Button2  2            //ȡ����ť
#define Sure_Button2  3              //ȷ����ť
#define X_Press_Button3  4           //X�ᰴť
#define Y_Press_Button3  5           //Y�ᰴť
#define Z_Press_Button3  6           //Z�ᰴť
#define A_Press_Button3  7           //A�ᰴť
#define B_Press_Button3  8           //B�ᰴť

//Jump_Work_Page button table
#define Sure_Button3 1             //ȷ����ť
#define Cancel_Button3  2          //ȡ����ť


//File_Manage_Page  button  table
#define Download_button 1             //���ذ�ť
#define Canael_Download_button 2      //ȡ�����ذ�ť
#define Delete_Button 3               //ɾ����ť
#define Storage_button 4              //�ڴ�Ԥ����ť
#define Open_and_Load_Button 5        //�򿪼��ذ�ť
#define Cloud_Last_Page_button 6      //��һҳ
#define Cloud_Next_Page_button 7      //��һҳ
#define SD_Last_Page_button 8         //��һҳ
#define SD_Next_Page_button 9         //��һҳ
#define Cloud_file_1 10               //�ƿռ��ļ�1
#define Cloud_file_2 11               //�ƿռ��ļ�2
#define Cloud_file_3 12               //�ƿռ��ļ�3
#define Cloud_file_4 13               //�ƿռ��ļ�4
#define SD_file_1 14                  //SD���ļ�1
#define SD_file_2 15                  //SD���ļ�2
#define SD_file_3 16                  //SD���ļ�3
#define SD_file_4 17                  //SD���ļ�4
#define Working_Page_button3 18       //�ļ�����ť
#define Net_Account_Manage_Button3 19     //�����˻�����ť

//Delete_Page Button table
#define Sure_Button4 1             //ȷ����ť
#define Cancel_Button4 2           //ȡ����ť

//Storage_View_Page button table
#define Net_Account_Manage_Button4  1     //�����˻�����ť
#define Working_Page_button1 7            //�ӹ�ҳ�水ť
#define Return_File_Manage_Button 8       //�����ļ�����


//Net_Account_Manage_Page button table
#define Working_Page_button2 1            //�ӹ�ҳ�水ť
#define File_Manage_Button4  2            //�ļ�����ť
#define Choose_WiFi_Button 3              //ѡ��WiFi
#define Connect_WiFi_Button 4             //����WiFi
#define Sign_In_Button 5                  //��¼�˻�
#define Disconnect_WIFI_Button 18         //�Ͽ�WiFi
#define Sign_Out_Button 19                //�˳���¼

//Choose_WiFi_Page button table
#define WiFi_1_Button 1                  //WiFi1
#define WiFi_2_Button 2                  //WiFi2
#define WiFi_3_Button 3                  //WiFi3
#define WiFi_4_Button 4                  //WiFi4
#define WiFi_Last_Page_Button 6          //��һҳ
#define WiFi_Next_Page_Button 5          //��һҳ

//Disconnect_Remind_Page button table
#define Sure_Button5 1               //ȷ����ť
#define Cancel_Button5 2             //ȡ����ť
 
//SignOut_Remind_Page button table
#define Sure_Button6 1             //ȷ����ť
#define Cancel_Button6 2           //ȡ����ť

//Cancel_Download_Page button table
#define Sure_Button7 1              //ȷ����ť
#define Cancel_Button7 2            //ȡ����ť

//Save_Pram_Page button table
#define Sure_Button8 1              //ȷ����ť
#define Cancel_Button8 2            //ȡ����ť

typedef struct Speed_Control
{
	uint16 Initial_Spindle_Speed;                     //�����ʼת��
	int    Initial_Spindle_Speed_Percent;             //�����ʼ�ٶȰٷֱ�
	int    Initial_Work_Speed;                        //�ӹ���ʼ�ٶ�
	int    Initial_Work_Speed_Percent;                //�ӹ���ʼ�ٶȰٷֱ�
	uint32 Changed_Spindle_Speed;                     //�仯�������ת��              
	int16  Changed_Work_Speed;                        //�仯��ļӹ��ٶ�
	uint8  Spindle_Speed_Changed;                     //�����ٶȷ����仯��־λ
	uint8  Work_Speed_Changed;                        //�ӹ��ٶȷ����仯��־λ
}Speed_Control;



typedef struct Pram_Status
{
	uint8 Safe_Z_last_status;             //��ȫZ��һ��״̬
	uint8 Safe_Z_button_satus;            //��ȫZ��ť״̬
	uint8 Auto_Knife_last_status;         //�Զ��Ե���һ����״̬
	uint8 Auto_Knife_button_status;       //�Զ��Ե���ť״̬
	uint8 Unit_Change_last_status;        //��λѡ����һ��״̬
	uint8 Unit_Change_button_status;       //��λѡ��ť״̬
	uint8 Voice_last_status;               //�������ƣ���һ�εĿ���״̬����0��ʹ�ܣ�1����ֹ
	uint8 voice_button_status;             //������ť״̬    0���ɿ�=ʹ�� ��1������=��ֹ
	uint8 Setting_Page_cancel_button;      //����ҳ��ȡ����ť״̬
  uint8 Save_Pram_Page_Sure_button;      //�������ҳ��ȷ����ť״̬
	uint8 Save_Pram_Page_concel_button;    //�������ҳ��ȡ����ť״̬
	float Safe_Z_num;                       //��ȫZ�߶�
	float Knife_high_num;                   //�Ե��߶�
	float Knife_block_high_num;             //�Ե���߶�
	float Auto_Knife_block_X;               //X��Ե���λ��
	float Auto_Knife_block_Y;               //Y��Ե���λ��
	float Auto_Knife_block_Z;               //Z��Ե���λ��
	float Soft_limit_X;                     //X������λֵ
	float Soft_limit_Y;                     //Y������λֵ
	float Soft_limit_Z;                     //Z������λֵ
	float Soft_limit_A;                     //A������λֵ
	float Soft_limit_B;                     //B������λֵ
}Pram_Status;




typedef struct Control_Panel_Pram
{
//  uint8  Press_button;                  //��¼�ĸ���ť����
  uint8  Axis_press;                    //��¼�ĸ���ѡ��
//	uint8  Y_press;                       //ѡ��Y��
//	uint8  Z_press;                       //ѡ��Z��
//  uint8	 A_press;                       //ѡ��A��
//  uint8  B_press;                       //ѡ��B��
	uint8  Clear_Button;                  //���㰴ť
	uint8  Return_Mac_Zero_button;        //�ػ�е�㰴ť
	uint8  Multiple_Change_button;        //�����л���ť
	uint8  Spin_Switch_button;            //���Ὺ��
	uint8  All_Spindle_Clear_Button;      //ȫ�����㰴ť
	uint8  Return_WorkPiece_Zero_button;  //�ع����㰴ť
	uint8  Coordinate_Change_button;      //�����л���ť��
	uint8  Soft_limit_button;            //����λ���ذ�ť
	uint8  Safe_Z_button;                 //��ȫZ���ذ�ť
	uint8  Jump_Work_button;              //���мӹ���ť
	uint8  Auto_knife_button;             //�Զ��Ե���ť
	uint8  Start_Button;                  //��ʼ��ť
	float  X_value;                       //X�Ṥ������ֵ
	float  X_last_value; 
	float  Y_value;                       //Y�Ṥ������ֵ
	float  Y_last_value; 
	float  Z_value;                       //Z�Ṥ������ֵ
	float  Z_last_value; 
	float  A_value;                       //A�Ṥ������ֵ
	float  A_last_value;
	float  B_value;                       //B�Ṥ������ֵ
	float  B_last_value;
	float  X_Mac_value;                   //X���е����ֵ
	float  Y_Mac_value;                   //Y���е����ֵ
	float  Z_Mac_value;                   //Z���е����ֵ
	float  A_Mac_value;                   //A���е����ֵ
	float  B_Mac_value;                   //B���е����ֵ
	float  Temp_save_Xvalue;              //��ʱ����X������ֵ
	float  Temp_save_Yvalue;              //��ʱ����X������ֵ            
	float  Temp_save_Zvalue;              //��ʱ����X������ֵ
	float  Temp_save_Avalue;              //��ʱ����X������ֵ
	float  Temp_save_Bvalue;              //��ʱ����X������ֵ
	
}Control_Panel_Pram;



typedef struct State
{
	uint8 Work_state;                           //��������״̬���
	uint8 WiFi_state;                           //WiFI״̬���
	uint8 BT_state;                             //����״̬���
	uint8 Account_state;                        //�˻�����״̬���
	uint8 Hand_wheel_state;                     //���ֿ���״̬	
}State;


typedef struct Return_Workpiece_Zero
{
	uint8 all_spindle_status;     //ȫ�ᰴťѡ��״̬��1���ɿ���0:ѡ�У������ᶼ���㣩
	uint8 X_clear_status;         //X��ѡ������״̬��1��ûѡ�У�0:ѡ������
	uint8 Y_clear_status;         //Y��ѡ������״̬��1��ûѡ�У�0:ѡ������
	uint8 Z_clear_status;
	uint8 A_clear_status;
	uint8 B_clear_status;	
	uint8 Sure;                  //ȷ����ťѡ�б�־λ��1��ѡ�У�0��δѡ��
	uint8 Cancel;                //ȡ����ťѡ�б�־λ��1��ѡ�У�0��δѡ��	
	float Re_X_Value;            //���ּ��̴���X�������ֵ
	float Re_Y_Value;            //���ּ��̴���Y�������ֵ
	float Re_Z_Value;
	float Re_A_Value;
	float Re_B_Value;
	uint8 X_get_value;           //X���ȡ��������ֵ��־λ��1����ȡ��������ֵ��0��û�л�ȡ��������ֵ
	uint8 Y_get_value;           //Y���ȡ��������ֵ��־λ��1����ȡ��������ֵ��0��û�л�ȡ��������ֵ
	uint8 Z_get_value;
	uint8 A_get_value;
	uint8 B_get_value;
}Return_Workpiece_Zero;




typedef struct Devide_Set
{
	uint8 Devide_contronl;       //���п��Ʊ��λ
 
}Devide_Set;

typedef struct Jump_Work_Set
{
	uint8 First_get_into;              //�״ν������мӹ�ҳ����λ
	uint8 Jump_Work_Sure;              //���мӹ�ȷ��
	uint8 Jump_Work_cancel;            //���мӹ�ȡ��
	int32 New_work_line;               //��ת�ļӹ�����
	
}Jump_Work_Set;


//LCD�����ݴ���
void TFT_command_analyse(void);


void Get_Pulses_num(void);

//���������������
void Pulses_Count_Process(void);
//��Ļ�������ȿ���
void light_Control_Process(void);

//��������ǵĹ���ҳ�棬�����������
void TFT_Page_handle(void);

/*! 
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void Usart2_Receive_data_handle( PCTRL_MSG msg, uint16 size );

/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param str �ı��ؼ�����
*/

float NotifyText(uint8 *str);

//��������״̬��ʾ
void Work_state_control(void);



//��ʾ�����Ѿ�ֹͣ�ӹ�
void Show_Stop_Working(uint8 state);

//��ʾ�������ڼӹ�
void Show_Start_Working(uint8 state);

//XYZAB���갴ť��λ
void XYZAB_button_reset(void);

//������������
void Power_On_Set(void);

//����ҳ�漸������ֵ��ȡ���ϵ籣����flash��
void Get_Setting_page_pram(void);

//�ӹ����������ٶȺͼӹ��ٶȰ�ť����
void Spindle_and_Work_Speed_Key_Process(void);

//����ҳ��������ʾ��ť�����������
void Speaker_Key_Process(uint8  state);

//����ҳ�氲ȫZ��ť�����������
void Safe_Z_process(uint8 state);

//����ҳ���Զ��Ե���ť�����������
void Auto_Knife_process(uint8 state);

//����ҳ�浥λ�л���ť�����������
void Unit_Change_process(uint8 state);

//�ָ���һ������״̬
void Return_last_status(void);

//���浱ǰ����
void Save_Set(void);

//������������л�����
void Coordinate_Change_Process(void);

//������屶�ʿ���
void Multiple_Change_Process(void);

//�ڼӹ�ҳ��Ϳ������ҳ����ʾ����������
void TFT_Show_coordanate_value(void);


//�ڻع�����ҳ����ʾ����������ֵ
void Show_coordinate_on_return_workpiece_zero_page(void);
////��������������
//void Send_Coordinate_to_Host_Machine(void);



//��ʾX������
void Show_X_Coordinata(void);

//��ʾX������
void Show_Y_Coordinata(void);

//��ʾX������
void Show_Z_Coordinata(void);

//��ʾA������
void Show_A_Coordinata(void);

//��ʾB������
void Show_B_Coordinata(void);


//��ʾX��ѡ��״̬
void Show_X_Axis_State(void);

//��ʾY��ѡ��״̬
void Show_Y_Axis_State(void);

//��ʾZ��ѡ��״̬
void Show_Z_Axis_State(void);

//��ʾA��ѡ��״̬
void Show_A_Axis_State(void);

//��ʾB��ѡ��״̬
void Show_B_Axis_State(void);


#endif

