#include "tft.h"
#include "hmi_user_uart.h"
#include "stdio.h"
#include "hmi_driver.h"
#include "flash.h"
#include "ulitity.h"
#include "cJSON.h"
#include "stdlib.h"
#include "timer.h"


extern uint8  Work_Page_Status;
extern int32 Pulses_counter;              // ������������
extern Override override;
extern Speed_Control Speed;
extern Pram_Status pram_status;
extern Control_Panel_Pram control_panel_pram;
extern Return_Workpiece_Zero return_workpiece_zero;
extern Devide_Set devide_set;  

uint8 Coordinate_Change_Counter=0;                                    //�����л���ť��������
uint8 Override_Change_Counter=1;                                      //�����л���ť��������

//��ʾ�����Ѿ�ֹͣ�ӹ�
void WorkingStatus_Stoped(void)
{
	AnimationPlayFrame(0,11,0);
	AnimationPlayFrame(1,28,0);
	AnimationPlayFrame(2,43,0);
	AnimationPlayFrame(6,33,0);
	AnimationPlayFrame(10,5,0);
	AnimationPlayFrame(11,13,0);
	AnimationPlayFrame(13,9,0);
	AnimationPlayFrame(14,12,0);
}
//��ʾ�������ڼӹ�
void WorkingStatus_Starting(void)
{
	AnimationPlayFrame(0,11,1);
	AnimationPlayFrame(1,28,1);
	AnimationPlayFrame(2,43,1);
	AnimationPlayFrame(6,33,1);
	AnimationPlayFrame(10,5,1);
	AnimationPlayFrame(11,13,1);
	AnimationPlayFrame(13,9,1);
	AnimationPlayFrame(14,12,1);
	
}

//��X�Ṥ����������
void X_coordinate_clear(void)
{
  SetTextValue(2,17,"000000.00");
	SetTextValue(0,16,"000000.00");
}

//��Y�Ṥ����������
void Y_coordinate_clear(void)
{
	SetTextValue(2,18,"000000.00");
	SetTextValue(0,17,"000000.00");
  
}

//��Z�Ṥ����������
void Z_coordinate_clear(void)
{
  SetTextValue(2,19,"000000.00");
	SetTextValue(0,18,"000000.00");
}

//��A�Ṥ����������
void A_coordinate_clear(void)
{
  SetTextValue(2,20,"000000.00");
	SetTextValue(0,19,"000000.00");
}

//��B�Ṥ����������
void B_coordinate_clear(void)
{
  SetTextValue(2,21,"000000.00");
	SetTextValue(0,20,"000000.00");
}

//�������Ṥ����������
void All_Workpiece_coordinate_clear(void)
{
	uint8 i=0;
  for(i=16;i<21;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //�������������ı�ֵ
	}
	for(i=17;i<22;i++)
	{
		SetTextValue(2,i,"000000.00"); 	   //�������������ı�ֵ
	}
}

//XYZAB���갴ť��λ
void XYZAB_button_reset(void)
{
	uint8 i;
	for(i=22;i<27;i++)
	{
		SetButtonValue(2,i,0);           
	}
}

//������������
void Power_On_Set(void)
{
	uint8 i=0;
	char buf[6];
	SetScreen(17);           //�л�����������
	delay_ms(10); 
	SetScreen(17);           //�л�����������
	delay_ms(10);	
	for(i=0;i<101;i++)
	{
		SetProgressValue(17,3,i);     //���ý�������ֵ
		delay_ms(5);                  //��ʱ0.05��
	}
	
	SetScreen(0);           //�л����ӹ�ҳ��
	delay_ms(10);
	
  All_Workpiece_coordinate_clear();//�����й�����������
	
	for(i=34;i<39;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //	��е���������ı�ֵ
		delay_ms(5);
	}
	for(i=38;i<43;i++)
	{
		SetTextValue(2,i,"000000.00"); 	   //�������������ı�ֵ
		delay_ms(5);
	}
	for(i=22;i<27;i++)
	{
		SetButtonValue(2,i,0);            //XYZAB�Ḵλ
		delay_ms(5);
	}
	
	SetTextValue(0,27,"G54");        //���õ�ǰ����ֵ��G54
	SetTextValue(2,30,"G54");
	SetTextValue(0,28,"��");         //���������������ء�
	SetTextValue(2,31,"��");
	SetTextValue(0,29,"X1");         //���ñ��ʣ�X1
	SetTextValue(2,32,"X1");
	
	Speed.Initial_Spindle_Speed=20000;                     //�����ʼת��
	Speed.Initial_Spindle_Speed_Percent=100;               //�����ʼ�ٶȰٷֱ�
	Speed.Initial_Work_Speed=1000;                         //�ӹ���ʼ�ٶ�
	Speed.Initial_Work_Speed_Percent=100;                  //�ӹ���ʼ�ٶȰٷֱ�
	Speed.Changed_Spindle_Speed=20000;                     //�仯�������ת��              
	Speed.Changed_Work_Speed=1000;                         //�仯��ļӹ��ٶ�
	
	SetProgressValue(0,1,Speed.Initial_Spindle_Speed_Percent);  //�����ٶȽ�������ʼ����100%
	sprintf(buf,"%d",Speed.Initial_Spindle_Speed);
	SetTextValue(0,24,(uchar *)buf);                            //�����ٶ��ı����ʼ����20000
	

	SetProgressValue(0,2,Speed.Initial_Work_Speed_Percent);	     //�ӹ��ٶȽ�������ʼ����100%
	sprintf(buf,"%d",Speed.Initial_Work_Speed_Percent);
	SetTextValue(0,39,(uchar *)buf);                             //�ӹ��ٶȽ�������ʼ����100%	      
	sprintf(buf,"%d",Speed.Initial_Work_Speed);
	SetTextValue(0,25,(uchar *)buf);                             //�ӹ��ٶ��ı����ʼ����100%
	   
	
	SetButtonValue(2,13,0);          //��ʼ��ť���ɿ�״̬
	SetButtonValue(2,14,1);          //ֹͣ��ť�ǰ���״̬
	override.Override_num=0.01;       //����Ĭ�ϱ���
	Work_Page_Status=Working_Page;    //��������ӹ�ҳ��
	


}

