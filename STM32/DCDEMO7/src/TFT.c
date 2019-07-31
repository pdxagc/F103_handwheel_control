#include "tft.h"
#include "hmi_user_uart.h"
#include "stdio.h"
#include "hmi_driver.h"
#include "flash.h"
#include "ulitity.h"
#include "cJSON.h"
#include "stdlib.h"
#include "stm32f10x_tim.h"
#include "timer.h"
#include "cmd_queue.h"
#include "malloc.h"
#include "string.h"
#include "24c02.h"


Speed_Control Speed;
Pram_Status pram_status;
Control_Panel_Pram control_panel_pram;
Return_Workpiece_Zero return_workpiece_zero;
Devide_Set devide_set;  

uint8  cmd_buffer[CMD_MAX_SIZE];                     //LCDָ���
uint8  Override_num;
char  Working_line_buf[20];                      //����ӹ�����
uint16 Pulses_counter;                           // ������������

uint8 Coordinate_Change_Counter=0;                      //�����л���ť��������
uint8 Override_Change_Counter=1;                        //�����л���ť��������
uint8 Pulses_count_mark=1;                              //�������ʹ�ܱ��λ
uint8 file_name[20]="�������";    //�ļ���

uint8  Work_Page_Status = 0;             //����ҳ����
uint8  get_cmd_type;                     //��ȡָ������
uint8  get_ctrl_msg;                     //��ȡ��Ϣ������
uint8  get_button_state;                 //��ȡ��ť״̬
uint8  get_control_type;                 //��ȡ�ؼ�����
uint16 get_screen_id;                    //��ȡ����ID
uint16 get_control_id;                   //��ȡ�ؼ�ID
uint32 get_value;                        //��ȡ��ֵ
uint8  input_buf[20];                    //������������

uint8  Press_button=0XFF;          //��¼�ĸ���ť����(��Ҫ�Ѱ������͸���̻�)
uint8 last_time_work_state=0;      //��¼��һ�λ�������״̬,Ĭ���ǹػ�״̬
int32 Working_line;                //�ӹ�����
uint8 first_time_re_workpiece;     //�״ν���ع�����ҳ��


uint8 Cloud_File_Button=0;        //�ƿռ��ļ�ѡ�а�ť
uint8 SD_File_Button=0;           //SD���ļ�ѡ�а�ť
uint8 Download_Per=0;             //�Ѿ����ذٷֱ�
uint8 Start_Download=0;           //��ʼ���ر�־λ��0��û���أ�1����������
uint8 Cloud_Page=1;               //�ƿռ�ҳ��
uint8 Last_time_Cloud_Page=0;
uint8 SD_Page=1;                  //SD��ҳ��
uint8 Last_time_SD_Page=0;
uint8 WiFi_Page=1;
uint8 Last_time_WiFi_Page=0;


uint8 Network_Control=4;   //0:��Ч��1����ʾ��������ؼ���2��������������ؼ���3����ʾ�Ͽ����ӿؼ���4�����ضϿ����ӿؼ�
uint8 Sign_Control=4;      //0:��Ч��1����ʾ������¼�ؼ���2������������¼�ؼ���3����ʾ�˳���¼�ؼ���4�������˳���¼�ؼ�
uint8 WiFi_Password_Right=0;  //����������ȷ
uint8 Account_Password_Right=0;//�˻�������ȷ

State state;                               //��������״̬�ṹ�����
Speed_Control Speed;                       //���������ٶ���صĽṹ�����
Pram_Status pram_status;                   //��������ҳ����ز����Ľṹ�����
Control_Panel_Pram control_panel_pram;      //�������������ز����Ľṹ�����
Return_Workpiece_Zero return_workpiece_zero;//�����ع�������ز����Ľṹ�����
Devide_Set devide_set;                     //��������������ز����Ľṹ�����
Jump_Work_Set jump_work_set;               //�������мӹ���ز����Ľṹ�����

//������TFT����������з���
void TFT_command_analyse(void)
{
	qsize  size = 0;                                             //ָ��� 
  size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);              //���յ�LCD�������ݣ���USART2��ָ�����cmd_buffer�л�ȡһ��ָ��õ�ָ���       
	if(size>0 && cmd_buffer[1]!=0x07)                            //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
	{                                                                           
		Usart2_Receive_data_handle((PCTRL_MSG)cmd_buffer, size);   //ָ��������� �����Ӧ�ý����ĸ�Work_Page_Status�������Ӧ�Ĳ���λ
		memset(cmd_buffer, 0, CMD_MAX_SIZE);                       //��ָ���cmd_buffer����
	}
}

//���������������
void Pulses_Count_Process(void)
{
  if( Work_Page_Status == Working_Page || Work_Page_Status == ControlPanel_Page )  //�����ڼӹ�ҳ����߿������ҳ��ʱ���������������ʼ
	{
//		if(Pulses_count_mark)       //�������ʹ�ܱ��λ��1
//		{
		  TIM_Cmd(TIM4, ENABLE);    //������ʱ��4����ʼ�������
			Pulses_count_mark=0;
//		}
		Get_Pulses_num();          //������������
		
		sprintf(Working_line_buf,"%u",Pulses_counter);  
		SetTextValue(0,22,(uchar *)Working_line_buf);     //��ʾ������� 
	}
	else
	{
		if(Pulses_count_mark==0)
		{
	    TIM_Cmd(TIM4, DISABLE);          //��ֹ TIM4�����岻����
		  Pulses_count_mark=1;
		}
	}
	

}

