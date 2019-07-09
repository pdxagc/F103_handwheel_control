
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



volatile uint32  timer_tick_count = 0;               //��ʱ������
uint8  cmd_buffer[CMD_MAX_SIZE];                     //LCDָ���

uint8  TX_Data [30]; //the sending package    �洢����3Ҫ���͵����ݣ��������ͣ�
uint8  RX_Data [30]; //the receiving package  ���մ���3���������ݣ����ж��н��գ�

uint8  Work_Page_Status = 0;             //����ҳ����
uint8  get_cmd_type;                     //��ȡָ������
uint8  get_ctrl_msg;                     //��ȡ��Ϣ������
uint8  get_button_state;                 //��ȡ��ť״̬
uint8  get_control_type;                 //��ȡ�ؼ�����
uint16 get_screen_id;                    //��ȡ����ID
uint16 get_control_id;                   //��ȡ�ؼ�ID
uint32 get_value;                        //��ȡ��ֵ
uint8  input_buf[20];                    //������������

uint8 last_time_work_state=0;      //��¼��һ�λ�������״̬,Ĭ���ǹػ�״̬
uint16 Pulses_counter;              // ������������
uint8 Override_num;                //����
uint8 first_time_re_workpiece;     //�״ν���ع�����ҳ��
uint8 Pulses_check=1;              //����ͬ����־λ
uint8 Pulses_count_mark=1;           //����������ܱ��λ
int32 Working_line;                //�ӹ�����
uint8 Mark_10ms=0;                 //10ms��ʱ���λ
uint8 Mark_10ms_Count;             //10ms��ʱ���ͳ��λ
uint8 Mark_20ms=0;                 //20ms��ʱ���λ
uint8 Mark_20ms_Count=0;           //20ms��ʱ
uint8 Mark_60ms=0;                //100ms��ʱ���λ

uint8 file_name[20]="�������";    //�ļ���
char  Working_line_buf[20];        //����ӹ�����

State state;                               //��������״̬�ṹ�����
Speed_Control Speed;                       //���������ٶ���صĽṹ�����
Pram_Status pram_status;                   //��������ҳ����ز����Ľṹ�����
Control_Panel_Pram control_panel_pram;      //�������������ز����Ľṹ�����
Return_Workpiece_Zero return_workpiece_zero;//�����ع�������ز����Ľṹ�����
Devide_Set devide_set;                     //��������������ز����Ľṹ�����
Jump_Work_Set jump_work_set;               //�������мӹ���ز����Ľṹ�����

