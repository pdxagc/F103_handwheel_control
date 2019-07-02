/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   ulitity.c
** �޸�ʱ��:   2018-05-18
** �ļ�˵��:   �û�MCU��������������
** ����֧�֣�  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------*/
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "ulitity.h"
#include "string.h"


#define TIME_100MS 10                                                 //100����(10����λ)
#define voice_on   0
#define voice_off  1

uint8 get_cmd_type;                      //��ȡָ������
uint8 get_ctrl_msg;                      //��ȡ��Ϣ������
uint8 get_button_state;                  //��ȡ��ť״̬
uint8 get_control_type;                  //��ȡ�ؼ�����
uint16 get_screen_id;                    //��ȡ����ID
uint16 get_control_id;                   //��ȡ�ؼ�ID
uint32 get_value;                        //��ȡ��ֵ
uint8  input_buf[20];                    //������������

uint8 Voice_Control=0;                   //�������ƣ�����֮ǰ�Ŀ���״̬����0��ʹ�ܣ�1����ֹ
uint8 voice_button;                      //������ť
uint8 Setting_Sure;                      //����ҳ��ȷ����ť״̬
uint8 Setting_concel;                    //����ҳ��ȡ����ť״̬

volatile uint32  timer_tick_count = 0;                                //��ʱ������

uint8  cmd_buffer[CMD_MAX_SIZE];                                      //ָ���
static uint16 current_screen_id = 0;                                  //��ǰ����ID
static uint8 Work_state=0;                                            //��������״̬���
static uint8 WiFi_state=0;                                            //WiFI״̬���
static uint8 BT_state=0;                                              //����״̬���
static uint8 Account_state=0;                                         //�˻�����״̬���
uint8 Work_Page_Status = 0;                                           //����ҳ����
uint8 Coordinate_Change_Counter=0;                                    //�����л���ť��������
uint8 Override_Change_Counter=0;                                      //�����л���ť��������
uint32 Progress_Value = 0;                                            //��������ֵ 

float  X1=0,X2=0;                                                     //X�����꣬X1:�������꣬X2:��е����
float  Y1=0,Y2=0;
float  Z1=0,Z2=0;
float  A1=111.1,A2=111.1;
float  B1=111.1,B2=111.1;
uint8 X1buf[9],X2buf[8],Y1buf[9],Y2buf[9],Z1buf[9],Z2buf[9],A1buf[9],A2buf[9],B1buf[9],B2buf[9];
uint16 Spindle_Speed=20000;                                            //�������ת��
int Spindle_Speed_Value=100;                                           //�����ʼ�ٶȰٷֱ�
int Work_Speed=1000;                                                     //�ӹ���ʼ�ٶ�
int Work_Speed_Value=100;                                              //�ӹ���ʼ�ٶȰٷֱ�



/*! 
*  \brief  �ӹ�ҳ���������ݸ���
*/ 
void UpdateWorkPageTextWord(void)
{
	//�ӹ�ҳ�湤������
	SetTextValue(0,16,X1buf);
	SetTextValue(0,17,Y1buf);
	SetTextValue(0,18,Z1buf);
	SetTextValue(0,19,A1buf);
	SetTextValue(0,20,B1buf);
	//�ӹ�ҳ���е����
	SetTextValue(0,31,X2buf);
	SetTextValue(0,35,Y2buf);
	SetTextValue(0,36,Z2buf);
	SetTextValue(0,37,A2buf);
	SetTextValue(0,38,B2buf);	
	}

	//��������������ݸ���
void UpdateControlPanelTextWord(void)
{	
	//������幤������
	SetTextValue(2,38,X1buf);
	SetTextValue(2,39,Y1buf);
	SetTextValue(2,40,Z1buf);
	SetTextValue(2,41,A1buf);
	SetTextValue(2,42,B1buf);
	//������幤������
	SetTextValue(2,17,X2buf);
	SetTextValue(2,18,Y2buf);
	SetTextValue(2,19,Z2buf);
	SetTextValue(2,20,A2buf);
	SetTextValue(2,21,B2buf); 			                                                                                                                                                          
}