//����ҳ�漸������ֵ��ȡ���ϵ籣����flash��
void Setting_page_pram_get(void)
{
	pram_status.Voice_last_status=FlashRead(START_ADDR1);
	pram_status.voice_button_status=pram_status.Voice_last_status;            //������������ģʽ
	pram_status.Safe_Z_last_status=FlashRead(START_ADDR2);
	pram_status.Safe_Z_button_satus=pram_status.Safe_Z_last_status;           //���ð�ȫZģʽ
	pram_status.Auto_Knife_last_status=FlashRead(START_ADDR3);
	pram_status.Auto_Knife_button_status=pram_status.Auto_Knife_last_status;  //�����Զ��Ե�ģʽ
	pram_status.Unit_Change_last_status=FlashRead(START_ADDR4);
	pram_status.Unit_Change_button_status=pram_status.Unit_Change_last_status; //���õ�λģʽ
}

//�ӹ����������ٶȺͼӹ��ٶȰ�ť����
void Spindle_and_Work_Speed_Key_Process(void)
{
	char buf[10];
  if(Speed.Spindle_Speed_Changed==Changed)
	{
		Speed.Spindle_Speed_Changed=Unchanged;
		sprintf(buf,"%d",Speed.Changed_Spindle_Speed);
		SetTextValue(0,24,(uchar *)buf);                                 //�޸������ٶ��ı�����ֵ		               
		SetProgressValue(0,1,Speed.Initial_Spindle_Speed_Percent);       //�޸������ٶȽ�������ֵ
		
		sprintf(buf,"$xvm:%d",Speed.Changed_Spindle_Speed);              
		Usart_SendString(USART2,(char *)buf);                            //ͨ��USART2���������������ٶ�
	}
	if(Speed.Work_Speed_Changed==Changed)
	{
		Speed.Work_Speed_Changed=Unchanged;
		sprintf(buf,"%d",Speed.Changed_Work_Speed);
		SetTextValue(0,25,(uchar *)buf);                                   //�޸ļӹ��ٶ��ı�����ֵ
		SetProgressValue(0,2,Speed.Initial_Work_Speed_Percent);            //�޸ļӹ��ٶȽ�������ֵ	
		sprintf(buf,"%d",Speed.Initial_Work_Speed_Percent);
		SetTextValue(0,39,(uchar *)buf);
		
		sprintf(buf,"$xfr:%d",Speed.Changed_Work_Speed);
		Usart_SendString(USART2,(char *)buf);                             //ͨ��USART2���������ͼӹ��ٶ�           
		
	}
}

