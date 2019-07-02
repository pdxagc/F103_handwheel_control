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


enum Page_Status
{
	Working_Page,                //�ӹ�ҳ��
	Setting_page,                //����ҳ��
	ControlPanel_Page,           //�������ҳ��
	Return_WorkPiece_Zero_Page,  //�ع�����ҳ��
	Jump_Work_Page,              //���мӹ�
	File_Manage_Page,            //�ļ�����ҳ��
	Leading_In_Page,             //����ҳ��
	Leading_Out_Pgae,            //����ҳ��
	Delete_Page,                 //ɾ��ҳ��
	Storage_View_Page,           //�ڴ�ռ�Ԥ��ҳ��
	Net_Account_Manage_Page,     //����������������¼ҳ��
	Choose_WiFi_Page,            //ѡ��WIFiҳ��
	Disconnet_and_SignIn_Page,   //�Ͽ�������������¼ҳ��
	Disconnect_and_SignOut_Page, //�Ͽ��������˳���¼ҳ��
	Disconnect_Remind_Page,      //�Ͽ���������ҳ��
	SignOut_Remind_Page,         //�˳���¼����ҳ��
	Save_Pram_Page               //���������������ҳ��
};



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
  uint8  X_press;                      //ѡ��X��
	uint8  Y_press;                      //ѡ��Y��
	uint8  Z_press;                      //ѡ��Z��
  uint8	 A_press;                      //ѡ��A��
  uint8  B_press;                      //ѡ��B��
	uint8  Clear_Button;                 //���㰴ť
	uint8  Override_Change_button;       //�����л���ť��־λ
	uint8  All_Spindle_Clear_Button;     //ȫ�����㰴ť
	uint8  Start_Button;                  //��ʼ��ť
	int32  X_Pulses_counter;              //X�������������棨���л�ʱ���ã�
	int32  Y_Pulses_counter;              //Y��������������
	int32  Z_Pulses_counter;              //Z��������������
	int32  A_Pulses_counter;              //A��������������
	int32  B_Pulses_counter;              //B��������������
	double  X_value;                       //X�Ṥ������ֵ
  double  X_value_temp;                  //�����л�ʱ����ԭX������ֵ
	double  Y_value;                       //Y�Ṥ������ֵ
	double  Y_value_temp;
	double  Z_value;                       //Z�Ṥ������ֵ
	double  Z_value_temp;
	double  A_value;                       //A�Ṥ������ֵ
	double  A_value_temp;
	double  B_value;                       //B�Ṥ������ֵ
	double  B_value_temp;
	double  X_Mac_value;                   //X���е����ֵ
	double  Y_Mac_value;                   //Y���е����ֵ
	double  Z_Mac_value;                   //Z���е����ֵ
	double  A_Mac_value;                   //A���е����ֵ
	double  B_Mac_value;                   //B���е����ֵ
}Control_Panel_Pram;



typedef struct State
{
	uint8 Work_state;                           //��������״̬���
	uint8 WiFi_state;                           //WiFI״̬���
	uint8 BT_state;                             //����״̬���
	uint8 Account_state;                        //�˻�����״̬���
	uint8 Hand_wheel_state;                     //���ֿ���״̬	
}State;

typedef struct Override
{
	float Override_num;                           //Ĭ�ϱ��� 
	float Override_num_temp_X;                    //����X�ᱶ��
	float Override_num_temp_Y;                    //����Y�ᱶ��
	float Override_num_temp_Z;                    //����Z�ᱶ��
	float Override_num_temp_A;                    //����A�ᱶ��
	float Override_num_temp_B;                    //����B�ᱶ��	
}Override;

typedef struct Return_Workpiece_Zero
{
	uint8 all_spindle;            //ȫ�ᰴť״̬��1�����£�0:�ɿ�
	uint8 Re_X;                   //X ��ѡ��״̬��1��ѡ�У�0:ûѡ��
	uint8 Re_Y;
	uint8 Re_Z;
	uint8 Re_A;
	uint8 Re_B;
	uint8 Sure;                  //ȷ����ť
	uint8 Cancel;                //ȡ����ť
	float Save_X_Value;          //�����޸�ǰX�������ֵ
	float Save_Y_Value;          //�����޸�ǰY�������ֵ
	float Save_Z_Value;
	float Save_A_Value;
	float Save_B_Value;
	
	float Re_X_Value;            //�������ּ��̴���X�������ֵ
	float Re_Y_Value;            //�������ּ��̴���Y�������ֵ
	float Re_Z_Value;
	float Re_A_Value;
	float Re_B_Value;
}Return_Workpiece_Zero;




typedef struct Devide_Set
{
	uint8 Devide_contronl;       //���п��Ʊ��λ
	uint8 first_clear;           //��һ��������λ
	uint8 Second_clear;          //�ڶ���������λ
  float X_clear_data1;         //X���һ����������
	float X_clear_data2;         //X��ڶ�����������
	float X_devide_date;         //X���������
	float Y_clear_data1;         //Y���һ����������
	float Y_clear_data2;         //Y���һ����������
	float Y_devide_date;         //Y���������
 
}Devide_Set;

typedef struct Jump_Work_Set
{
	uint8 First_get_into;              //�״ν������мӹ�ҳ����λ
	uint8 Jump_Work_Sure;              //���мӹ�ȷ��
	uint8 Jump_Work_cancel;            //���мӹ�ȡ��
	int32 New_work_line;               //��ת�ļӹ�����
	
}Jump_Work_Set;


//��ʾ�����Ѿ�ֹͣ�ӹ�
void WorkingStatus_Stoped(void);

//��ʾ�������ڼӹ�
void WorkingStatus_Starting(void);

//��X�Ṥ����������
void X_coordinate_clear(void);

//��Y�Ṥ����������
void Y_coordinate_clear(void);

//��Z�Ṥ����������
void Z_coordinate_clear(void);

//��A�Ṥ����������
void A_coordinate_clear(void);

//��B�Ṥ����������
void B_coordinate_clear(void);

//�����й�����������
void All_Workpiece_coordinate_clear(void);

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
void TFT_Show_coordanate_value(void);


//�ڻع�����ҳ����ʾ����������ֵ
void Show_coordinate_on_return_workpiece_zero_page(void);
////��������������
//void Send_Coordinate_to_Host_Machine(void);

//����������X������
void Send_X_Coordinate_to_Host(void);

//����������Y������
void Send_Y_Coordinate_to_Host(void);

//����������Z������
void Send_Z_Coordinate_to_Host(void);

//����������A������
void Send_A_Coordinate_to_Host(void);

//����������B������
void Send_B_Coordinate_to_Host(void);


//�������岢��ʾX������
void Show_X_Coordinata(void);

//�������岢��ʾX������
void Show_Y_Coordinata(void);

//�������岢��ʾX������
void Show_Z_Coordinata(void);

//�������岢��ʾA������
void Show_A_Coordinata(void);

//�������岢��ʾB������
void Show_B_Coordinata(void);


#endif