//�ӹ����������ٶȺͼӹ��ٶȰ�ť����
void Spindle_and_Work_Speed_Key_Process(uint8 control_id )
{
	char buf[6];
	uint32 Speed1=0;
	int16 Speed2=0;
	if(control_id==3)                  //�����ٶȼ�
	{
			Spindle_Speed_Value -= 10; 
			if(Spindle_Speed_Value <= 0)
			{
					Spindle_Speed_Value = 0;
			}
      Speed1=Spindle_Speed*Spindle_Speed_Value/100;
			sprintf(buf,"%d",Speed1);
		  SetTextValue(0,24,(uchar *)buf);                //�޸��ı�����ֵ
      //SetTextInt32(0,24,Speed1,0,5);		               
			SetProgressValue(0,1,Spindle_Speed_Value);      //�޸Ľ�������ֵ
	}
	if(control_id==4)                 //�����ٶȼ�
	{
			Spindle_Speed_Value += 10;
			if(Spindle_Speed_Value >= 100)
			{
					Spindle_Speed_Value = 100;
			}
			Speed1=Spindle_Speed*Spindle_Speed_Value/100;
			sprintf(buf,"%d",Speed1);
		  SetTextValue(0,24,(uchar *)buf);              //�޸��ı�����ֵ
      //SetTextInt32(0,24,Speed1,0,5);
			SetProgressValue(0,1,Spindle_Speed_Value);    //�޸Ľ�������ֵ
	}
	if(control_id==5)                //�ӹ��ٶȼ�
	{
		if(Work_Speed_Value <= 100)
		{
			Work_Speed_Value -=10;
			if(Work_Speed_Value <= 0)
			{
				Work_Speed_Value=0;
			}
			SetTextInt32(0,39,Work_Speed_Value,0,2);          //�޸��ı�����ֵ
		}
		else if(100 < Work_Speed_Value && Work_Speed_Value <=300)
		{
			Work_Speed_Value  -=25;	
			SetTextInt32(0,39,Work_Speed_Value,0,3);         //�޸��ı�����ֵ						
		}
		Speed2=Work_Speed*Work_Speed_Value/100;
    sprintf(buf,"%d",Speed2);
		SetTextValue(0,25,(uchar *)buf);                    //�޸��ı�����ֵ
		SetProgressValue(0,2,Work_Speed_Value);            //�޸Ľ�������ֵ
		
	}
	if(control_id==6)               //�ӹ��ٶȼ�
	{
		if(Work_Speed_Value < 100)
		{
			Work_Speed_Value +=10;
			SetTextInt32(0,39,Work_Speed_Value,0,2);          //�޸��ı�����ֵ
		}
		else if(100 <= Work_Speed_Value && Work_Speed_Value <=300)
		{
			Work_Speed_Value +=25;
			if(Work_Speed_Value >= 300)
			{
				Work_Speed_Value=300;
			}	
			SetTextInt32(0,39,Work_Speed_Value,0,3);          //�޸��ı�����ֵ						
		}
		Speed2=Work_Speed*Work_Speed_Value/100;
    sprintf(buf,"%d",Speed2);
		SetTextValue(0,25,(uchar *)buf);                    //�޸��ı�����ֵ
		SetProgressValue(0,2,Work_Speed_Value);             //�޸Ľ�������ֵ
	}
}

