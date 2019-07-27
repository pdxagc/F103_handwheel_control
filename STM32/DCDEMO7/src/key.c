#include "key.h"
#include "stm32f10x_gpio.h"
#include "hmi_user_uart.h"
#include "tft.h"
#include "ulitity.h"


extern uint8 Press_button;
extern Control_Panel_Pram control_panel_pram;      //�������������ز����Ľṹ�����


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

}


//����ɨ�躯��
void Key_scan(void)
{
  uint8 Last_time_Axis_mode;
	
	if(key_X == 0||key_Y == 0||key_Z == 0||key_A == 0||key_B == 0)  //�а�������
	{
		
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


}