/******************************************************************************************************/
/*!                                                                                 
*  \brief  �������                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
 { 
  uint16 time_conuter=0;	
  uint16 Recdata1,Recdata2;	 
	uint8  check_time=0;      //����ͬ��������
	Set_System();             //����ʱ��                                                                                                                                                                                                                                                              
	systicket_init();         //����ʱ�ӽ���
	Usart1_Init(115200);      //����1��ʼ��(��LCD��ͨѶ)	 	 
	queue_reset();            //��մ��ڽ��ջ����� 
	TIME2_Init();             //��ʱ��2��ʼ��
	//TIME3_Init();             //��ʱ��3��ʼ��(������ѯ������)
  TIME4_Init();             //��ʱ��4��ʼ��(������������)                                                                                              
	delay_ms(300);            //��ʱ�ȴ���������ʼ�����,����ȴ�300ms  
	Usart3_Init(115200);      //����3��ʼ��(���̻�ͨѶ)
	
	 
	Power_On_Set();                 //���������������Ͳ�����ʼ������	
	Setting_page_pram_get();        //��ȡ����������ҳ����ز���ֵ��������flash��	
	Return_last_status();           //�ָ���һ�δ���������ҳ����ز���ֵ

		//    MCU��ҪƵ���򴮿����������ݣ����򴮿������ڲ��������������Ӷ��������ݶ�ʧ(��������С����׼��8K��������4.7K)
		//    1) һ������£�����MCU�򴮿����������ݵ����ڴ���100ms���Ϳ��Ա������ݶ�ʧ�����⣻
		//    2) �����Ȼ�����ݶ�ʧ�����⣬���жϴ�������BUSY���ţ�Ϊ��ʱ���ܷ������ݸ���������
		
	while(Pulses_check)        //����ͬ����������
	{
		if(RX_Data[1] == CMD_UPDATE_MACH3_NUMBER)
		{
			check_time++;
			Recdata1=RX_Data[18];
			Recdata2=RX_Data[19];	
			TIM4->CNT = (Recdata1<<8)+Recdata2;  //ͬ������
      if(check_time>5)
        Pulses_check=0;
			
      sprintf(Working_line_buf,"%u",Pulses_counter);	
			SetTextValue(0,22,(uchar *)Working_line_buf);     //��ʾ�ӹ���������Ҫ������ѯ��			
		}
	}
	
	while(1)                                                                        
	{
		//time_conuter++;
		
		Pulses_Count();                        //������������
		Usart3_Receive_Data_handle();          //���̻�ͨѶ������������� 
		
//		if(Mark_10ms)
//		{
//  	   			
//			Mark_10ms=0;
//		}	
		if(Mark_20ms) 
		{
		  LCD_command_analyse();                 //����LCD�������� 			  
			Mark_20ms=0;
		}
		if(Mark_60ms)                         //��ʱ��100ms
		{
			LCD_Show_coordanate_value();        //��ʾ��������
			Mark_60ms=0;
		}
		LCD_handle();                          //����LCD������
			
    	
//		sprintf(Working_line_buf,"%d",Pulses_counter);  
//		SetTextValue(0,23,(uchar *)Working_line_buf);     //��ʾ�ӹ���������Ҫ������ѯ��
	}  
}





//������LCD����������з���
void LCD_command_analyse(void)
{
	qsize  size = 0;                                             //ָ��� 
  size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);              //���յ�LCD�������ݣ���USART1��ָ�����cmd_buffer�л�ȡһ��ָ��õ�ָ���       
	if(size>0 && cmd_buffer[1]!=0x07)  //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
	{                                                                           
		Usart1_Receive_data_handle((PCTRL_MSG)cmd_buffer, size);           //ָ��������� �����Ӧ�ý����ĸ�Work_Page_Status�������Ӧ�Ĳ���λ
		memset(cmd_buffer, 0, CMD_MAX_SIZE);                       //��ָ���cmd_buffer����
	}
}


//���������������
void Pulses_Count(void)
{
	if( Work_Page_Status == Working_Page || Work_Page_Status == ControlPanel_Page )
	{
		if(Pulses_count_mark)
		{
		  TIM_Cmd(TIM4, ENABLE);
			Pulses_count_mark=0;
		}
		Get_Pulses_num();   
		sprintf(Working_line_buf,"%u",Pulses_counter);  
		SetTextValue(0,22,(uchar *)Working_line_buf);     //��ʾ������� 
	}
	else
	{
	  TIM_Cmd(TIM4, DISABLE);          //��ֹ TIM4�����岻����
		Pulses_count_mark=1;
	}
	

}
/************************************************************
*
*��������ǵĹ���ҳ�棬�����������
*
*************************************************************/
void LCD_handle(void)
{
	
  Work_state_control();   //��������״̬��ʾ
	
	switch(Work_Page_Status)
	{	
		case Working_Page: //*********************************************************�ӹ�ҳ��***************************************************************************************************
		{	
			
			//LCD_Show_coordanate_value();		                  //��������ʾ�����ͻ�е����	
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
		}
	  	break;
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
		}
		  break;	
		case ControlPanel_Page:  //******************************************************�������ҳ��*****************************************************************************************
		{	
			//Get_Pulses_num();                                   //����������� 
		  //LCD_Show_coordanate_value();		                    //��������ʾ�����ͻ�е����
			
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
			break;
		case Leading_In_Page:    //***************************************************** ����ҳ��****************************************************************************************
			break;
		case Delete_Page:         //*****************************************************ɾ��ҳ��****************************************************************************************
			break;
		case Storage_View_Page:    //*****************************************************�ڴ�Ԥ��ҳ��****************************************************************************************
			break;	
		case Net_Account_Manage_Page:  //**************************************************�������˻���¼����**********************************************************************************
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
																																																																																																
					default:
							 break;
				}
			
			
			}
			break;
		case Choose_WiFi_Page:        //*****************************************************ѡ��WiFiҳ��****************************************************************************************
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

}
	

