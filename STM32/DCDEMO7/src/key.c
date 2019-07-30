#include "key.h"
#include "stm32f10x_gpio.h"
#include "hmi_user_uart.h"
#include "hmi_driver.h"
#include "tft.h"
#include "ulitity.h"
#include "stdio.h"
#include "iic.h"
#include "24c02.h"



extern uint8 Estop_button;         //����ֹͣ��ť
extern uint8 Press_button;         //��¼�ĸ���ť����(��Ҫ�Ѱ������͸���̻�)
extern Control_Panel_Pram control_panel_pram;      //�������������ز����Ľṹ�����
extern uint8 Estop_Press_time;

//������ʼ������
void Key_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTA,PORTCʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;     //PA4  X�ᰴť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);         //��ʼ��GPIOA4
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;    //PA5  Y�ᰴť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);        //��ʼ��GPIOA.5
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;    //PA6  Z�ᰴť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);        //��ʼ��GPIOA.6
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;    //PB0  A�ᰴť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);        //��ʼ��GPIOB.0
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;    //PB1  B�ᰴť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);        //��ʼ��GPIOB.1
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;    //PB12  ��ͣ��ť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PB12���ó���������	  
	GPIO_Init(GPIOB, &GPIO_InitStructure);        //��ʼ��GPIOB12

}


//����ɨ�躯��
void Key_scan(void)
{
	static uint8 public_button_unpressed=1;  //�����ɿ���־  
  uint8 Last_time_Axis_mode;          //��¼��һʱ�����Ǹ��ᴦ��ѡ��״̬
	
	if(public_button_unpressed && (key_X == 0||key_Y == 0||key_Z == 0||key_A == 0||key_B == 0)) //�а�������
	{
		public_button_unpressed=0;
		Last_time_Axis_mode = Axis_Gets();   //��ȡ�ϴ�ѡ����
		delay_ms(10);     //ȥ���� 
		if(key_X == 0)    //X�ᰴť
		{
			control_panel_pram.Axis_press = CMD_X_AXIS;
			if(Last_time_Axis_mode != X_mode)  //��һ�β���X��ѡ��
			{
			  Show_X_Axis_State();   //��ʾX��ѡ��״̬		
			}
		}
		else if(key_Y == 0)  //Y�ᰴť
		{
			control_panel_pram.Axis_press = CMD_Y_AXIS;	
			if(Last_time_Axis_mode != Y_mode)
			{	 
         Show_Y_Axis_State();  //��ʾY��ѡ��״̬
			}
		}
		else if(key_Z == 0)  //Z�ᰴť
		{
			control_panel_pram.Axis_press = CMD_Z_AXIS;	
			if(Last_time_Axis_mode != Z_mode)
			{
			   Show_Z_Axis_State();  //��ʾZ��ѡ��״̬
			}
		}
		else if(key_A == 0)  //A�ᰴť
		{
			control_panel_pram.Axis_press = CMD_A_AXIS;	
			if(Last_time_Axis_mode != A_mode)
			{
			  Show_A_Axis_State();  //��ʾA��ѡ��״̬
			}
		}
		else if(key_B == 0)  //B�ᰴť
		{
			control_panel_pram.Axis_press = CMD_B_AXIS;	
			if(Last_time_Axis_mode != B_mode)
			{
			  Show_B_Axis_State();  //��ʾB��ѡ��״̬
			}
		}		
	}
	else if(key_X && key_Y && key_Z && key_A && key_B)
	{
	  public_button_unpressed=1;
	}

}


//����ֹͣ��ťɨ��
void Estop_Button_Scan(void)
{
//	uint8 Readvalue;
//	char buf[20];
	static uint8 Estop_button_unpressed=1;         //����ֹͣ��ť�ɿ���־λ,1:�ɿ���0������
  if(Estop_button_unpressed && key_Estop == 0)   //��ͣ�ᰴť����
	{
		Estop_button_unpressed=0;
		delay_ms(10);               //ȥ���� 
		if(key_Estop == 0)
		{
			Press_button = CMD_EStop;
			if(Estop_button==Estop_Off)
				Estop_button=Estop_On;
			else
			{
				Estop_button=Estop_Off;
				ClearTextValue(0,26);
			}
			
			//����24c02
//			Estop_Press_time++;
//			AT24CXX_WriteOneByte(0,Estop_Press_time);
//			delay_ms(10);  
//			Readvalue=AT24CXX_ReadOneByte(0);
//			sprintf(buf,"%u",Readvalue);	
//			SetTextValue(0,23,(uchar *)buf);
	  }
	}
	else if(key_Estop == 1)
	{
	  Estop_button_unpressed=1;
	}

}
