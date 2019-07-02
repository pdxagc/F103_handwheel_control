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
	int32  X_Pulses_counter;              //X��������������
	int32  Y_Pulses_counter;              //Y��������������
	int32  Z_Pulses_counter;              //Z��������������
	int32  A_Pulses_counter;              //A��������������
	int32  B_Pulses_counter;              //B��������������
	float  X_value;                       //X�Ṥ������ֵ
	float  X_value_temp;
	float  Y_value;                       //Y�Ṥ������ֵ
	float  Y_value_temp;
	float  Z_value;                       //Z�Ṥ������ֵ
	float  Z_value_temp;
	float  A_value;                       //A�Ṥ������ֵ
	float  A_value_temp;
	float  B_value;                       //B�Ṥ������ֵ
	float  B_value_temp;
	float  X_Mac_value;                   //X���е����ֵ
	float  Y_Mac_value;                   //Y���е����ֵ
	float  Z_Mac_value;                   //Z���е����ֵ
	float  A_Mac_value;                   //A���е����ֵ
	float  B_Mac_value;                   //B���е����ֵ
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

//������������л�����
void Coordinate_Change_Process(void);

//������屶�ʿ���
void Override_Change_Process(void);

//�ӹ�ʱ���ꡢ�ļ�������ʾ
void TFT_Show_coordanate_value(void);

//��������������
void Send_Coordinate_to_Host_Machine(void);

#endif