/************************************************************
*
*��������ǵĹ���ҳ�棬�����������
*������Work_Page_Status �����ֵ�ǰ���ڵĹ���ҳ��
*
*************************************************************/
void TFT_Page_handle(void)
{
	switch(Work_Page_Status)
	{	
		case Working_Page: //*********************************************************�ӹ�ҳ��***************************************************************************************************
		{	
			                           
			Spindle_and_Work_Speed_Key_Process();	           	//�ӹ����������ٶȺͼӹ��ٶȰ�ť����
	
			//SetTextValue(0,21,(uchar *)file_name);          //��ʾ���ڼ��ص��ļ���	

//			sprintf(Working_line_buf,"%d",Pulses_counter);  
//			SetTextValue(0,22,(uchar *)Working_line_buf);     //��ʾ�ӹ���������Ҫ������ѯ��
			
		}break;
		case Setting_page:  //********************************************************����ҳ��*************************************************************************************************
		{
		 if(pram_status.Screen_ID1_Setting_concel)          //ȡ����ť����
		 {
				Return_last_status();                           //�ָ���һ������״̬ 
				pram_status.Screen_ID1_Setting_concel=0;
				Work_Page_Status=Working_Page;				 
		 }		
		}break;
		case Save_Pram_Page:  //****************************************************��ʾ�Ƿ񱣴��������****************************************************************************************
		{
			if(state.Work_state==Stop)                                   //ֹͣ�ӹ�
			{
				if(pram_status.Screen_ID21_Setting_Sure)                   //��ȷ����ť������
				{
					Save_Set();                                              //��������
					Work_Page_Status=Working_Page;
				}
				if(pram_status.Screen_ID21_Setting_concel)                 //ȡ����ť����
				{
					Return_last_status();                                    //�ָ���һ������״̬
					Work_Page_Status=Working_Page;					
				}	
			}				
		} break;	
		case ControlPanel_Page:  //******************************************************�������ҳ��*****************************************************************************************
		{	
			//Get_Pulses_num();                                   //����������� 
			
			//SetTextValue(2,27,(uchar *)file_name);            //��ʾ���ڼ��ص��ļ���	         				
			//sprintf(Working_line_buf,"%d",Pulses_counter);  
			//SetTextValue(2,28,(uchar *)Working_line_buf);     //��ʾ�ӹ���������Ҫ������ѯ��	
			
		}break;
		case Return_WorkPiece_Zero_Page:   //*******************************************�ع�����ҳ��***************************************************************************************
		{ 
			
			if(first_time_re_workpiece)      //�״ν������ҳ��
			{
				SetButtonValue(3,1,0);         //��������ʾΪѡ��״̬()
				SetButtonValue(3,4,0);
				SetButtonValue(3,5,0);
				SetButtonValue(3,6,0);
				SetButtonValue(3,7,0);
				SetButtonValue(3,8,0);
				
				return_workpiece_zero.all_spindle_status=0;   //��������ʾΪѡ��״̬
				return_workpiece_zero.X_clear_status=0;
				return_workpiece_zero.Y_clear_status=0;
				return_workpiece_zero.Z_clear_status=0;
				return_workpiece_zero.A_clear_status=0;
				return_workpiece_zero.B_clear_status=0;
				
				control_panel_pram.Temp_save_Xvalue=control_panel_pram.X_value;   //����Ӧ����ֵ��������
				control_panel_pram.Temp_save_Yvalue=control_panel_pram.Y_value;
				control_panel_pram.Temp_save_Zvalue=control_panel_pram.Z_value;
				control_panel_pram.Temp_save_Avalue=control_panel_pram.A_value;
				control_panel_pram.Temp_save_Bvalue=control_panel_pram.B_value;
				first_time_re_workpiece=0;					
			}
			
			if(return_workpiece_zero.X_clear_status==0 && return_workpiece_zero.Y_clear_status==0 && return_workpiece_zero.Z_clear_status==0 && return_workpiece_zero.A_clear_status==0 && return_workpiece_zero.B_clear_status==0)   //�����ᶼѡ��
			{
				SetButtonValue(3,1,0);              //ȫ�ᰴť��ʾѡ��״̬
			}
			else
			{
				SetButtonValue(3,1,1);              //ȫ�ᰴť��ʾδѡ��״̬
			}
			
			if(return_workpiece_zero.Sure)        //ȷ����ť����
			{
				//*********************************************************************************************
				if(return_workpiece_zero.X_clear_status==0)  //��X������
				{
					Puless_count_clear();
					control_panel_pram.X_value=0;
					control_panel_pram.Temp_save_Xvalue=0;													
				}
				else                                         //X�᲻����
				{
					if(return_workpiece_zero.X_get_value)     //��ȡ��������ֵ
					{
						Puless_count_clear();
						control_panel_pram.X_value = return_workpiece_zero.Re_X_Value;
						control_panel_pram.Temp_save_Xvalue = return_workpiece_zero.Re_X_Value;						
						return_workpiece_zero.X_get_value=0;     //��־λ����
					}
					else                                      //û��������ֵ
					{
						Puless_count_clear();                //��������Ĵ�������	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.Y_clear_status==0) //Y������
				{
					Puless_count_clear();
					control_panel_pram.Y_value=0;
					control_panel_pram.Temp_save_Yvalue=0;						
										
				}
				else                                        //Y�᲻����
				{
					if(return_workpiece_zero.Y_get_value)     //��ȡ��������ֵ
					{
						Puless_count_clear();
						control_panel_pram.Y_value = return_workpiece_zero.Re_Y_Value;
						control_panel_pram.Temp_save_Yvalue = return_workpiece_zero.Re_Y_Value;													
						return_workpiece_zero.Y_get_value=0;
					}
					else
					{
						 Puless_count_clear();                //��������Ĵ�������	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.Z_clear_status==0)  //Z������
				{
					Puless_count_clear();
					control_panel_pram.Z_value=0;
					control_panel_pram.Temp_save_Zvalue=0;												
				}
				else
				{
					if(return_workpiece_zero.Z_get_value)     //��ȡ��������ֵ
					{
						Puless_count_clear();
						control_panel_pram.Z_value = return_workpiece_zero.Re_Z_Value;
						control_panel_pram.Temp_save_Zvalue = return_workpiece_zero.Re_Z_Value;						
			
						return_workpiece_zero.Z_get_value=0;
					}
					else
					{
						Puless_count_clear();                //��������Ĵ�������	
					
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.A_clear_status==0)  //A������
				{
					Puless_count_clear();
					control_panel_pram.A_value=0;
					control_panel_pram.Temp_save_Avalue=0;					
							
				}
				else
				{
					if(return_workpiece_zero.A_get_value)     //��ȡ��������ֵ
					{
						Puless_count_clear();
						control_panel_pram.A_value = return_workpiece_zero.Re_A_Value;
						control_panel_pram.Temp_save_Avalue = return_workpiece_zero.Re_A_Value;								
						return_workpiece_zero.A_get_value=0;
					}
					else
					{
						Puless_count_clear();                //��������Ĵ�������	
					}
				}
				//*********************************************************************************************
				if(return_workpiece_zero.B_clear_status==0)  //B������
				{
					Puless_count_clear();
					control_panel_pram.B_value = 0;
					control_panel_pram.Temp_save_Bvalue = 0;					
							
				}
				else 
				{
					if(return_workpiece_zero.B_get_value)     //��ȡ��������ֵ
					{
						Puless_count_clear();
						control_panel_pram.B_value = return_workpiece_zero.Re_B_Value;
						control_panel_pram.Temp_save_Bvalue = return_workpiece_zero.Re_B_Value;						
						return_workpiece_zero.B_get_value=0;
					}
					else
					{
						Puless_count_clear();                //��������Ĵ�������	
					}
				}
				
				return_workpiece_zero.Sure=0;				
				Work_Page_Status=ControlPanel_Page;
			} 
			if(return_workpiece_zero.Cancel)        //ȡ����ť����
			{
				control_panel_pram.X_value=control_panel_pram.Temp_save_Xvalue;
				control_panel_pram.Y_value=control_panel_pram.Temp_save_Yvalue;
				control_panel_pram.Z_value=control_panel_pram.Temp_save_Zvalue;
				control_panel_pram.A_value=control_panel_pram.Temp_save_Avalue;
				control_panel_pram.B_value=control_panel_pram.Temp_save_Bvalue;					
				Puless_count_clear();                //��������Ĵ�������							
				return_workpiece_zero.Re_X_Value=0;
				return_workpiece_zero.Re_Y_Value=0;
				return_workpiece_zero.Re_Z_Value=0;
				return_workpiece_zero.Re_A_Value=0;
				return_workpiece_zero.Re_B_Value=0;	
				return_workpiece_zero.Cancel=0;
				
			}
			Show_coordinate_on_return_workpiece_zero_page();  //�ڻع�����ҳ����ʾ����������ֵ				
			
		}
		  break;
		case Jump_Work_Page:     //*****************************************************���мӹ�ҳ��****************************************************************************************
		{
			if(jump_work_set.First_get_into)
			{
				sprintf(Working_line_buf,"%d",Working_line);  
				SetTextValue(5,4,(uchar *)Working_line_buf);     //��ʾ�ϴμӹ����� 
				SetTextValue(5,3,(uchar *)Working_line_buf);     //��ʾ�ϴμӹ�����
				jump_work_set.First_get_into=0;
			}
			
		}
			break;
		case File_Manage_Page:    //*****************************************************�ļ�����ҳ��****************************************************************************************
		{
		  char buf[20];
		  if(Press_button==CMD_Download)   //���ذ�ť����
			{
				Start_Download=1;               //���ر�־λ��1
				SetButtonValue(6,1,1);    //��ֵ��ťΪ����״̬
			  SetControState(6,1,0);		 //��ֹ�ؼ�	
				Press_button=0xFF;
			}		
			if(Start_Download)
			{
			   sprintf(buf,"������%d%%",Download_Per);
				 SetTextValue(6,38,(uchar *)buf);     //��ʾ���ذٷֱ�
				 if(Download_Per==100)
				 {
					 Download_Per=0;
					 Start_Download=0;
					 ClearTextValue(6,38);     //����ı�
					 SetControState(6,1,1);		 //ʹ�ܿؼ�	
					 SetButtonValue(6,1,0);    //����ֵ��ťλ��ʼ״̬
				 }
			}
			if(Press_button==CMD_Cancel_Download)  //��ȡ�����ذ�ť
			{
				Start_Download=0;
				Download_Per=0;
				ClearTextValue(6,38);     //����ı�
				SetControState(6,1,1);		 //ʹ�ܿؼ�	
				SetButtonValue(6,1,0);    //���ð�ťΪ��ʼ״̬
				Press_button=0xFF;
			
			}
			if(Last_time_Cloud_Page!=Cloud_Page)
			{
			  sprintf(buf,"%u",Cloud_Page);
				SetTextValue(6,20,(uchar *)buf);     //��ʾ�ڼ�ҳ
				Last_time_Cloud_Page=Cloud_Page;
			}
			if(Last_time_SD_Page!=SD_Page)
			{
			  sprintf(buf,"%u",SD_Page);
				SetTextValue(6,21,(uchar *)buf);     //��ʾ�ڼ�ҳ
				Last_time_SD_Page=SD_Page;
			}
			if(Press_button==CMD_Delete)
			{
			  //sprintf(buf,"%u",SD_Page);
			  SetTextValue(9,3,(uchar *)file_name);     //��ʾ�ļ�����
				Press_button=0xFF;
			}
			if(Press_button==CMD_File_Delete_Sure)
			{
			
			}
		
		}
			break;
		case Same_file_Error_Page:    //**************************************************ͬ���ļ�����ҳ��****************************************************************************************
			break;
		case Download_Fail_Pgae:    //***************************************************����ʧ������ҳ��
			break;
		case Delete_Page:         //*****************************************************ɾ��ҳ��****************************************************************************************
		{		}	
		break;
		case Storage_View_Page:    //*****************************************************�ڴ�Ԥ��ҳ��****************************************************************************************
			break;	
		case Net_Account_Manage_Page:  //**************************************************�������˻���¼����**********************************************************************************
			{
				if(WiFi_Password_Right==5)   //WiFi�˺ź�������ȷ
				{
				  Network_Control=3;
					WiFi_Password_Right=0;
				}
				if(Account_Password_Right==5) //�û��˻���������ȷ
				{
					Sign_Control=3;
					Account_Password_Right=0;
				}
				switch(Network_Control)     //�������ӿؼ���ʾ�����ؿ���
				{
					case 0: break;
					case Show_Disconnect_Network_Control: 
					{
						ShowControl(11,18,1);
						Network_Control=0;
					}break;
					case Hide_Disconnect_Network_Control: 
					{
						ShowControl(11,18,0);
						Network_Control=0;
					}break;				
				}
				//ShowControl(11,4,1);
				switch(Sign_Control)     //�˻���¼�˳��ؼ���ʾ�����ؿ���
				{
					case 0: break;
					case Show_Sign_Out_Control: 
					{
						ShowControl(11,19,1);
						Network_Control=0;
					}break;
					case Hide_Sign_Out_Control: 
					{
						ShowControl(11,19,0);
						Network_Control=0;
					}break;					
				}									
			}
			break;
		case Choose_WiFi_Page:        //*****************************************************ѡ��WiFiҳ��****************************************************************************************
		{
		  char buf[20];
			if(Last_time_WiFi_Page!=WiFi_Page)
			{
			  sprintf(buf,"%u",WiFi_Page);
				SetTextValue(12,11,(uchar *)buf);     //��ʾ�ڼ�ҳ
				Last_time_WiFi_Page=WiFi_Page;
			}	
		}break;
		case Disconnect_Remind_Page: 
			break;
		case SignOut_Remind_Page: 
			break;
		case Cancel_Download_Pgae:
			break;
		default:
			break;
	}

}


/****************************************************
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
******************************************************/
void Usart2_Receive_data_handle( PCTRL_MSG msg, uint16 size )
{
    get_cmd_type = msg->cmd_type;                                         //��ȡָ������
    get_ctrl_msg = msg->ctrl_msg;                                         //��ȡ��Ϣ������	  
    get_control_type = msg->control_type;                                 //��ȡ�ؼ�����
    get_screen_id = PTR2U16(&msg->screen_id);                             //��ȡ����ID
    get_control_id = PTR2U16(&msg->control_id);                           //��ȡ�ؼ�ID
    get_value = PTR2U32(msg->param);                                      //��ȡ��ֵ
	
	  if(get_control_type==kCtrlButton)                                      //��ť�ؼ�
			get_button_state= msg->param[1];                                     //��ȡ��ť״̬
   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //�ؼ�����֪ͨ
    {  
				switch(get_screen_id)                                             //����ID
				{
					case Working_Page:                                              //����0���ӹ�ҳ��
					{
							Work_Page_Status=Working_Page;
							switch(get_control_id)
							{
									case 3:                  //�����ٶȼ�
									{
										if(get_button_state)
										{
											  Speed.Spindle_Speed_Changed=Changed;
												Speed.Initial_Spindle_Speed_Percent -= 10; 
												if(Speed.Initial_Spindle_Speed_Percent <= 0)
												{
													Speed.Initial_Spindle_Speed_Percent = 0;
												}
												Speed.Changed_Spindle_Speed=Speed.Initial_Spindle_Speed*Speed.Initial_Spindle_Speed_Percent/100;
												get_button_state=0;
									  }
									}break;	
									case 4:                  //�����ٶȼ�
									{
										if(get_button_state)
										{   
											  Speed.Spindle_Speed_Changed=Changed;
												Speed.Initial_Spindle_Speed_Percent += 10;
												if(Speed.Initial_Spindle_Speed_Percent >= 100)
												{
													Speed.Initial_Spindle_Speed_Percent = 100;
												}
												Speed.Changed_Spindle_Speed=Speed.Initial_Spindle_Speed*Speed.Initial_Spindle_Speed_Percent/100;
												get_button_state=0;
									  }
									}break;
									case 5:                  //�ӹ��ٶȼ�
									{
										if(get_button_state)
										{
											  Speed.Work_Speed_Changed=Changed;
												if(Speed.Initial_Work_Speed_Percent <= 100)
												{
													Speed.Initial_Work_Speed_Percent -=10;
													if(Speed.Initial_Work_Speed_Percent <= 0)
													{
														Speed.Initial_Work_Speed_Percent=0;
													}
												}
												else if(100 < Speed.Initial_Work_Speed_Percent && Speed.Initial_Work_Speed_Percent <=300)
												{
													Speed.Initial_Work_Speed_Percent-=25;						
												}
												Speed.Changed_Work_Speed=Speed.Initial_Work_Speed*Speed.Initial_Work_Speed_Percent/100;
		                    get_button_state=0;
									  }												
									}break;
									case 6:                  //�ӹ��ٶȼ�
									{
										if(get_button_state)
										{
											  Speed.Work_Speed_Changed=Changed;
												if(Speed.Initial_Work_Speed_Percent < 100)
												{
													Speed.Initial_Work_Speed_Percent +=10;
												}
												else if(100 <= Speed.Initial_Work_Speed_Percent && Speed.Initial_Work_Speed_Percent <=300)
												{
													Speed.Initial_Work_Speed_Percent +=25;
													if(Speed.Initial_Work_Speed_Percent >= 300)
													{
														Speed.Initial_Work_Speed_Percent=300;
													}						
												}
												Speed.Changed_Work_Speed=Speed.Initial_Work_Speed*Speed.Initial_Work_Speed_Percent/100;
												get_button_state=0;
									  }	
									}break;
									case 12:Work_Page_Status=File_Manage_Page;break;
									case 13:Work_Page_Status=Net_Account_Manage_Page;break;
									case 14:Work_Page_Status=Setting_page;break;
									case 15:Work_Page_Status=ControlPanel_Page;break;
									case 42:                                           //X�ᰴť����
									{																			
										control_panel_pram.Axis_press = CMD_X_AXIS;								
									}break;
									case 43:                                           //Y�ᰴť����
									{
																				
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 44:                                              //Z�ᰴť����
									{
																			
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 45:                                             //A�ᰴť����
									{
																				
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 46:                                           //B�ᰴť����
									{
																			
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;								
									default:break;
						  }
					}break;
					case Setting_page:                                              //����1������ҳ��
					{
						//char buf[20];
						Work_Page_Status=Setting_page;
						if(state.Work_state==Stop)             //ֹͣ�ӹ�״̬
						{
							switch(get_control_id)                                
							{
									case 1:                           //��ȫZ��ť����
									{
										if(get_button_state)
											pram_status.Safe_Z_button_satus=1;
										else
											pram_status.Safe_Z_button_satus=0;													
									}break;
									case 2:                          //�Զ��Ե���ť����
									{
										if(get_button_state)
											pram_status.Auto_Knife_button_status=1;
										else
											pram_status.Auto_Knife_button_status=0;						
									}break;
									case 3:                          //��λѡ��ť����
									{
										if(get_button_state)
											pram_status.Unit_Change_button_status=1;
										else
											pram_status.Unit_Change_button_status=0;						
									}break;
									case 6:                          //��ȡ��ȫZ�߶�
									{
										
										pram_status.Safe_Z_num=NotifyText(msg->param);										
//										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);    
									}										
									break;
									case 7:                   //��ȡ�Ե��߶�
									{
										
										pram_status.Knife_high_num=NotifyText(msg->param);										
//										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8:                     //��ȡ�Ե���߶�
									{
										
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9:                  //��ȡ�Ե���X��λ��
									{
										
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10:                 //��ȡ�Ե���Y��λ��
									{
										
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11:                 //��ȡ�Ե���Z ��λ��
									{
										
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
//										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12:                  //��ȡ����λX��λ��
									{
										
										pram_status.Soft_limit_X=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13:                   //��ȡ����λY��λ��
									{
										
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14:                 //��ȡ����λZ��λ��
									{
										
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15:                 //��ȡ����λA��λ��
									{ 
										
										pram_status.Soft_limit_A=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16:                  //��ȡ����λB��λ��
									{
										
										pram_status.Soft_limit_B=NotifyText(msg->param);										
//										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 27:                           //������ʾ��ť���� 
									{
										if(get_button_state)                                
											pram_status.voice_button_status=voice_off; //��ť���£��������ѹ�
										else
											pram_status.voice_button_status=voice_on;  //��ť�ɿ����������ѿ�													
									}break;
									case 4:                             //ȷ����ť����
									{
		//								pram_status.Screen_ID1_Setting_Sure=1;	
		//                pram_status.Screen_ID1_Setting_concel=0;								
									}break;
									case 5:                             //ȡ����ť����
									{
		//								pram_status.Screen_ID1_Setting_Sure=0;
										pram_status.Screen_ID1_Setting_concel=1;						
									}break;
								
									default: break;		
                 }									
						}					
					}break;   
					case ControlPanel_Page:                                         //����2���������
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //���㰴ť����
									{
										if(get_button_state)
										{
											Press_button = CMD_Clear;								
										}																					
									}break;    
                  case 2:                                             //�ػ�е�㰴ť����	
                  {
										if(get_button_state)
										{
											Press_button = CMD_Return_Machine_Zero;
																								
										}
										get_button_state=0;	
									}break;                                     									
									case 3:								                              //�����л���ť����
									{
										if(get_button_state)
										{
												control_panel_pram.Override_Change_button=1;
												Override_Change_Process();                    //�����л�����
												Press_button = CMD_Override_Change;												
										}
										get_button_state=0;										
									}break;
								  case 4:                                             //���Ὺ�ذ�ť����
									{
										if(get_button_state)
										{
											Press_button = CMD_Spin_On_Off;						
										}
										get_button_state=0;	
									}break;
			            case 5:                                              //ȫ�����㰴ť����
									{
                    if(get_button_state)   
										{										
											control_panel_pram.All_Spindle_Clear_Button=1;
											Press_button = CMD_All_Spin_Clear;
											get_button_state=0;
										}
									}break;
									case 6:                                            //�ع����㰴ť����
									{
										if(get_button_state)    
										{
											Work_Page_Status=Return_WorkPiece_Zero_Page;
											first_time_re_workpiece=1;
											Press_button = CMD_Return_WorkPiece_Zero;
											get_button_state=0;
										}
									}break;
									case 7:                                            //�����л���ť����
									{
											if(get_button_state)
											{										
												  Coordinate_Change_Process();//�����л�����
												  Press_button = CMD_Coordinate_Change;
												  get_button_state=0;
											 }										
									}break;
									case 8:                                            //����λ���ذ�ť����
									{											
											if(get_button_state)
											{
												Press_button = CMD_Soft_Limit;						
											}
											get_button_state=0;	
									}break;
									case 9:                                            // ��ȫZ����
									{
										if(get_button_state)
											{
												Press_button = CMD_Safe_Z;						
											}
											get_button_state=0;	
									}break;
									case 10:                                         //���мӹ���ť����
									{
										if(get_button_state)
										{
											jump_work_set.First_get_into = 1;
											Work_Page_Status=Jump_Work_Page;
											Press_button = CMD_Jump_Work;
											get_button_state=0;
										}
									}break;
									case 11:                                          //�Ե���ť����
									{										
											if(get_button_state)
											{
												Press_button = CMD_Auto_knife;								
											}
											get_button_state=0;	
									}break;
									case 12:                                         //���а�ť����
									{
											if(get_button_state)
											{
												Press_button = CMD_Divided;								
											}
											get_button_state=0;	
									}break;
									case 13:                                           //��ʼ��ť����
									{
										state.Work_state=Start;
										Press_button = CMD_Start;
										TIM_Cmd(TIM4, DISABLE);      //��ֹ TIM4
//										SetButtonValue(2,14,0);     //ֹͣ��ť�ɿ�״̬
										SetButtonValue(2,13,1);       //��ʼ��ť����״̬										
									}break;
									case 14:                                           //ֹͣ��ť����
									{
										state.Work_state=Stop;
//										SetButtonValue(2,13,0);     //��ʼ��ť�ɿ�״̬
										SetButtonValue(2,14,1);     //ֹͣ��ť����״̬
                    Press_button = CMD_Stop;
									  TIM_Cmd(TIM4, ENABLE);      //ʹ�� TIM4
																	
									}break;
									case 15:                                            //��λ��ť����
									{
										state.Work_state=Stop;
										Press_button = CMD_Reset;  						
									}break;
									case 16:                                          //�˳�������尴ť����
									{
										Work_Page_Status=Working_Page;
									}
									break;
									case 22:                                            //X�ᰴť����
									{	
																											
										control_panel_pram.Axis_press = CMD_X_AXIS;								
									}break;
									case 23:                                            //Y�ᰴť����
									{
																		
										control_panel_pram.Axis_press = CMD_Y_AXIS;						
									}break;
									case 24:                                              //Z�ᰴť����
									{
															
										control_panel_pram.Axis_press = CMD_Z_AXIS;														
									}break;
									case 25:                                             //A�ᰴť����
									{
																	
										control_panel_pram.Axis_press = CMD_A_AXIS;															
									}break;
									case 26:                                           //B�ᰴť����
									{  
																	
										control_panel_pram.Axis_press = CMD_B_AXIS;											
									}break;
									default:break;				
							}
					}break;
					case Return_WorkPiece_Zero_Page:                                //����3���ع�����ҳ��
					{
						Work_Page_Status=Return_WorkPiece_Zero_Page;
						if(state.Work_state==Stop)    //ֹͣ�ӹ�״̬
						{
							switch(get_control_id)                                
							{
									case 1:                  //ȫ�ᰴť����
									{
										if(get_button_state)            //��ť�ǰ���״̬  
										{
											return_workpiece_zero.all_spindle_status=1; //��ʱȫ�ᰴť���£�XYZAB����ʾ��û��ѡ��״̬
											SetButtonValue(3,4,1);
											SetButtonValue(3,5,1);
											SetButtonValue(3,6,1);
											SetButtonValue(3,7,1);
											SetButtonValue(3,8,1);
											return_workpiece_zero.X_clear_status=1;
											return_workpiece_zero.Y_clear_status=1;
											return_workpiece_zero.Z_clear_status=1;
											return_workpiece_zero.A_clear_status=1;
											return_workpiece_zero.B_clear_status=1;										
										}
										else 
										{
											return_workpiece_zero.all_spindle_status=0;
											return_workpiece_zero.X_clear_status=0;
											return_workpiece_zero.Y_clear_status=0;
											return_workpiece_zero.Z_clear_status=0;
											return_workpiece_zero.A_clear_status=0;
											return_workpiece_zero.B_clear_status=0;
											
										}
									
									}break;
									case 2:                     //ȡ����ť����
									{
									  TIM_Cmd(TIM4, ENABLE);      //ʹ�� TIM4
										if(get_button_state) 
										{
											return_workpiece_zero.Cancel=1;  //ȡ����ť����											
										}
										else
										{
											return_workpiece_zero.Cancel=0;  //ȡ����ť�ɿ�
											Work_Page_Status=ControlPanel_Page;
											
										}										
									
									}break;
									case 3:                   //ȷ����ť����
									{
										TIM_Cmd(TIM4, ENABLE);      //ʹ�� TIM4
										if(get_button_state) 
										{
											return_workpiece_zero.Sure=1;  //ȷ����ť����											
										}
										else
										{
											Work_Page_Status=ControlPanel_Page;
										}
																		
									}break;
									case 4:                           //X�ᰴť����
									{
										if(get_button_state) 
										{											
										  return_workpiece_zero.X_clear_status=1;
										}
										else
										{
											return_workpiece_zero.X_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 5:                           //Y�ᰴť����
									{
										if(get_button_state) 
										{											
										  return_workpiece_zero.Y_clear_status=1;
										}
										else
										{
											return_workpiece_zero.Y_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 6:                           //Z�ᰴť����
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.Z_clear_status=1;
										}
										else
										{
											return_workpiece_zero.Z_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 7:                           //A�ᰴť����
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.A_clear_status=1;
										}
										else
										{
											return_workpiece_zero.A_clear_status=0;
											return_workpiece_zero.all_spindle_status=0;
										}
									
									}break;
									case 8:                           //B�ᰴť����
									{
										if(get_button_state)
										{											
										  return_workpiece_zero.B_clear_status=1;
										}
										else
										{
											return_workpiece_zero.B_clear_status=0;
                      return_workpiece_zero.all_spindle_status=0;											
										}											
									}break;
									case 9:                    //ͨ�����ּ��̸���X������ֵ
									{
									  
										return_workpiece_zero.Re_X_Value = NotifyText(msg->param);     //��ȡ������ֵ
										return_workpiece_zero.X_get_value = 1;                         //��ȡ��������ֵ����־λ��1
										control_panel_pram.X_value = return_workpiece_zero.Re_X_Value;
										
									}break;
									case 10:                 //ͨ�����ּ��̸���Y������ֵ
									{
									  
										return_workpiece_zero.Re_Y_Value = NotifyText(msg->param);
										return_workpiece_zero.Y_get_value = 1;                         //��ȡ��������ֵ����־λ��1
										control_panel_pram.Y_value = return_workpiece_zero.Re_Y_Value;
									}break;
									case 11:                  //ͨ�����ּ��̸���Z������ֵ
									{
									 
										return_workpiece_zero.Re_Z_Value = NotifyText(msg->param);
										return_workpiece_zero.Z_get_value = 1;                         //��ȡ��������ֵ����־λ��1
										control_panel_pram.Z_value = return_workpiece_zero.Re_Z_Value;	

									}break;
									case 12:                  //ͨ�����ּ��̸���A������ֵ
									{
									  
										return_workpiece_zero.Re_A_Value = NotifyText(msg->param);
										return_workpiece_zero.A_get_value = 1;                         //��ȡ��������ֵ����־λ��1
										control_panel_pram.A_value = return_workpiece_zero.Re_A_Value;
									}break;
									case 13:                  //ͨ�����ּ��̸���B������ֵ
									{
									  
										return_workpiece_zero.Re_B_Value = NotifyText(msg->param);
										return_workpiece_zero.B_get_value = 1;                         //��ȡ��������ֵ����־λ��1
										control_panel_pram.B_value = return_workpiece_zero.Re_B_Value;

									}break;
									default:break;
							}
						}
						else
						{
						
						}
					}break; 
					case Jump_Work_Page:                                            //����5:���мӹ�ҳ��
					{
						switch(get_control_id)
						{
							case 1:                         //ȷ����ť����
							{
							  jump_work_set.Jump_Work_Sure = 1;
								jump_work_set.Jump_Work_cancel = 0;
								Working_line = jump_work_set.New_work_line;
								Work_Page_Status = ControlPanel_Page;
							}break;
							case 2:                        //ȡ����ť����
							{
							  jump_work_set.Jump_Work_cancel = 1;
								jump_work_set.Jump_Work_Sure = 0;
								Work_Page_Status = ControlPanel_Page;
							}break;
							case 3:                        //��ȡ�µ�����
							{
							  jump_work_set.New_work_line=NotifyText(msg->param);
							}break;
						}
						
					}break;              
					case File_Manage_Page:                                           //����6:�ļ�����
					{
						Work_Page_Status=File_Manage_Page;
						switch(get_control_id)
						{
							case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_Download;								
								}
							}
							break;
							case 2:  //ȡ�����ذ�ť����
							{
							  
							}
							break;
							case 3: 
								{
							  if(get_button_state)
								{
									Press_button = CMD_Delete;								
								}
							}
							break;
							case 4:
              {
								Work_Page_Status=Storage_View_Page;
							  if(get_button_state)
								{
									Press_button = CMD_Storage_View;								
								}
							}								
							break;
							case 5: Work_Page_Status=ControlPanel_Page;break;
							case 6: 
							{
							  if(get_button_state)
								{
									if(Cloud_Page>1)
								    Cloud_Page--;
								}
							}	
							break;
							case 7: 
							{
							  if(get_button_state)
								{
								  Cloud_Page++;
								}
							}
								break;
							case 8: 
							{
							  if(get_button_state)
								{
								  if(SD_Page>1)
								    SD_Page--;
								}
							}
								break;
							case 9: 
							{
							  if(get_button_state)
								{
								  SD_Page++;
								}
							}
								break;
							case 10:Cloud_File_Button=0X01;break;
							case 11:Cloud_File_Button=0X02;break;
							case 12:Cloud_File_Button=0X03;break;
							case 13:Cloud_File_Button=0X04;break;
							case 14:SD_File_Button=0X01;break;
							case 15:SD_File_Button=0X02;break;
							case 16:SD_File_Button=0X03;break;
							case 17:SD_File_Button=0X04;break;
							
							case 18:Work_Page_Status=Working_Page;break;
							case 19:Work_Page_Status=Net_Account_Manage_Page;break;
							default:break;
						}
					}break;
					case Same_file_Error_Page: Work_Page_Status=Same_file_Error_Page;break;//����7��ͬ���ļ�����ҳ��
					case Download_Fail_Pgae: Work_Page_Status=Download_Fail_Pgae;break;    //����8������ʧ��ҳ��
					case Delete_Page: Work_Page_Status=Delete_Page;                        //����9��ɾ��ҳ��
					{
						switch(get_control_id)
						{
					    case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_File_Delete_Sure;								
								}
							}
						}
					}
					break;
					case Storage_View_Page: Work_Page_Status=Storage_View_Page;break;   //����10���ڴ�ռ�Ԥ��ҳ��
					case Net_Account_Manage_Page:                                       //����11�������˻�����
					{
						Work_Page_Status=Net_Account_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Working_Page;break;
							case 2: Work_Page_Status=File_Manage_Page;break;
							case 3: Work_Page_Status=Choose_WiFi_Page;break;
							case 4: 
							{
							  if(get_button_state)
								{
									WiFi_Password_Right++;								
								}
							
							}break;
							case 5:
							{
							  if(get_button_state)
								{
									Account_Password_Right++;								
								}
							
							}
							default:break;
						}
					}break;
					case Choose_WiFi_Page:                                                 //����12��ѡ��WiFi
   				{
						Work_Page_Status=Choose_WiFi_Page;
						switch(get_control_id)
						{
							case 1: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 2: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 3: 
							{
							  Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 4: 
							{
								Work_Page_Status=Net_Account_Manage_Page;
							}break;
							case 5:
							{
							  if(get_button_state)
								{
								    WiFi_Page++;
								}
							}break;
							case 6: 
							{
								if(get_button_state)
								{
									if(WiFi_Page>1)
								    WiFi_Page--;
								}
							}break;
							default:break;
						}
					}break;          
					case Disconnect_Remind_Page:                                            //����15���Ͽ���������ҳ��
					{
						switch(get_control_id)
						{
							case 1: Network_Control=4;
						}
					}break;     
					case SignOut_Remind_Page:                                                //����16���˳���¼����ҳ��
					{
					  switch(get_control_id)
						{
							case 1: Sign_Control=4;
						}
					}	
					break;         
					case Cancel_Download_Pgae:                                                //ȡ����������
					{
            switch(get_control_id)
						{
							case 1:
							{
							  if(get_button_state)
								{
									Press_button = CMD_Cancel_Download;								
								}
							}
							break;
							case 2: 
							{
							  if(get_button_state)
								{
									Press_button = 0xFF;								
								}
							}
							break;
					  }
					}break;
					case Save_Pram_Page:                                                    //����21 ���������������ҳ��
					{
						Work_Page_Status=Save_Pram_Page;
						if(state.Work_state==Stop)    //ֹͣ�ӹ�״̬
						{
								switch(get_control_id)
								{
									case 1:    //ȷ����ť����
									{							
										pram_status.Screen_ID21_Setting_Sure=1;
										pram_status.Screen_ID21_Setting_concel=0;
									}break;
									case 2:   //ȡ����ť����
									{
										pram_status.Screen_ID21_Setting_Sure=0;
										pram_status.Screen_ID21_Setting_concel=1;						
									}
									default: break;	
								}
					  }
					}break;
					default: break;						
  			}
        
     }
		 
}

/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param str �ı��ؼ�����
*  \pram  value ����ı�����
*/
float NotifyText(uint8 *str)
{                                                                          
		float value=0;                                                            
		sscanf((char *)str,"%f",&value);            
    return  value;                                                                      
}                                                                                



//��������״̬��ʾ
void Work_state_control(void)
{
  if(last_time_work_state!=state.Work_state)     //�жϹ���״̬�Ƿ����仯�������仯����д���
	{
		if(state.Work_state==Start)         
		{
			Show_Start_Working(Work_Page_Status);      //TFT����ʾ��ǰ���ڼӹ�״̬
		}
		else                                 
		{
			Show_Stop_Working(Work_Page_Status);        //TFT����ʾ��ǰ�Ѿ�ֹͣ�ӹ�
		}
		last_time_work_state=state.Work_state;
	}

}

//��ʾ�����Ѿ�ֹͣ�ӹ�
void Show_Stop_Working(uint8 state)
{
	if(state==0)
	  AnimationPlayFrame(0,11,0);
	else if(state==1)
	  AnimationPlayFrame(1,28,0);
	else if(state==2)
	  AnimationPlayFrame(2,43,0);
	else if(state==6)
	  AnimationPlayFrame(6,33,0);
	else if(state==10)
	  AnimationPlayFrame(10,5,0);
	else if(state==11)
	  AnimationPlayFrame(11,13,0);
}
//��ʾ�������ڼӹ�
void Show_Start_Working(uint8 state)
{
	if(state==0)
		AnimationPlayFrame(0,11,1);
	else if(state==1)
		AnimationPlayFrame(1,28,1);
	else if(state==2)
		AnimationPlayFrame(2,43,1);
	else if(state==6)
		AnimationPlayFrame(6,33,1);
	else if(state==10)
		AnimationPlayFrame(10,5,1);
	else if(state==11)
		AnimationPlayFrame(11,13,1);
	
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
	delay_ms(500); 
		
	for(i=0;i<50;i++)
	{
		SetProgressValue(17,3,i*2);     //���ý�������ֵ
		delay_ms(5);                    //��ʱ0.05��
	}
	SetScreen(0);                    //�л����ӹ�ҳ��
	//SetTouchPaneOption(1,1,0,0);     //����������
	delay_ms(10);
	
	SetTextValue(0,27,"G54");        //���õ�ǰ����ֵ��G54
	SetTextValue(2,30,"G54");
	SetTextValue(0,28,"��");         //���������������ء�
	SetTextValue(2,31,"��");
	SetTextValue(0,29,"X10");         //���ñ��ʣ�X10
	SetTextValue(2,32,"X10");
	Override_num=10;                   //����Ĭ�ϱ���
	
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
	
	Work_Page_Status=Working_Page;    //��������ӹ�ҳ��
	
	control_panel_pram.Axis_press = CMD_X_AXIS;
	SetButtonValue(0,42,1);           //X��ѡ��״̬
	SetButtonValue(2,22,1);
	


}

//����ҳ�漸������ֵ��ȡ���ϵ籣����flash��
void Get_Setting_page_pram(void)
{
	pram_status.Voice_last_status=AT24CXX_ReadOneByte(0);
	pram_status.voice_button_status=pram_status.Voice_last_status;            //������������ģʽ
	pram_status.Safe_Z_last_status=AT24CXX_ReadOneByte(1);
	pram_status.Safe_Z_button_satus=pram_status.Safe_Z_last_status;           //���ð�ȫZģʽ
	pram_status.Auto_Knife_last_status=AT24CXX_ReadOneByte(2);
	pram_status.Auto_Knife_button_status=pram_status.Auto_Knife_last_status;  //�����Զ��Ե�ģʽ
	pram_status.Unit_Change_last_status=AT24CXX_ReadOneByte(3);
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
		
	}
	if(Speed.Work_Speed_Changed==Changed)
	{
		Speed.Work_Speed_Changed=Unchanged;
		sprintf(buf,"%d",Speed.Changed_Work_Speed);
		SetTextValue(0,25,(uchar *)buf);                                   //�޸ļӹ��ٶ��ı�����ֵ
		SetProgressValue(0,2,Speed.Initial_Work_Speed_Percent);            //�޸ļӹ��ٶȽ�������ֵ	
		sprintf(buf,"%d",Speed.Initial_Work_Speed_Percent);
		SetTextValue(0,39,(uchar *)buf);  
		
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
	AT24CXX_WriteOneByte(0,pram_status.Voice_last_status);
//	FLASH_WriteByte(START_ADDR1,(uint16)pram_status.Voice_last_status);
}
//����ҳ�氲ȫZ��ť�����������
void Safe_Z_process(uint8 state)
{
	if(state)                  //ģʽ2����ť����״̬��
	{
		SetButtonValue(1,1,1);
	  pram_status.Safe_Z_last_status=1;
//		Usart_SendString(USART2,"Safe_z:mode2"); 
	}
	else
	{
		SetButtonValue(1,1,0);      //ģʽ1
	  pram_status.Safe_Z_last_status=0;
//		Usart_SendString(USART2,"Safe_z:mode1"); 
	}
	AT24CXX_WriteOneByte(1,pram_status.Safe_Z_last_status);
//	FLASH_WriteByte(START_ADDR2,(uint16)pram_status.Safe_Z_last_status);
}
//����ҳ���Զ��Ե���ť�����������
void Auto_Knife_process(uint8 state)
{
  if(state)                  //ģʽ2����ť����״̬��
	{
		SetButtonValue(1,2,1);
	  pram_status.Auto_Knife_last_status=1;
//		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	else
	{
		SetButtonValue(1,2,0);      //ģʽ1
	  pram_status.Auto_Knife_last_status=0;
//		Usart_SendString(USART2,"Auto_Knife:mode2");
	}
	AT24CXX_WriteOneByte(2,pram_status.Auto_Knife_last_status);
//	FLASH_WriteByte(START_ADDR3,(uint16)pram_status.Auto_Knife_last_status);
}
//����ҳ�浥λ�л���ť�����������
void Unit_Change_process(uint8 state)
{
	if(state )                  //ģʽ2����ť����״̬:Ӣ�ƣ�
	{
		SetButtonValue(1,3,1);
	  pram_status.Unit_Change_last_status=1;
//		Usart_SendString(USART2,"Unit:mode2");
	}
	else
	{
		SetButtonValue(1,3,0);      //ģʽ1
	  pram_status.Unit_Change_last_status=0;
//		Usart_SendString(USART2,"Unit:mode1");
	}
	AT24CXX_WriteOneByte(3,pram_status.Unit_Change_last_status);
//  FLASH_WriteByte(START_ADDR4,(uint16)pram_status.Unit_Change_last_status);

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
			SetTextValue(0,29,"X10");    //�����ı�ֵ
			SetTextValue(2,32,"X10");
			Override_num=10;
			break;  
		}
		case 2:
		{
			SetTextValue(0,29,"X20");                                                                                                                                 
			SetTextValue(2,32,"X20");
			Override_num=20;
			break;
		}
		case 3:
		{
			SetTextValue(0,29,"X1");
			SetTextValue(2,32,"X1");
			Override_num=1;
			break;
		}
		case 4:
		{
			SetTextValue(0,29,"X2");
			SetTextValue(2,32,"X2");
			Override_num=2;
			break;
		}
		case 5:
		{
			SetTextValue(0,29,"X5");
			SetTextValue(2,32,"X5");
			Override_num=5;
			Override_Change_Counter=0;
			break;
		}	
    default:break;		
	}	
  
}

// �ڼӹ�ҳ��Ϳ������ҳ����ʾ����������
void TFT_Show_coordanate_value(void)
{
		
		Show_X_Coordinata();
		Show_Y_Coordinata();
		Show_Z_Coordinata();
		Show_A_Coordinata();
		Show_B_Coordinata();			

//	sprintf((char *)buf1,"%09.2f",control_panel_pram.X_Mac_value); 
//	SetTextValue(2,38,(uchar *)buf1);	                               //��ʾX���е����
//	SetTextValue(0,34,(uchar *)buf1);

//	
//	sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_Mac_value);
//	SetTextValue(2,39,(uchar *)buf1);                                //��ʾY���е����
//	SetTextValue(0,35,(uchar *)buf1);
//	
//	sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_Mac_value);
//	SetTextValue(2,40,(uchar *)buf1);                                //��ʾZ���е����
//	SetTextValue(0,36,(uchar *)buf1);
//	
//	sprintf((char *)buf1,"%09.2f",control_panel_pram.A_Mac_value);
//	SetTextValue(2,41,(uchar *)buf1);                                //��ʾA���е����
//	SetTextValue(0,37,(uchar *)buf1);
//	
//	sprintf((char *)buf1,"%09.2f",control_panel_pram.B_Mac_value);
//	SetTextValue(2,42,(uchar *)buf1);                               //��ʾB���е����
//	SetTextValue(0,38,(uchar *)buf1);

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


//��ʾX������
void Show_X_Coordinata(void)
{	
	char buf1[20];
	if(control_panel_pram.X_last_value!=control_panel_pram.X_value)
	{
		sprintf((char *)buf1,"%09.2f",control_panel_pram.X_value);      //��ʾX�Ṥ������		
		SetTextValue(0,16,(uchar *)buf1);
		SetTextValue(2,17,(uchar *)buf1);
		control_panel_pram.X_last_value=control_panel_pram.X_value;
	}
}


//��ʾY������
void Show_Y_Coordinata(void)
{
	char buf1[20];
	if(control_panel_pram.Y_last_value!=control_panel_pram.Y_value)
	{
		sprintf((char *)buf1,"%09.2f",control_panel_pram.Y_value);       //��ʾY�Ṥ������
		SetTextValue(0,17,(uchar *)buf1);
		SetTextValue(2,18,(uchar *)buf1);
		control_panel_pram.Y_last_value=control_panel_pram.Y_value;
	}
}

//��ʾZ������
void Show_Z_Coordinata(void)
{
	char buf1[20];
	if(control_panel_pram.Z_last_value!=control_panel_pram.Z_value)
	{
		sprintf((char *)buf1,"%09.2f",control_panel_pram.Z_value);      //��ʾZ�Ṥ������
		SetTextValue(0,18,(uchar *)buf1);
		SetTextValue(2,19,(uchar *)buf1);
		control_panel_pram.Z_last_value=control_panel_pram.Z_value;
	}
}

//��ʾA������
void Show_A_Coordinata(void)
{
	char buf1[20];
	if(control_panel_pram.A_last_value!=control_panel_pram.A_value)
	{
		sprintf((char *)buf1,"%09.2f",control_panel_pram.A_value);       //��ʾA�Ṥ������
		SetTextValue(0,19,(uchar *)buf1);
		SetTextValue(2,20,(uchar *)buf1);
		control_panel_pram.A_last_value=control_panel_pram.A_value;		
	}
}

//��ʾB������
void Show_B_Coordinata(void)
{
	char buf1[20];
	if(control_panel_pram.B_last_value!=control_panel_pram.B_value)
	{
		sprintf((char *)buf1,"%09.2f",control_panel_pram.B_value);        //��ʾB�Ṥ������
		SetTextValue(0,20,(uchar *)buf1);
		SetTextValue(2,21,(uchar *)buf1);
		control_panel_pram.B_last_value=control_panel_pram.B_value;
	}
}

//��ʾX��ѡ��״̬
void Show_X_Axis_State(void)
{
  SetButtonValue(0,42,1);           //X��ѡ��״̬
	SetButtonValue(0,43,0);
	SetButtonValue(0,44,0);
	SetButtonValue(0,45,0);
	SetButtonValue(0,46,0);
	
  SetButtonValue(2,22,1);
	SetButtonValue(2,23,0);
	SetButtonValue(2,24,0);
	SetButtonValue(2,25,0);
	SetButtonValue(2,26,0);
}

//��ʾY��ѡ��״̬
void Show_Y_Axis_State(void)
{
  SetButtonValue(0,42,0);           //Y��ѡ��״̬
	SetButtonValue(0,43,1);
	SetButtonValue(0,44,0);
	SetButtonValue(0,45,0);
	SetButtonValue(0,46,0);
	
  SetButtonValue(2,22,0);
	SetButtonValue(2,23,1);
	SetButtonValue(2,24,0);
	SetButtonValue(2,25,0);
	SetButtonValue(2,26,0);
}

//��ʾZ��ѡ��״̬
void Show_Z_Axis_State(void)
{
  SetButtonValue(0,42,0);           //Z��ѡ��״̬
	SetButtonValue(0,43,0);
	SetButtonValue(0,44,1);
	SetButtonValue(0,45,0);
	SetButtonValue(0,46,0);
	
  SetButtonValue(2,22,0);
	SetButtonValue(2,23,0);
	SetButtonValue(2,24,1);
	SetButtonValue(2,25,0);
	SetButtonValue(2,26,0);
}

//��ʾA��ѡ��״̬
void Show_A_Axis_State(void)
{
  SetButtonValue(0,42,0);           //A��ѡ��״̬
	SetButtonValue(0,43,0);
	SetButtonValue(0,44,0);
	SetButtonValue(0,45,1);
	SetButtonValue(0,46,0);
	
  SetButtonValue(2,22,0);
	SetButtonValue(2,23,0);
	SetButtonValue(2,24,0);
	SetButtonValue(2,25,1);
	SetButtonValue(2,26,0);
}

//��ʾB��ѡ��״̬
void Show_B_Axis_State(void)
{
  SetButtonValue(0,42,0);           //B��ѡ��״̬
	SetButtonValue(0,43,0);
	SetButtonValue(0,44,0);
	SetButtonValue(0,45,0);
	SetButtonValue(0,46,1);
	
  SetButtonValue(2,22,0);
	SetButtonValue(2,23,0);
	SetButtonValue(2,24,0);
	SetButtonValue(2,25,0);
	SetButtonValue(2,26,1);
}
