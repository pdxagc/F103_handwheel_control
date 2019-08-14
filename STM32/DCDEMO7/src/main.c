
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "ulitity.h"
#include "string.h"
#include "flash.h"
#include "timer.h"
#include "exti.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "tft.h"
#include "malloc.h"
#include "key.h"
#include "iic.h"
#include "24c02.h"




volatile uint32  timer_tick_count = 0;               //��ʱ������

uint8 TX_Data[30]; //the sending package    �洢����3Ҫ���͵����ݣ��������ͣ�
uint8 RX_Data[30]; //the receiving package  ���մ���3���������ݣ����ж��н��գ�
uint8 Pulses_check=1;              //����ͬ��������־λ


uint8 Mark_10ms=0;                 //10ms��ʱ���λ
uint8 Mark_20ms=0;                 //20ms��ʱ���λ
uint8 Mark_30ms=0;                 //30ms��ʱ���λ
uint8 Mark_200ms=0;                //200ms��ʱ���λ
uint8 Mark_500ms=0;                //500ms��ʱ���λ

uint8 Send_Estop_to_handwheel=1;   //�����ַ��ͽ���ֹͣ��Ϣ��1�����ͣ�0��������
uint8 Clear_Estop_massage=0;       //������ֽ���ֹͣ��Ϣ��1�������0�������

uint8 Estop_button=Estop_Off;      //����ֹͣ��ť
uint8 Estop_Press_time=0;          //������ť���´���ͳ��
uint8 Light_mark_button;           //����������� 
uint8 last_time_light_mark_button; //��¼�ϴα��ⴥ�����ֵ

uint16 master_ask=0,master_send=0;

/******************************************************************************************************/
/*!                                                                                 
*  \brief  �������                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
 { 
	char buf4[20];
  uint16 time_conuter=0;	
  char bufrec[20],buf1[20];
  uint16 Recdata1,Recdata2,RecPulses;	 
	uint8  check_time=0;      //����ͬ��������
	 
	Set_System();             //����ʱ��                                                                                                                                                                                                                                                              
	systicket_init();         //����ʱ�ӽ���
	Usart1_Init(115200);      //����1��ʼ��(���̻�ͨѶ)
  Usart2_Init(115200);      //����2��ʼ��(��TFT��ͨѶ) 	 
	queue_reset();            //��մ��ڽ��ջ����� 
	TIME2_Init();             //��ʱ��2��ʼ��
	Key_Init();               //������ʼ��	
	//EXTIX_Init();             //�жϳ�ʼ��
	TIME3_Init();             //��ʱ��3��ʼ��(�������ȿ���)
  TIME4_Init();             //��ʱ��4��ʼ��(������������)
  IIC_Init();               //IIC��ʼ��
	delay_ms(300);            //��ʱ�ȴ���������ʼ�����,����ȴ�300ms
   
	//Usart3_Init(115200);      //����3��ʼ��
	
	 
	Power_On_Set();                 //���������������Ͳ�����ʼ������	
	Get_Setting_page_pram();        //��ȡ����������ҳ����ز���ֵ��������flash��	
	Return_last_status();           //�ָ���һ�δ���������ҳ����ز���ֵ

		//    MCU��ҪƵ���򴮿����������ݣ����򴮿������ڲ��������������Ӷ��������ݶ�ʧ(��������С����׼��8K��������4.7K)
		//    1) һ������£�����MCU�򴮿����������ݵ����ڴ���100ms���Ϳ��Ա������ݶ�ʧ�����⣻
		//    2) �����Ȼ�����ݶ�ʧ�����⣬���жϴ�������BUSY���ţ�Ϊ��ʱ���ܷ������ݸ���������
		
	while(Pulses_check)        //����ͬ����������
	{
		
		if(RX_Data[1] == CMD_UPDATE_MACH3_NUMBER)  //���յ���������
		{	
			Recdata1=RX_Data[18];
			Recdata2=RX_Data[19];			 
      RecPulses= (Recdata1<<8)+Recdata2;  //��ȡ����ֵ
      if(TIM4->CNT==RecPulses)
				check_time++;
			else 
				TIM4->CNT=RecPulses;
      if(check_time>5)
        Pulses_check=0;
			
//      sprintf(bufrec,"%u",RecPulses);	
//			SetTextValue(0,23,(uchar *)bufrec);     //��ʾ����ֵ			
		}
	}

   

//   while(AT24CXX_Check())   //���24C02�Ƿ�����
//	 {
//	   SetTextValue(0,26,(uchar *)"���ʧ��");	 
//	 }
//	   SetTextValue(0,26,(uchar *)"���ɹ�");
	//event generator
	// event queue 
	// StateMachine
		
	while(1)                                                                        
	{				
			
		
		Pulses_Count_Process();               //������������	 
		TFT_Page_handle();                    //������벻ͬ�Ĺ���ҳ��,�����������	
		Communication_Data_handle();          //����1����̻����ݣ����ݽ��մ���
		
		if(Mark_10ms)
		{	
      			
			Estop_button_process();               //������ť���������
			TFT_command_analyse();               //����TFT�������������ʲô��ť �
		  Mark_10ms=0;
			sprintf(buf4,"master_ask:%u",master_ask);	
			SetTextValue(3,29,(uchar *)buf4);
			sprintf(buf4,"master_send:%u",master_send);	
			SetTextValue(3,49,(uchar *)buf4);
		}
		if(Mark_20ms) 
		{
			Key_scan();                         //����������ɨ��
			
      light_Control_Process();             //�������ȿ���		
			Mark_20ms=0;
		}
		if(Mark_200ms)                         //��ʱ��200ms
		{
			
			TFT_Show_coordanate_value();         //��ʾ��������ͻ�е���� 
			Work_state_control();                //��̻�����״̬��ʾ��δ����������WiFi״̬��
			Mark_200ms=0;
			
			//time_conuter++;
			     
//			Usart1_Send_Data(10);
//			if(Start_Download)
//			{
//				time_conuter++;
//				if(time_conuter==1)
//				{
//					time_conuter=0;
//					Download_Per++;
//				}
//		  }
		}	

				
		
	}
	
}