//��������״̬��ʾ
void Work_state_control(void)
{
  if(last_time_work_state!=state.Work_state)     //�жϹ���״̬�Ƿ����仯�������仯����д���
	{
		if(state.Work_state==Start)         
		{
			Show_Start_Working(Work_Page_Status);      //��������ʾ��ǰ���ڼӹ�״̬
		}
		else                                 
		{
			Show_Stop_Working(Work_Page_Status);        //��������ʾ��ǰ�Ѿ�ֹͣ�ӹ�
		}
		last_time_work_state=state.Work_state;
	}

}

/****************************************************
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
******************************************************/
void Usart1_Receive_data_handle( PCTRL_MSG msg, uint16 size )
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
					case Working_Page:                                       //����0���ӹ�ҳ��
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
										control_panel_pram.Axis_press= CMD_X_AXIS;								
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
					case Setting_page:                                       //����1������ҳ��
					{
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
										char buf[20];
										pram_status.Safe_Z_num=NotifyText(msg->param);										
										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);    
									}										
									break;
									case 7:                   //��ȡ�Ե��߶�
									{
										char buf[20];
										pram_status.Knife_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8:                     //��ȡ�Ե���߶�
									{
										char buf[20];
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9:                  //��ȡ�Ե���X��λ��
									{
										char buf[20];
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10:                 //��ȡ�Ե���Y��λ��
									{
										char buf[20];
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
//		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11:                 //��ȡ�Ե���Z ��λ��
									{
										char buf[20];
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12:                  //��ȡ����λX��λ��
									{
										char buf[20];
										pram_status.Soft_limit_X=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13:                   //��ȡ����λY��λ��
									{
										char buf[20];
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14:                 //��ȡ����λZ��λ��
									{
										char buf[20];
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
//		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15:                 //��ȡ����λA��λ��
									{ 
										char buf[20];
										pram_status.Soft_limit_A=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
//		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16:                  //��ȡ����λB��λ��
									{
										char buf[20];
										pram_status.Soft_limit_B=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
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
					case ControlPanel_Page:                                       //����2���������
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //���㰴ť����
									{
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Clear;								
										}																					
									}break;    
                  case 2:                                             //�ػ�е�㰴ť����	
                  {
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Return_Machine_Zero;
																								
										}
										get_button_state=0;	
									}break;                                     									
									case 3:								                              //�����л���ť����
									{
										if(get_button_state)
										{
												control_panel_pram.Override_Change_button=1;
												Override_Change_Process();                    //�����л�����
												control_panel_pram.Press_button = CMD_Override_Change;												
										}
										get_button_state=0;										
									}break;
								  case 4:                                             //���Ὺ�ذ�ť����
									{
										if(get_button_state)
										{
											control_panel_pram.Press_button = CMD_Spin_On_Off;						
										}
										get_button_state=0;	
									}break;
			            case 5:                                              //ȫ�����㰴ť����
									{
                    if(get_button_state)   
										{										
											control_panel_pram.All_Spindle_Clear_Button=1;
											control_panel_pram.Press_button = CMD_All_Spin_Clear;
											get_button_state=0;
										}
									}break;
									case 6:                                            //�ع����㰴ť����
									{
										if(get_button_state)    
										{
											Work_Page_Status=Return_WorkPiece_Zero_Page;
											first_time_re_workpiece=1;
											control_panel_pram.Press_button = CMD_Return_WorkPiece_Zero;
											get_button_state=0;
										}
									}break;
									case 7:                                            //�����л���ť����
									{
											if(get_button_state)
											{										
												  Coordinate_Change_Process();//�����л�����
												  control_panel_pram.Press_button = CMD_Coordinate_Change;
												  get_button_state=0;
											 }										
									}break;
									case 8:                                            //����λ���ذ�ť����
									{											
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Soft_Limit;						
											}
											get_button_state=0;	
									}break;
									case 9:                                            // ��ȫZ����
									{
										if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Safe_Z;						
											}
											get_button_state=0;	
									}break;
									case 10:                                         //���мӹ���ť����
									{
										if(get_button_state)
										{
											jump_work_set.First_get_into = 1;
											Work_Page_Status=Jump_Work_Page;
											control_panel_pram.Press_button = CMD_Jump_Work;
											get_button_state=0;
										}
									}break;
									case 11:                                          //�Ե���ť����
									{										
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Auto_knife;								
											}
											get_button_state=0;	
									}break;
									case 12:                                         //���а�ť����
									{
											if(get_button_state)
											{
												control_panel_pram.Press_button = CMD_Divided;								
											}
											get_button_state=0;	
									}break;
									case 13:                                           //��ʼ��ť����
									{
										state.Work_state=Start;
										control_panel_pram.Press_button = CMD_Start;
										TIM_Cmd(TIM4, DISABLE);      //��ֹ TIM4
//										SetButtonValue(2,14,0);     //ֹͣ��ť�ɿ�״̬
										SetButtonValue(2,13,1);     //��ʼ��ť����״̬										
									}break;
									case 14:                                           //ֹͣ��ť����
									{
										state.Work_state=Stop;
//										SetButtonValue(2,13,0);     //��ʼ��ť�ɿ�״̬
										SetButtonValue(2,14,1);     //ֹͣ��ť����״̬
                    control_panel_pram.Press_button = CMD_Stop;
									  TIM_Cmd(TIM4, ENABLE);      //ʹ�� TIM4
																	
									}break;
									case 15:                                            //��λ��ť����
									{
										state.Work_state=Stop;
										control_panel_pram.Press_button = CMD_Reset;  						
									}break;
									case 16:                                          //�˳�������尴ť����
									{
										Work_Page_Status=Working_Page;
									}
									break;
									case 22:                                            //X�ᰴť����
									{	
																											
										control_panel_pram.Axis_press= CMD_X_AXIS;								
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
					case Return_WorkPiece_Zero_Page:                                        //����3���ع�����ҳ��
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
					case Jump_Work_Page:                                         //����5:���мӹ�ҳ��
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
					case File_Manage_Page:                                               //����6:�ļ�����
					{
						Work_Page_Status=File_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Leading_In_Page;break;
							case 2: Work_Page_Status=Leading_Out_Pgae;break;
							case 3: Work_Page_Status=Delete_Page;break;
							case 4: Work_Page_Status=Storage_View_Page;break;
							case 5: Work_Page_Status=Working_Page;break;
							case 18:Work_Page_Status=Working_Page;break;
							case 19:Work_Page_Status=Net_Account_Manage_Page;break;
							default:break;
						}
					}break;
					case Leading_In_Page: Work_Page_Status=Leading_In_Page;break;            //����7������ҳ��
					case Leading_Out_Pgae: Work_Page_Status=Leading_Out_Pgae;break;            //����8������ҳ��
					case Delete_Page: Work_Page_Status=Delete_Page;break;                    //����9��ɾ��ҳ��
					case Storage_View_Page: Work_Page_Status=Storage_View_Page;break;         //����10���ڴ�ռ�Ԥ��ҳ��
					case Net_Account_Manage_Page:                                              //����11�������˻�����
					{
						Work_Page_Status=Net_Account_Manage_Page;
						switch(get_control_id)
						{
							case 1: Work_Page_Status=Working_Page;break;
							case 2: Work_Page_Status=File_Manage_Page;break;
							case 3: Work_Page_Status=Choose_WiFi_Page;break;
							default:break;
						}
					}break;
					case Choose_WiFi_Page: Work_Page_Status=Choose_WiFi_Page;break;           //����12��ѡ��WiFi
					case Disconnet_and_SignIn_Page: Work_Page_Status=Disconnet_and_SignIn_Page;break;   //����13���Ͽ�������������¼ҳ��
					case Disconnect_and_SignOut_Page: Work_Page_Status=Disconnect_and_SignOut_Page;break; //����14���Ͽ��������˳���¼ҳ��
					case Disconnect_Remind_Page: Work_Page_Status=Disconnect_Remind_Page;break;     //����15���Ͽ���������ҳ��
					case SignOut_Remind_Page: Work_Page_Status=SignOut_Remind_Page;break;         //����16���˳���¼����ҳ��
					
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

