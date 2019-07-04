#ifndef _TFT_H
#define _TFT_H
#include "hmi_user_uart.h"


#define voice_on   0
#define voice_off  1
#define Changed   1
#define Unchanged  0

#define START_ADDR1 0x0800d000     //����һ��Flash��������ʼ��ַ�궨��
#define START_ADDR2 0x0800d400
#define START_ADDR3 0x0800d800
#define START_ADDR4 0x0800dc00

#define	Working_Page  0                //�ӹ�ҳ��
#define	Setting_page  1                //����ҳ��
#define	ControlPanel_Page  2           //�������ҳ��
#define	Return_WorkPiece_Zero_Page  3  //�ع�����ҳ��
#define	Jump_Work_Page  5              //���мӹ�
#define	File_Manage_Page 6            //�ļ�����ҳ��
#define	Leading_In_Page 7             //����ҳ��
#define	Leading_Out_Pgae 8            //����ҳ��
#define	Delete_Page  9                 //ɾ��ҳ��
#define	Storage_View_Page 10           //�ڴ�ռ�Ԥ��ҳ��
#define	Net_Account_Manage_Page 11     //�����˻�����
#define	Choose_WiFi_Page  12           //ѡ��WIFiҳ��
#define	Disconnet_and_SignIn_Page  13   //�Ͽ�������������¼ҳ��
#define	Disconnect_and_SignOut_Page 14 //�Ͽ��������˳���¼ҳ��
#define	Disconnect_Remind_Page 15      //�Ͽ���������ҳ��
#define	SignOut_Remind_Page 16         //�˳���¼����ҳ��
//#define Leading_Out_Pgae 20            //����ҳ��
#define	Save_Pram_Page 21              //���������������ҳ��



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
	uint8 Screen_ID1_Setting_Sure;         //����ҳ��(screen id:1)ȷ����ť״̬
	uint8 Screen_ID1_Setting_concel;       //����ҳ��(screen id:1)ȡ����ť״̬
  uint8 Screen_ID21_Setting_Sure;         //����ҳ��(screen id:2)ȷ����ť״̬
	uint8 Screen_ID21_Setting_concel;       //����ҳ��(screen id:2)ȡ����ť״̬
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
	uint8  Press_button;                  //��¼�ĸ���ť����
  uint8  Axis_press;                    //��¼�ĸ���ѡ��
//	uint8  Y_press;                       //ѡ��Y��
//	uint8  Z_press;                       //ѡ��Z��
//  uint8	 A_press;                       //ѡ��A��
//  uint8  B_press;                       //ѡ��B��
	uint8  Clear_Button;                  //���㰴ť
	uint8  Return_Mac_Zero_button;        //�ػ�е�㰴ť
	uint8  Override_Change_button;        //�����л���ť
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


//��ʾ�����Ѿ�ֹͣ�ӹ�
void Show_Stop_Working(uint8 state);

//��ʾ�������ڼӹ�
void Show_Start_Working(uint8 state);

//XYZAB���갴ť��λ
void XYZAB_button_reset(void);

//������������
void Power_On_Set(void);

//����ҳ�漸������ֵ��ȡ���ϵ籣����flash��
void Setting_page_pram_get(void);

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
void Override_Change_Process(void);

//�ڼӹ�ҳ��Ϳ������ҳ����ʾ����������
void TFT_Show_coordanate_value(uint8 state);


//�ڻع�����ҳ����ʾ����������ֵ
void Show_coordinate_on_return_workpiece_zero_page(void);
////��������������
//void Send_Coordinate_to_Host_Machine(void);



//��ʾX������
void Show_X_Coordinata(uint16 screen_id,uint16 control_id);

//��ʾX������
void Show_Y_Coordinata(uint16 screen_id,uint16 control_id);

//��ʾX������
void Show_Z_Coordinata(uint16 screen_id,uint16 control_id);

//��ʾA������
void Show_A_Coordinata(uint16 screen_id,uint16 control_id);

//��ʾB������
void Show_B_Coordinata(uint16 screen_id,uint16 control_id);


#endif