//����ҳ��������ʾ��ť�����������
void Speaker_Key_Process(uint8  state)
{
	if(state==voice_off)        //������������0��EE 93 00 FF FC FF FF
	{
		 SpeakerControl(0);
		 SetButtonValue(1,27,1);
		 pram_status.Voice_last_status=voice_off;
	}
	else                        //���������������EE 93 64 FF FC FF FF 
	{
		 SpeakerControl(100);
		 SetButtonValue(1,27,0);
     pram_status.Voice_last_status=voice_on;	
	}
	FLASH_WriteByte(START_ADDR1,(uint16)pram_status.Voice_last_status);
}
//����ҳ�氲ȫZ��ť�����������
void Safe_Z_process(uint8 state)
{
	if(state)                  //ģʽ2����ť����״̬��
	{
		SetButtonValue(1,1,1);
	  pram_status.Safe_Z_last_status=1;
		Usart_SendString(USART2,"Safe_z:mode2"); 
	}
	else
	{
		SetButtonValue(1,1,0);      //ģʽ1
	  pram_status.Safe_Z_last_status=0;
		Usart_SendString(USART2,"Safe_z:mode1"); 
	}
	FLASH_WriteByte(START_ADDR2,(uint16)pram_status.Safe_Z_last_status);
}
//����ҳ���Զ��Ե���ť�����������
void Auto_Knife_process(uint8 state)
{
  if(state)                  //ģʽ2����ť����״̬��
	{
		SetButtonValue(1,2,1);
	  pram_status.Auto_Knife_last_status=1;
		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	else
	{
		SetButtonValue(1,2,0);      //ģʽ1
	  pram_status.Auto_Knife_last_status=0;
		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	FLASH_WriteByte(START_ADDR3,(uint16)pram_status.Auto_Knife_last_status);
}
//����ҳ�浥λ�л���ť�����������
void Unit_Change_process(uint8 state)
{
	if(state )                  //ģʽ2����ť����״̬:Ӣ�ƣ�
	{
		SetButtonValue(1,3,1);
	  pram_status.Unit_Change_last_status=1;
		Usart_SendString(USART2,"Unit:mode2");
	}
	else
	{
		SetButtonValue(1,3,0);      //ģʽ1
	  pram_status.Unit_Change_last_status=0;
		Usart_SendString(USART2,"Unit:mode1");
	}
  FLASH_WriteByte(START_ADDR4,(uint16)pram_status.Unit_Change_last_status);

}

//�ָ���һ������״̬
void Return_last_status(void)
{
  Speaker_Key_Process(pram_status.Voice_last_status);	        //����ҳ��������ʾ��ť״̬�������
	Safe_Z_process(pram_status.Safe_Z_last_status);             //����ҳ�氲ȫZ��ť�����������
	Auto_Knife_process(pram_status.Auto_Knife_last_status);     //����ҳ���Զ��Ե���ť�����������
	Unit_Change_process(pram_status.Unit_Change_last_status);   //����ҳ�浥λ�л���ť�����������
}

//���浱ǰ����
void Save_Set(void)
{
  Speaker_Key_Process(pram_status.voice_button_status);     //������ʾ��ť������
	Safe_Z_process(pram_status.Safe_Z_button_satus);           //��ȫZ����������
	Auto_Knife_process(pram_status.Auto_Knife_button_status);  //�Զ��Ե���ť�����������
	Unit_Change_process(pram_status.Unit_Change_button_status); //��λ�л���ť�����������

}
//������������л�����
void Coordinate_Change_Process(void)
{
	Coordinate_Change_Counter++;
	switch(Coordinate_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,27,"G55");        //�����ı�ֵ
			SetTextValue(2,30,"G55");       
			break;  
		}
		case 2:
		{
			SetTextValue(0,27,"G56");
			SetTextValue(2,30,"G56");
			break;
		}
		case 3:
		{
			SetTextValue(0,27,"G57");
			SetTextValue(2,30,"G57");
			break;
		}
		case 4:
		{
			SetTextValue(0,27,"G58");
			SetTextValue(2,30,"G58");
			break;
		}
		case 5:
		{
			SetTextValue(0,27,"G59");
			SetTextValue(2,30,"G59");
			break;
		}
		case 6:
		{
			SetTextValue(0,27,"G54");
			SetTextValue(2,30,"G54");
			Coordinate_Change_Counter=0;
			break;
		}	
	}
}
//������屶�ʿ���
void Override_Change_Process(void)
{
	Override_Change_Counter++;	
	switch(Override_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,29,"X1");    //�����ı�ֵ
			SetTextValue(2,32,"X1");
			override.Override_num=0.01;
			break;  
		}
		case 2:
		{
			SetTextValue(0,29,"X2");                                                                                                                                 
			SetTextValue(2,32,"X2");
			override.Override_num=0.02;
			break;
		}
		case 3:
		{
			SetTextValue(0,29,"X5");
			SetTextValue(2,32,"X5");
			override.Override_num=0.05;
			break;
		}
		case 4:
		{
			SetTextValue(0,29,"X10");
			SetTextValue(2,32,"X10");
			override.Override_num=0.1;
			break;
		}
		case 5:
		{
			SetTextValue(0,29,"X20");
			SetTextValue(2,32,"X20");
			override.Override_num=0.2;
			Override_Change_Counter=0;
			break;
		}	
    default:break;		
	}	
  
}

