/*! 
*  \file hmi_driver.h
*  \brief ���ڳ�ʼ��
*  
*/
#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f10x_it.h"     //�����û�MCU�����޸�


#define uchar      unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long



//////COMMAND TABLE//////////

//�������
#define CMD_X_AXIS	0                  //X��ѡ��
#define CMD_Y_AXIS	1                  //Y��ѡ��
#define CMD_Z_AXIS  2                   //Z��ѡ��
#define CMD_A_AXIS  3                   //A��ѡ��
#define CMD_B_AXIS  100                 //B��ѡ��
#define CMD_Clear   4                     //����
#define CMD_Return_WorkPiece_Zero 5     //�ع�����
#define CMD_Divided	 6                   //����
#define CMD_Override_Change	7           //�����л�
#define CMD_Spin_On_Off	8               //���Ὺ��
#define CMD_Auto_knife	9               //�Զ��Ե�
#define CMD_Return_Machine_Zero 10      //�ػ�е��
#define CMD_ 11
#define CMD_Start 12                    //��ʼ�ӹ�
#define CMD_1	13
#define CMD_2	14
#define CMD_EStop	15                    //����ֹͣ
//#define CMD_Stop  16
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
#define CMD_Auto_Knife_Mode  29          //�Զ��Ե�ģʽ
#define CMD_Uint_Mode  30               //��λģʽ
#define CMD_Voice_Switch  31            //��������
#define CMD_Sure   32                   //ȷ��
#define CMD_Cancel 33                   //ȡ��

#define CMD_ASK_SLAVE 29                 //������������
#define CMD_RPY_HC_MPG1	36               //������ȡ����
#define CMD_UPDATE_MACH3_NUMBER	41       //������������

// ����1��ʼ��
void Usart1_Init(uint32 Baudrate);


//����2��ʱ�ӳ�ʼ������    A2,A3    
void Usart3_Init(uint32 BaudRate);

//RS485 ģʽ����.en:0,����;1,����.
void RS485_TX_Set(uint8 en);


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
 void Usart3_send_Str(uint8 buf[]);

// ��������ǰ������һ��У�����	
char SetXor	(char length, char start);

//�������ݺ�����һ�����������ǰ������Աȣ�һ�µĻ�����յ�������ȷ
unsigned char CheckXor (char data, unsigned char len);

//��ַУ��
uint8_t Check_Address (char data);

//����2�ж�ִ�к���  //place at ISR
void Usart3_Recieve_ISR_Process (void);

//����2�������ݴ�����
void Usart3_Data_handle (void);

// ����2�������ݺ���
void Usart3_Send_Data (uint8 length);

//�ж������Ƿ����仯
uint8 Check_Pulses_change(void);

//�жϰ����Ƿ����仯
uint8 Check_CMD_button_change(void);

//�жϱ����Ƿ����仯
uint8 Check_Override_change(void);

//����ָ�������
void Create_CMD_and_Date(void);

#endif