//����ҳ��������ʾ��ť����
void Speaker_Key_Process(uint8  Button_state)
{
	if(Button_state==voice_off)        //���������������EE 93 64 FF FC FF FF 
	{
		 SpeakerControl(0);
		 Voice_Control=voice_off;
	}
	else                               //������������0��EE 93 00 FF FC FF FF 
	{
		 SpeakerControl(100);
     Voice_Control=voice_on;	
	}
}
//������������л�����
void Coordinate_Change_Process(void)
{
	Coordinate_Change_Counter++;
	switch(Coordinate_Change_Counter)
	{
		case 1:
		{
			SetTextValue(0,27,"G54");        //�����ı�ֵ
			SetTextValue(2,30,"G54");       
			break;  
		}
		case 2:
		{
			SetTextValue(0,27,"G55");
			SetTextValue(2,30,"G55");
			break;
		}
		case 3:
		{
			SetTextValue(0,27,"G56");
			SetTextValue(2,30,"G56");
			break;
		}
		case 4:
		{
			SetTextValue(0,27,"G57");
			SetTextValue(2,30,"G57");
			break;
		}
		case 5:
		{
			SetTextValue(0,27,"G58");
			SetTextValue(2,30,"G58");
			break;
		}
		case 6:
		{
			SetTextValue(0,27,"G59");
			SetTextValue(2,30,"G59");
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
			break;  
		}
		case 2:
		{
			SetTextValue(0,29,"X2");
			SetTextValue(2,32,"X2");
			break;
		}
		case 3:
		{
			SetTextValue(0,29,"X3");
			SetTextValue(2,32,"X3");
			break;
		}
		case 4:
		{
			SetTextValue(0,29,"X5");
			SetTextValue(2,32,"X5");
			break;
		}
		case 5:
		{
			SetTextValue(0,29,"X10");
			SetTextValue(2,32,"X10");
			break;
		}
		case 6:
		{
			SetTextValue(0,29,"X20");
			SetTextValue(2,32,"X20");
			Override_Change_Counter=0;
			break;
		}			
	}		
}

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
//������������
void Power_On_Set(void)
{
	uint8 i=0;
	char buf[6];
	SetScreen(17);           //��������
	delay_ms(10); 
	SetScreen(17);           //��������
	delay_ms(10);	
	for(i=0;i<101;i++)
	{
		SetProgressValue(17,3,i);      //���ý�������ֵ
		delay_ms(20);                  //��ʱ0.05��
	}
	
	SetScreen(0);           //�л����ӹ�ҳ��
	delay_ms(10);
	
	//�����󣬼ӹ�ҳ���������
	
	for(i=16;i<21;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //�������������ı�ֵ
	}
	for(i=34;i<39;i++)
	{
		SetTextValue(0,i,"000000.00"); 	   //	��е���������ı�ֵ
	}
	
	SetTextValue(0,27,"G59");        //���õ�ǰ����ֵ��G54
	SetTextValue(2,30,"G59");
	SetTextValue(0,28,"��");         //���������������ء�
	SetTextValue(2,31,"��");
	SetTextValue(0,29,"X0");         //���ñ��ʣ�X1
	SetTextValue(2,32,"X0");
	SetProgressValue(0,1,100);        //���������ʼ���ٶȣ�100%
	sprintf(buf,"%d",Spindle_Speed);
	SetTextValue(0,24,(uchar *)buf);
	
	SetProgressValue(0,2,100);	      //���üӹ���ʼ���ٶȣ�100%	
	SetTextInt32(0,39,100,0,3);       //���üӹ���ʼ���ٶ��ı�ֵ��100
	SetTextInt32(0,25,1000,0,2);      //��ʾ�ӹ���ʼ���ٶ�Ϊ100%ʱ���ٶ�
	
	SetButtonValue(2,13,0);          //��ʼ��ť���ɿ�״̬
	SetButtonValue(2,14,1);          //ֹͣ��ť�ǰ���״̬

}
/******************************************************************************************************/
/*!                                                                                 
*  \brief  �������                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
{  
	qsize  size = 0;        //ָ��� 
	
	Set_System();           //����ʱ��                                                                                                                                 
	Interrupts_Config();    //���ô����ж�                                                                                                                             
	systicket_init();       //����ʱ�ӽ���                                                                                                          
	UartInit(115200);       //���ڳ�ʼ��                                                                                                                      
	queue_reset();          //��մ��ڽ��ջ�����                                                                                                
	delay_ms(300);          //��ʱ�ȴ���������ʼ�����,����ȴ�300ms  
 
	Power_On_Set();         //������������
	Work_Page_Status=Working_Page;
	
	while(1)                                                                        
	{   			
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);                  //�ӻ������л�ȡһ��ָ��õ�ָ���       

		if(size>0&&cmd_buffer[1]!=0x07)                                  //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
		{                                                                           
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);                   //ָ��������� ���ж�����������ĸ�Work_Page_Status
		}                                                                           
		else if(size>0&&cmd_buffer[1]==0x07)                             //���Ϊָ��0x07��������STM32  
		{                                                                           
			__disable_fault_irq();                                                   
			NVIC_SystemReset();                                                                                                                                          
		}
	 
		
		switch(Work_Page_Status)
		{
			//********�ӹ�ҳ��********
			case Working_Page:       
			{
				switch(get_control_type)                                     //�ؼ������ж�
				{
					case kCtrlButton:                                          //��ť�ؼ�,kCtrlButton=0X10
					{
						if(get_button_state)
						{
							Spindle_and_Work_Speed_Key_Process(get_control_id);
							get_button_state=0;							
						}							                     
					}break;						
					case kCtrlText:                                           //�ı��ؼ�
					{
					
					}	break;                        							                                                                                                                                                                                  
					default:						
							 break;
				}			
			}break;
			//********����ҳ��********
		  case Setting_page:  
			{
				switch(get_control_type)                                    //�ؼ������ж�
				{
					case kCtrlButton:                                         //��ť�ؼ�
					{
						if(get_control_id==27)                                //������ʾ��ť����     
						{
							if(get_button_state)                                
								voice_button=voice_off;                           //��ť���£��������ѹ�
							else
								voice_button=voice_on;                            //��ť�ɿ����������ѿ�													
						}
						if(get_control_id==4)                                //ȷ����ť����
						{
												
						}
						if(get_control_id==5)                                //ȡ����ť����
						{
							if(Voice_Control==voice_off)
								SetButtonValue(1,27,1);
							else 
								SetButtonValue(1,27,0);							
						}
						
					}break;						
					case kCtrlText:                                                       //�ı��ؼ�
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				 }
			
			}
			break;
			//********��ʾ�Ƿ񱣴��������*********
			case Save_Pram_Page:      
			{
				switch(get_control_type)                              //�ؼ������ж�
				{
					case kCtrlButton:                                   //��ť�ؼ�
					{
						if(get_control_id==1)                           //ȷ����ť����
						{
							Speaker_Key_Process(voice_button);            //������ʾ��ť����						
						}
						if(get_control_id==2)                           //ȡ����ť����
						{
							if(Voice_Control==voice_off)
								SetButtonValue(1,27,1);
							else 
								SetButtonValue(1,27,0);						
						}
					}
					case kCtrlText:                                                       //�ı��ؼ�
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				}	
			}break;
			//********�������ҳ��***********
			case ControlPanel_Page: 
			{
				switch(get_control_type)                              //�ؼ������ж�
				{
					case kCtrlButton:                                   //��ť�ؼ�
					{							
						if(get_button_state)                            //��ť����
						{
							if(get_control_id==7)                         //�����л���ť����
							{			
								Coordinate_Change_Process();		            //�����л�����		 
							}
							if(get_control_id==3)                         //�����л���ť����
							{
								Override_Change_Process();                  //�����л�����			
							} 
														
						}	
						switch(get_control_id)
						{
							case 13:                        //��ʼ��ť����
							{
								Work_state=1;
								SetButtonValue(2,14,0);                    //ֹͣ��ť�ɿ�״̬
								if(get_button_state==0)
								{
									delay_ms(200);
									SetButtonValue(2,13,1);                    //��ʼ��ť����״̬
								}							
								
							}break;
							case 14:                       //ֹͣ��ť����
							{
								Work_state=0;
								SetButtonValue(2,13,0);                    //��ʼ��ť�ɿ�״̬
								if(get_button_state==0)
								{
									delay_ms(200);
									SetButtonValue(2,14,1);                    //ֹͣ��ť����״̬
								}								
								
							}break;
							case 15:                       //��λ��ť����
							{
								Work_state=0;
								SetButtonValue(2,13,0);                    //��ʼ��ť�ɿ�״̬
								SetButtonValue(2,14,1);                    //ֹͣ��ť����״̬						
							}break;
					 }
					}break;						
					case kCtrlText:                                                       //�ı��ؼ�
					{
					
					}break;                                                                                                                                                                                  
					default:
							 break;
				}
				
			}break;
			//
			case Return_WorkPiece_Zero_Page: 
				break;
			case Jump_Work_Page:
				break;
			case File_Manage_Page: 
				break;
			case Leading_In_Page: 
				break;
			case Delete_Page: 
				break;
			case Storage_View_Page: 
				break;
			//********�������˻���¼����**********
			case Net_Account_Manage_Page: 
				{
					switch(get_control_type)                              //�ؼ������ж�
					{
						case kCtrlButton:                                   //��ť�ؼ�
						{							
													
							
						}break;						
						case kCtrlText:                                     //�ı��ؼ�
						{
							switch(get_control_id)
							{
								case 7:
								{
									SetTextValue(11,8,(uchar *)input_buf);
								
								}
								case 8:
								{
								
								}
								case 9:
								{
								
								}
							
							
							}
							 
						
						}break;						
								// NotifyText(get_screen_id,get_control_id,get_value);                       
								                                                                                                                                                                                  
						default:
								 break;
					}
				
				
				}
				break;
			case Choose_WiFi_Page: 
				break;
			case Disconnet_and_SignIn_Page: 
				break;
			case Disconnect_and_SignOut_Page: 
				break;
			case Disconnect_Remind_Page: 
				break;
			case SignOut_Remind_Page: 
				break;
			case Leading_Out_Pgae:
				break;
			default:
				break;
	 }

		
		
		if(Work_state)               //�ӹ�״̬����ʼ
		{
			WorkingStatus_Starting();       
		}
		else                         //�ӹ�״̬��ֹͣ
		{
			WorkingStatus_Stoped();  
		}
		
		//    �ر�ע��
		//    MCU��ҪƵ���򴮿����������ݣ����򴮿������ڲ��������������Ӷ��������ݶ�ʧ(��������С����׼��8K��������4.7K)
		//    1) һ������£�����MCU�򴮿����������ݵ����ڴ���100ms���Ϳ��Ա������ݶ�ʧ�����⣻
		//    2) �����Ȼ�����ݶ�ʧ�����⣬���жϴ�������BUSY���ţ�Ϊ��ʱ���ܷ������ݸ���������

		//����Ϊ����ʾ���㣬ÿ20�����Զ�����һ��
		//��һ���ж�10ms��timer_tick_countֵ+1,100*timer_tick_count = 1s				
	}  
}




/***
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
//	  uint8 *Just_get_value;
	
	  char input_string;
    get_cmd_type = msg->cmd_type;                                         //��ȡָ������
    get_ctrl_msg = msg->ctrl_msg;                                         //��ȡ��Ϣ������
	  get_button_state= msg->param[1];                                       //��ȡ��ť״̬
    get_control_type = msg->control_type;                                 //��ȡ�ؼ�����
    get_screen_id = PTR2U16(&msg->screen_id);                             //��ȡ����ID
    get_control_id = PTR2U16(&msg->control_id);                           //��ȡ�ؼ�ID
    //get_value = PTR2U32(msg->param);                                      //��ȡ��ֵ
//	  Just_get_value = msg->param; 	  
//	  SetTextValue(11,6,Just_get_value);

	  sprintf(input_buf,"%s",msg->param);
	   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //�ؼ�����֪ͨ
    {  
//			if(get_ctrl_msg==MSG_GET_CURRENT_SCREEN)                            //����ID�仯֪ͨ,�������л�Work_Page_Status
//			{
				switch(get_screen_id)                                             //�����л�
				{
					case 0: Work_Page_Status=Working_Page;break;
					case 1: Work_Page_Status=Setting_page;break;
					case 2: Work_Page_Status=ControlPanel_Page;break;
					case 3: Work_Page_Status=Return_WorkPiece_Zero_Page;break;
					case 5: Work_Page_Status=Jump_Work_Page;break;
					case 6: Work_Page_Status=File_Manage_Page;break;
					case 7: Work_Page_Status=Leading_In_Page;break;
					case 9: Work_Page_Status=Delete_Page;break;
					case 10: Work_Page_Status=Storage_View_Page;break;
					case 11: Work_Page_Status=Net_Account_Manage_Page;break;
					case 12: Work_Page_Status=Choose_WiFi_Page;break;
					case 13: Work_Page_Status=Disconnet_and_SignIn_Page;break;
					case 14: Work_Page_Status=Disconnect_and_SignOut_Page;break;
					case 15: Work_Page_Status=Disconnect_Remind_Page;break;
					case 16: Work_Page_Status=SignOut_Remind_Page;break;
					case 20: Work_Page_Status=Leading_Out_Pgae;break;
					case 21: Work_Page_Status=Save_Pram_Page;break;
					default: break;						
  			}
        
     }
}


/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param str �ı��ؼ�����
*/
void NotifyText(uint16 screen_id, uint16 control_id, uint32 str)
{
    if(screen_id==11)                                                                 //����ID2���ı����ú���ʾ
    {                                                                            
        int32 value=0;                                                            
        //sscanf(str,"%ld",&value);                                                    //���ַ���ת��Ϊ���� 
			  value=str;
        if(control_id==7)                                                            //��ߵ�ѹ
        {                                                                         
//            //�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�                             
//            if(value<0)                                                              
//            {                                                                        
//                value = 0;                                                            
//            }                                                                        
//            else if(value>380)                                                       
//            {                                                                        
//                value = 380;                                                           
//            }                                                                        
            SetTextInt32(11,8,value,0,4);                                             //������ߵ�ѹ
            //SetTextInt32(4,5,value/2,1,1);                                           //������ߵ�ѹ/2
        }                                                                         
    }                                                                            
}                                                                                

