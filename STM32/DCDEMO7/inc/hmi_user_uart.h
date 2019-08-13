/*! 
*  \file hmi_driver.h
*  \brief ���ڳ�ʼ��
*  
*/
#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f10x_it.h"     //�����û�MCU�����޸�


#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long


#define X_mode 10
#define Y_mode 11
#define Z_mode 12
#define A_mode 13
#define B_mode 14


//����������ͨѶ�����
// COMMAND TABLE
#define CMD_ASK_SLAVE 29                 //������������
#define CMD_RPY_HC_MPG1	36               //���ַ�������
#define CMD_UPDATE_MACH3_NUMBER	41       //����������������
#define CMD_Working_File_Name 50         //�ӹ��ļ���
#define CMD_Work_line 51                 //�ӹ�����
#define CMD_Working_Code 52              //�ӹ�����
#define CMD_SPin_Speed 53                //�����ٶ�
#define CMD_Working_Speed 54             //�ӹ��ٶ�
#define CMD_Warn_Massage  55             //������Ϣ
#define CMD_Clond_File_Name 56           //�ƿռ��ļ���
#define CMD_SD_Card_File_Name 57         //SD���ļ���
#define CMD_Storage_Data  58             //�ڴ�����
#define CMD_Wifi_Name  59                //wifi����
#define CMD_Wifi_Password 60             //wif����
#define CMD_Account_Name  61             //�û��˻�
#define CMD_Account_Password 62          //�û�����


//����������ͨѶ����ָ���
//�������
#define CMD_X_AXIS	0                  //X��ѡ��
#define CMD_Y_AXIS	1                  //Y��ѡ��
#define CMD_Z_AXIS  2                   //Z��ѡ��
#define CMD_A_AXIS  3                   //A��ѡ��
#define CMD_B_AXIS  100                 //B��ѡ��
#define CMD_Clear   4                     //����
#define CMD_Return_WorkPiece_Zero 5     //�ع�����
#define CMD_Divided	 6                   //����
#define CMD_Multiple_Change	7           //�����л�
#define CMD_Spin_On_Off	 8               //���Ὺ��
#define CMD_Auto_knife	9               //�Զ��Ե�
#define CMD_Return_Machine_Zero 10      //�ػ�е��
#define CMD_ 11
#define CMD_Start 12                    //��ʼ�ӹ�
#define CMD_1	13
#define CMD_2	14
#define CMD_EStop	15                    //����ֹͣ

#define CMD_All_Spin_Clear	17          //ȫ������
#define CMD_Coordinate_Change 18        //�����л�
#define CMD_Soft_Limit	19              //����λ
#define CMD_Jump_Work	20                //���мӹ�
#define CMD_Safe_Z	21                   //��ȫZ
#define CMD_Reset	22                     //��λ
#define CMD_Stop  23                     //ֹͣ�ӹ�

//�ӹ�ҳ��
#define CMD_Spindle_Speed_Add  24        //�����ٶȼ�
#define CMD_Spindle_Speed_Subtract 25    //�����ٶȼ�
#define CMD_Work_Speed_add  26            //�ӹ��ٶȼ�
#define CMD_Work_Speed_Suttract  27      //�ӹ��ٶȼ�

//����ҳ��
#define CMD_Safe_Z_Mode  28             //��ȫZģʽ
#define CMD_Auto_Knife_Mode  30          //�Զ��Ե�ģʽ
#define CMD_Uint_Mode  31               //��λģʽ
#define CMD_Voice_Switch  32            //��������
#define CMD_Sure   33                   //ȷ��
#define CMD_Cancel 34                   //ȡ��

//�ļ�����
#define CMD_Download 35                 //����
#define CMD_Cancel_Download  38         //ȡ������
#define CMD_Delete    39                //ɾ��
#define CMD_Storage_View 40             //�ڴ�Ԥ��
#define CMD_Open_Loading 42             //�򿪼���
#define CMD_File_Delete_Sure 43         //�ļ�ɾ��
#define CMD_Clode_Flie_Last_Page 44     //�ƿռ��ļ���һҳ
#define CMD_Clode_Flie_Next_Page 45     //�ƿռ��ļ���һҳ
#define CMD_SD_File_Last_page   46      //SD���ļ���һҳ
#define CMD_SD_File_Nest_page   47      //SD���ļ���һҳ
#define CMD_Cloud_File_1 60               //�ƿռ��ļ�1
#define CMD_Cloud_File_2 61               //�ƿռ��ļ�2
#define CMD_Cloud_File_3 62               //�ƿռ��ļ�3
#define CMD_Cloud_File_4 63               //�ƿռ��ļ�4
#define CMD_SD_File_1 64                  //SD���ļ�1
#define CMD_SD_File_2 65                  //SD���ļ�2
#define CMD_SD_File_3 66                  //SD���ļ�3
#define CMD_SD_File_4 67                  //SD���ļ�4


//�����������˻���¼
#define CMD_WiFi_Choose_Page   68      //WiFiҳ��
#define CMD_Connect_WIFI       69     //����WiFi
#define CMD_Disconnect_WiFi    70     //�Ͽ�WiFi
#define CMD_Sign_In            71     //��¼�˻�
#define CMD_SIgn_Out           72     //�˳��˻�
#define CMD_WiFi_1_Button  73                  //WiFi1
#define CMD_WiFi_2_Button 74                  //WiFi2
#define CMD_WiFi_3_Button 75                  //WiFi3
#define CMD_WiFi_4_Button 76                  //WiFi4
#define CMD_WiFi_Last_Page_Button 77          //��һҳ
#define CMD_WiFi_Next_Page_Button 78          //��һҳ







// ����1��ʼ��
void Usart1_Init(uint32 Baudrate);


//����2��ʱ�ӳ�ʼ������    A2,A3    
void Usart2_Init(uint32 BaudRate);

//����3��ʱ�ӳ�ʼ������    A2,A3 
void Usart3_Init(uint32 BaudRate);

//RS485 ģʽ����.en:0,����;1,����.
void RS485_mode_control(uint8 en);


/***************************************************************************
*   \name   SendChar()��������USART1ר�ã�
*   \brief  ����1���ֽ� 
*   \param  t ���͵��ֽ�
*****************************************************************************/
void  SendChar(uchar t);


// ���͵��ֽ�
void Usart_SendByte( USART_TypeDef * pUSARTx, uint8_t ch);

// ���͵��ַ���
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
 
//Usart3_send_Str ������������
// void Usart3_send_Str(uint8 buf[]);

// ��������ǰ������һ��У�����	
char SetXor	(char length, char start);

//�������ݺ�����һ�����������ǰ������Աȣ�һ�µĻ�����յ�������ȷ
unsigned char CheckXor (char data, unsigned char len);

//��ַУ��
uint8_t Check_Address (char data);

//�����жϣ����յ�̻�����  //place at ISR
void Usart1_Recieve_Process (void);

//����1�������ݴ�����
void Communication_Data_handle (void);

//��������������
void Send_data_to_Master(void);

// ����2�������ݺ���
void Usart1_Send_Data (uint8 length);

//�ж������Ƿ����仯
uint8 Check_Pulses_change(void);

//�жϰ����Ƿ����仯
uint8 Check_CMD_button_change(void);

//�жϱ����Ƿ����仯
uint8 Check_Multiple_change(void);

//ȷ�����ĸ���ѡ��
uint8 Axis_Gets(void);

//����ָ�������
void Create_CMD_and_Date(void);

#endif