// �ڼӹ�ҳ��Ϳ������ҳ����ʾ����������
void TFT_Show_coordanate_value(void)
{
	char buf1[20];
	sprintf((char *)buf1,"%09.2f",control_panel_pram.X_value); 
	SetTextValue(2,17,(uchar *)buf1);	                                //��ʾX�Ṥ������
	SetTextValue(0,16,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(2,18,(uchar *)buf1);                                //��ʾY�Ṥ������
	SetTextValue(0,17,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(2,19,(uchar *)buf1);                                //��ʾZ�Ṥ������
	SetTextValue(0,18,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.A_value);
	SetTextValue(2,20,(uchar *)buf1);                                //��ʾA�Ṥ������
	SetTextValue(0,19,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.B_value);
	SetTextValue(2,21,(uchar *)buf1);                               //��ʾB�Ṥ������
	SetTextValue(0,20,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.X_Mac_value); 
	SetTextValue(2,38,(uchar *)buf1);	                               //��ʾX���е����
	SetTextValue(0,34,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_Mac_value);
	SetTextValue(2,39,(uchar *)buf1);                                //��ʾY���е����
	SetTextValue(0,35,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_Mac_value);
	SetTextValue(2,40,(uchar *)buf1);                                //��ʾZ���е����
	SetTextValue(0,36,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.A_Mac_value);
	SetTextValue(2,41,(uchar *)buf1);                                //��ʾA���е����
	SetTextValue(0,37,(uchar *)buf1);
	
	sprintf((char *)buf1,"%09.2f",control_panel_pram.B_Mac_value);
	SetTextValue(2,42,(uchar *)buf1);                               //��ʾB���е����
	SetTextValue(0,38,(uchar *)buf1);

}


//��ʾ�ع�����ҳ������������ֵ
void Show_coordinate_on_return_workpiece_zero_page(void)
{
	char buf[20];
  sprintf((char *)buf,"%09.2f",control_panel_pram.X_value);
	SetTextValue(3,9,(uchar *)buf);	          //����������ʾX�Ṥ������
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(3,10,(uchar *)buf);	        //����������ʾY�Ṥ������     	
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(3,11,(uchar *)buf);	       //����������ʾZ�Ṥ������
	
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.A_value);
	SetTextValue(3,12,(uchar *)buf);	       //����������ʾA�Ṥ������
		
	
	sprintf((char *)buf,"%09.2f",control_panel_pram.B_value);
	SetTextValue(3,13,(uchar *)buf);	        //����������ʾB�Ṥ������
}
//����������X������
void Send_X_Coordinate_to_Host(void)
{
	char buf1[20];
	sprintf((char *)buf1,"{\"jogx\":%.2f%s",control_panel_pram.X_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //����������X������
}

//����������Y������
void Send_Y_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogy\":%.2f%s",control_panel_pram.Y_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //����������Y������
}

//����������Z������
void Send_Z_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogz\":%.2f%s",control_panel_pram.Z_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //����������Z������
}

//����������A������
void Send_A_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"joga\":%.2f%s",control_panel_pram.A_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //����������A������
} 

//����������B������
void Send_B_Coordinate_to_Host(void)
{
  char buf1[20];
	sprintf((char *)buf1,"{\"jogb\":%.2f%s",control_panel_pram.B_value,"}\\r\\n");
	Usart_SendString(USART2,(char *)buf1);                           //����������B������
}




//�������岢��ʾX������
void Show_X_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();   //�����������												
	control_panel_pram.X_value=control_panel_pram.Temp_save_Xvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.X_value);
	SetTextValue(2,17,(uchar *)buf2);	        //����������ʾ��������
	SetTextValue(0,16,(uchar *)buf2);         //����������ʾ��������	
}


//�������岢��ʾY������
void Show_Y_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.Y_value=control_panel_pram.Temp_save_Yvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.Y_value);
	SetTextValue(2,18,(uchar *)buf2);
	SetTextValue(0,17,(uchar *)buf2);         //����������ʾ��������

}

//�������岢��ʾZ������
void Show_Z_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.Z_value=control_panel_pram.Temp_save_Zvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.Z_value);
	SetTextValue(2,19,(uchar *)buf2);
	SetTextValue(0,18,(uchar *)buf2);        //����������ʾ��������
	

}

//�������岢��ʾA������
void Show_A_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.A_value=control_panel_pram.Temp_save_Avalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.A_value);
	SetTextValue(2,20,(uchar *)buf2);
	SetTextValue(0,19,(uchar *)buf2);         //����������ʾ��������
	

}

//�������岢��ʾB������
void Show_B_Coordinata(void)
{
	char buf2[20];
	Get_Pulses_num();
	control_panel_pram.B_value=control_panel_pram.Temp_save_Bvalue + Pulses_counter*override.Override_num;
	sprintf((char *)buf2,"%09.2f",control_panel_pram.B_value);
	SetTextValue(2,21,(uchar *)buf2);
	SetTextValue(0,20,(uchar *)buf2);         //����������ʾ��������

}

