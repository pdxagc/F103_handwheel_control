
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


#define Start 1
#define Stop  0
#define TIME_100MS 10                                //100����(10����λ)
volatile uint32  timer_tick_count = 0;               //��ʱ������
uint8  cmd_buffer[CMD_MAX_SIZE];                     //ָ���
uint8  USART2_RX_STA=0;                              //USART2����״̬���


uint8  Work_Page_Status = 0;              //����ҳ����
uint8  get_cmd_type;                     //��ȡָ������
uint8  get_ctrl_msg;                     //��ȡ��Ϣ������
uint8  get_button_state;                 //��ȡ��ť״̬
uint8  get_control_type;                 //��ȡ�ؼ�����
uint16 get_screen_id;                    //��ȡ����ID
uint16 get_control_id;                   //��ȡ�ؼ�ID
uint32 get_value;                        //��ȡ��ֵ
uint8  input_buf[20];                    //������������

int32 Pulses_counter;              // ������������
uint8 first_time_power_on=0;       //������һ�ο�����־λ
uint8 Send_cooddinate_status;      //����������λ
int32 Working_line;                //�ӹ�����

int32 Pulses_num_temp;             //��ʱ������������
uint8 file_name[20];               //�ļ���
char  Working_line_buf[10];        //����ӹ�����

State state;                             //��������״̬�ṹ�����
Override override;                       //����������ؽṹ��
Speed_Control Speed;                     //���������ٶ���صĽṹ�����
Pram_Status pram_status;                 //��������ҳ����ز����Ľṹ�����
Control_Panel_Pram control_panel_pram;   //�������������ز����Ľṹ�����


/******************************************************************************************************/
/*!                                                                                 
*  \brief  �������                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
{  
	qsize  size = 0;          //ָ��� 

	Set_System();             //����ʱ��                                                                                                                                 
	Interrupts_Config();      //���ô����ж�                                                                                                                             
	systicket_init();         //����ʱ�ӽ���	
	Usart1_Init(115200);      //����1��ʼ��(��TFT��ͨѶ)
  Usart2_Init(115200);      //����2��ʼ��(���̻�ͨѶ)
	queue_reset();            //��մ��ڽ��ջ����� 
	TIME2_Init();             //��ʱ��2��ʼ��(��������������)
	TIME3_Init();             //��ʱ��3��ʼ��(������ѯ������)
  TIME4_Init();             //��ʱ��4��ʼ��(��������)
	TIM_Cmd(TIM4, DISABLE);   //�ر�TIM4��ʱ��
	                                                                                               
	delay_ms(300);            //��ʱ�ȴ���������ʼ�����,����ȴ�300ms  
	
	Power_On_Set();                 //���������Ͳ�����ʼ������
		
	Setting_page_pram_get();        //����ҳ����ز���ֵ�ָ����ϵ籣����flash��
	
  if(first_time_power_on==0)      //�ϵ���һ�ο���ִ���������
  {
		Return_last_status();         //�ָ���һ������״̬
		first_time_power_on=1;
  }
	
		
		//    �ر�ע��
		//    MCU��ҪƵ���򴮿����������ݣ����򴮿������ڲ��������������Ӷ��������ݶ�ʧ(��������С����׼��8K��������4.7K)
		//    1) һ������£�����MCU�򴮿����������ݵ����ڴ���100ms���Ϳ��Ա������ݶ�ʧ�����⣻
		//    2) �����Ȼ�����ݶ�ʧ�����⣬���жϴ�������BUSY���ţ�Ϊ��ʱ���ܷ������ݸ���������
	
	while(1)                                                                        
	{
		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);                  //��USART1�������л�ȡһ��ָ��õ�ָ���       
		if(size>0 && cmd_buffer[1]!=0x07)                                  //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
		{                                                                           
			ProcessMessage((PCTRL_MSG)cmd_buffer, size);                   //ָ��������� ���ж�����������ĸ�Work_Page_Status
			memset(cmd_buffer, 0, CMD_MAX_SIZE);
		} 
		
		
    if(USART2_RX_STA)                                               //����2���յ�������
		{
		  uart2_command_handle();                                       //���ݷ���		
		}


		if(state.Work_state==Start)          //��ʾ��ǰ���ڼӹ�״̬
		{
			WorkingStatus_Starting();       
		}
		else                                 //��ʾ��ǰ�Ѿ�ֹͣ�ӹ�
		{
			WorkingStatus_Stoped();  
		}


		switch(Work_Page_Status)
		{
		
			case Working_Page: //*********************************************************�ӹ�ҳ��***************************************************************************************************
			{	
        				
				Spindle_and_Work_Speed_Key_Process();	           	//�ӹ����������ٶȺͼӹ��ٶȰ�ť����
				if(state.Work_state==Start)                        //�������ڼӹ�״̬��
				{
					  TFT_Show_coordanate_value();			            //��ʾ�����ͻ�е����
						SetTextValue(0,21,(uchar *)file_name);        //��ʾ���ڼ��ص��ļ���			
						sprintf(Working_line_buf,"%d",Working_line);  
						SetTextValue(0,22,(uchar *)Working_line_buf); //��ʾ�ӹ���������Ҫ������ѯ��
				}						    							       			
			}break;
		
		 case Setting_page:  //***********************************************************����ҳ��*************************************************************************************************
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
				if(state.Work_state==Stop)      //ֹͣ�ӹ�
				{
					if(pram_status.Screen_ID21_Setting_Sure)                    //��ȷ����ť������
					{
						Speaker_Key_Process(pram_status.voice_button_status);     //������ʾ��ť������
						Safe_Z_process(pram_status.Safe_Z_button_satus);           //��ȫZ����������
						Auto_Knife_process(pram_status.Auto_Knife_button_status);  //�Զ��Ե���ť�����������
						Unit_Change_process(pram_status.Unit_Change_button_status); //��λ�л���ť�����������
						Work_Page_Status=Working_Page;
					}
					if(pram_status.Screen_ID21_Setting_concel)                    //ȡ����ť����
					{
						Return_last_status();                                      //�ָ���һ������״̬
						Work_Page_Status=Working_Page;					
					}	
			  }				
			}
			break;	
			case ControlPanel_Page:  //******************************************************�������ҳ��*****************************************************************************************
			{
				char buf2[20];
        if(state.Work_state==Stop)	                          //ֹͣ�ӹ�
				{					
					if(control_panel_pram.X_press)                       //X��ѡ��״̬***************************************************************                   
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_X != override.Override_num)   //�����л�
							{	 
							   control_panel_pram.X_value_temp = control_panel_pram.X_value;
								 override.Override_num_temp_X = override.Override_num;
								 TIM4->CNT = 0;
								 Pulses_counter = 0;
							   Pulses_num_temp = 0;
							}
              Get_Pulses_num();   //�����������							
							control_panel_pram.X_Pulses_counter=Pulses_counter;						
							control_panel_pram.X_value=control_panel_pram.X_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.X_value);
							SetTextValue(2,17,(uchar *)buf2);	        //����������ʾ��������
							SetTextValue(0,16,(uchar *)buf2);         //����������ʾ��������
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //���㰴ť����
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
							Pulses_num_temp=0;
							control_panel_pram.X_value_temp=0;
							control_panel_pram.X_Pulses_counter=0;							
              X_coordinate_clear();                    //��X�Ṥ����������	
							control_panel_pram.Clear_Button=0;						
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //ȫ�����㴥��
						{
							
							Pulses_num_Clear();                    //������������
						  All_Workpiece_coordinate_clear();      //�������Ṥ����������							
							XYZAB_button_reset();                  //XYZAB���갴ť��λ
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
						
					}
					else if(control_panel_pram.Y_press)                   //Y��ѡ��״̬*******************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_Y!= override.Override_num)   //�����л�
							{	 
							   control_panel_pram.Y_value_temp=control_panel_pram.Y_value;
								 override.Override_num_temp_Y=override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.Y_Pulses_counter=Pulses_counter;
							control_panel_pram.Y_value=control_panel_pram.Y_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.Y_value);
							SetTextValue(2,18,(uchar *)buf2);
							SetTextValue(0,17,(uchar *)buf2);         //����������ʾ��������
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //���㰴ť����
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.Y_value_temp=0;
							control_panel_pram.Y_Pulses_counter=0;
							delay_ms(20);							
              Y_coordinate_clear();                   //��Y�Ṥ����������
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //ȫ�����㴥��
						{
							Pulses_num_Clear();                    //������������
						  All_Workpiece_coordinate_clear();      //�����й�����������														
							XYZAB_button_reset();                   //XYZAB���갴ť��λ
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
						
					}
					else if(control_panel_pram.Z_press)                   //Z��ѡ��״̬**********************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_Z != override.Override_num)   //�����л�
							{	 
							   control_panel_pram.Z_value_temp=control_panel_pram.Z_value;
								 override.Override_num_temp_Z = override.Override_num;
								 TIM4->CNT=0;
							 	 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.Z_Pulses_counter=Pulses_counter;
							control_panel_pram.Z_value=control_panel_pram.Z_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.Z_value);
							SetTextValue(2,19,(uchar *)buf2);
							SetTextValue(0,18,(uchar *)buf2);        //����������ʾ��������
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //���㰴ť����
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.Z_value_temp=0;
							control_panel_pram.Z_Pulses_counter=0;
							delay_ms(20);						
              Z_coordinate_clear();         //��Z�Ṥ����������
							control_panel_pram.Clear_Button=0;						
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //ȫ�����㴥��
						{
							Pulses_num_Clear();                    //������������
						  All_Workpiece_coordinate_clear();      //�����й�����������								
							XYZAB_button_reset();                  //XYZAB���갴ť��λ
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
					
					}
					else if(control_panel_pram.A_press)                   //A��ѡ��״̬************************************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_A != override.Override_num)   //�����л�
							{	 
							   control_panel_pram.A_value_temp = control_panel_pram.A_value;
								 override.Override_num_temp_A = override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.A_Pulses_counter=Pulses_counter;
							control_panel_pram.A_value=control_panel_pram.A_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.A_value);
							SetTextValue(2,20,(uchar *)buf2);
							SetTextValue(0,19,(uchar *)buf2);         //����������ʾ��������
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //���㰴ť����
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.A_value_temp=0;
							control_panel_pram.A_Pulses_counter=0;
							delay_ms(20);
							
              A_coordinate_clear();                  //��A�Ṥ����������
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //ȫ�����㴥��
						{
							Pulses_num_Clear();                    //������������
						  All_Workpiece_coordinate_clear();      //�����й�����������								
							XYZAB_button_reset();                  //XYZAB���갴ť��λ
							control_panel_pram.All_Spindle_Clear_Button=0;
						}
																							
					}
					else if(control_panel_pram.B_press)                   //B��ѡ��״̬*****************************************************************************
					{
						if(control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button==0)
						{
							if(override.Override_num_temp_B!= override.Override_num)   //�����л�
							{	 
							   control_panel_pram.B_value_temp=control_panel_pram.B_value;
								 override.Override_num_temp_B= override.Override_num;
								 TIM4->CNT=0;
								 Pulses_counter=0;
							   Pulses_num_temp=0;
							}
							Get_Pulses_num();
							control_panel_pram.B_Pulses_counter=Pulses_counter;
							control_panel_pram.B_value=control_panel_pram.B_value_temp+Pulses_counter*override.Override_num;
							sprintf((char *)buf2,"%09.2f",control_panel_pram.B_value);
							SetTextValue(2,21,(uchar *)buf2);
							SetTextValue(0,20,(uchar *)buf2);         //����������ʾ��������
							
						}
						if(control_panel_pram.Clear_Button && control_panel_pram.All_Spindle_Clear_Button==0)    //���㰴ť����
						{
							TIM4->CNT = 0;
							Pulses_counter=0;
	            Pulses_num_temp=0;
							control_panel_pram.B_value_temp=0;
							control_panel_pram.B_Pulses_counter=0;
							delay_ms(20);		
              B_coordinate_clear();                      	//��B�Ṥ����������
							control_panel_pram.Clear_Button=0;
						}
						if( control_panel_pram.Clear_Button==0 && control_panel_pram.All_Spindle_Clear_Button)  //ȫ�����㴥��
						{
							Pulses_num_Clear();                  //������������
						  All_Workpiece_coordinate_clear();    //�����й�����������								
							XYZAB_button_reset();                //XYZAB���갴ť��λ
							control_panel_pram.All_Spindle_Clear_Button=0;
						}				
					}
					
					if(control_panel_pram.X_press || control_panel_pram.Y_press || control_panel_pram.Z_press || control_panel_pram.A_press || control_panel_pram.B_press)
					{
						TIM_Cmd(TIM2, ENABLE);                           //ʹ��TIM2����ʱ��������������
						if(Send_cooddinate_status)
						{
					     Send_Coordinate_to_Host_Machine();            //��������������	
							 Send_cooddinate_status=0;
						}							
					}
					else 
					{
					   TIM_Cmd(TIM2, DISABLE);
					}
          					
			  }
				else                                                 //�������ڼӹ�״̬��
				{
					 TIM_Cmd(TIM3, ENABLE);                        //ʹ�� TIM3
					// TIM_Cmd(TIM4, DISABLE);                       //��ֹTIM4
					 TFT_Show_coordanate_value();			             //��ʾ�����ͻ�е����
					
					 SetTextValue(0,21,(uchar *)file_name);        //��ʾ���ڼ��ص��ļ���	         				
					 sprintf(Working_line_buf,"%d",Working_line);  
					 SetTextValue(0,22,(uchar *)Working_line_buf); //��ʾ�ӹ���������Ҫ������ѯ��
				
				}
				
			}break;
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

		

				
	}  
}




/***
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{

	
    get_cmd_type = msg->cmd_type;                                         //��ȡָ������
    get_ctrl_msg = msg->ctrl_msg;                                         //��ȡ��Ϣ������	  
    get_control_type = msg->control_type;                                 //��ȡ�ؼ�����
    get_screen_id = PTR2U16(&msg->screen_id);                             //��ȡ����ID
    get_control_id = PTR2U16(&msg->control_id);                           //��ȡ�ؼ�ID
    get_value = PTR2U32(msg->param);                                      //��ȡ��ֵ
	
	  if(get_control_type==kCtrlButton)                                      //��ť�ؼ�
			get_button_state= msg->param[1];                                     //��ȡ��ť״̬


	  //sprintf((char *)input_buf,"%s",msg->param);
	   
    if(get_cmd_type==NOTIFY_CONTROL)                                      //�ؼ�����֪ͨ
    {  
				switch(get_screen_id)                                             //����ID
				{
					case 0:                                       //����0
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
													Speed.Initial_Work_Speed_Percent  -=25;						
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
									default:break;
						  }
					}break;
					case 1:                                       //����1
					{
							Work_Page_Status=Setting_page;
						if(state.Work_state==Stop)    //ֹͣ�ӹ�״̬
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
									case 6:
									{
										char buf[20];
										pram_status.Safe_Z_num=NotifyText(msg->param);										
										sprintf(buf,"Safe_Z:%.2f",pram_status.Safe_Z_num);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 7:
									{
										char buf[20];
										pram_status.Knife_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_high:%.2f",pram_status.Knife_high_num);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 8: 
									{
										char buf[20];
										pram_status.Knife_block_high_num=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_high:%.2f",pram_status.Knife_block_high_num);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 9: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_X=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_X:%.2f",pram_status.Auto_Knife_block_X);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 10: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_Y=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Y:%.2f",pram_status.Auto_Knife_block_Y);
		                Usart_SendString(USART2,(char *)buf);       
									}										
									break;
									case 11: 
									{
										char buf[20];
										pram_status.Auto_Knife_block_Z=NotifyText(msg->param);										
										sprintf(buf,"Knife_block_Z:%.2f",pram_status.Auto_Knife_block_Z);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 12: 
									{
										char buf[20];
										pram_status.Soft_limit_X=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_X:%.2f",pram_status.Soft_limit_X);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 13: 
									{
										char buf[20];
										pram_status.Soft_limit_Y=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Y:%.2f",pram_status.Soft_limit_Y);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 14: 
									{
										char buf[20];
										pram_status.Soft_limit_Z=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_Z:%.2f",pram_status.Soft_limit_Z);
		                Usart_SendString(USART2,(char *)buf);      
									}										
									break;
									case 15: 
									{
										char buf[20];
										pram_status.Soft_limit_A=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_A:%.2f",pram_status.Soft_limit_A);
		                Usart_SendString(USART2,(char *)buf);     
									}										
									break;
									case 16: 
									{
										char buf[20];
										pram_status.Soft_limit_B=NotifyText(msg->param);										
										sprintf(buf,"Soft_limit_B:%.2f",pram_status.Soft_limit_B);
		                Usart_SendString(USART2,(char *)buf);       
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
					case 2:                                       //����2
					{
							Work_Page_Status=ControlPanel_Page;
							switch(get_control_id)
							{
                  case 1:                                           //���㰴ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                          //ֹͣ�ӹ�
										{
											control_panel_pram.Clear_Button=1;     
										}
									}break;    
                  case 2:                                             //�ػ�е�㰴ť����	
                  {
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
										}
									}break;                                     									
									case 3:								                              //�����л���ť����
									{
										if(state.Work_state==Start)      //��ʼ�ӹ�
										{
										}
										else                             //ֹͣ�ӹ�
										{
											if(get_button_state)
											{
													control_panel_pram.Override_Change_button=1;
													Override_Change_Process();//�����л�����											
											}
											get_button_state=0;	
										}											
									}break;
								  case 4:                                             //���Ὺ�ذ�ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
										}
									}break;
			            case 5:                                              //ȫ�����㰴ť����
									{
                    if(state.Work_state==Start)     //��ʼ�ӹ�
										{
										}
										else                            //ֹͣ�ӹ�
										{										
												control_panel_pram.All_Spindle_Clear_Button=1;
												TIM_Cmd(TIM4, DISABLE);       //�ر�TIM4��ʱ��
										}
									}break;
									case 6:                                            //�ع����㰴ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
											Work_Page_Status=Return_WorkPiece_Zero_Page;
										}
									}break;
									case 7:                                            //�����л���ť����
									{
										if(state.Work_state==Start)     //��ʼ�ӹ�
										{
										}
										else                           //ֹͣ�ӹ�
										{
											if(get_button_state)
											{										
												  Coordinate_Change_Process();//�����л�����
												  get_button_state=0;
											 }
										}											
									}break;
									case 8:                                            //����λ���ذ�ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
										}
									}break;
									case 9:                                            //��ȫZ��ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
										}
									}break;
									case 10:                                         //���мӹ���ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 11:                                          //�Ե���ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 12:                                         //���а�ť����
									{
										if(state.Work_state==Start)    //��ʼ�ӹ�
										{									
										}
										else                           //ֹͣ�ӹ�
										{
											Work_Page_Status=Jump_Work_Page;
										}
									}break;
									case 13:                                           //��ʼ��ť����
									{
										state.Work_state=Start;
										TIM_Cmd(TIM3, ENABLE);      //ʹ�� TIM3
										TIM_Cmd(TIM4, DISABLE);      //��ֹ TIM4
										SetButtonValue(2,14,0);     //ֹͣ��ť�ɿ�״̬
										SetButtonValue(2,13,1);     //��ʼ��ť����״̬										
									}break;
									case 14:                                           //ֹͣ��ť����
									{
										state.Work_state=Stop;
										TIM_Cmd(TIM3, DISABLE);     //��ֹ TIM3
										SetButtonValue(2,13,0);     //��ʼ��ť�ɿ�״̬
										SetButtonValue(2,14,1);     //ֹͣ��ť����״̬
                    if(control_panel_pram.X_press || control_panel_pram.Y_press || control_panel_pram.Z_press || control_panel_pram.A_press || control_panel_pram.B_press)
										{
										  TIM_Cmd(TIM4, ENABLE);      //ʹ�� TIM4
										}											
									}break;
									case 15:                                            //��λ��ť����
									{
										state.Work_state=Stop;
										TIM_Cmd(TIM3, DISABLE);      //��ֹ TIM3
										SetButtonValue(2,13,0);      //��ʼ��ť�ɿ�״̬
										SetButtonValue(2,14,1);      //ֹͣ��ť����״̬						
									}break;
									case 16:Work_Page_Status=Working_Page;break;
									case 22:                                            //X�ᰴť����
									{	
										SetButtonValue(2,22,1);     //X�ᱣ��ѡ��״̬
										TIM_Cmd(TIM4, ENABLE);      //����TIM4��ʱ��									
										if(control_panel_pram.X_press==0)
										{
											TIM4->CNT = control_panel_pram.X_Pulses_counter*4;
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=1;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;									
									}break;
									case 23:                                            //Y�ᰴť����
									{
										SetButtonValue(2,23,1);        //Y�ᱣ��ѡ��״̬
										TIM_Cmd(TIM4, ENABLE);         //����TIM4��ʱ��
										if(control_panel_pram.Y_press==0)
										{
											 TIM4->CNT = control_panel_pram.Y_Pulses_counter*4;
                       Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=1;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;
							
									}break;
									case 24:                                              //Z�ᰴť����
									{
										SetButtonValue(2,24,1);         //Z�ᱣ��ѡ��״̬
										TIM_Cmd(TIM4, ENABLE);          //����TIM4��ʱ��
										if(control_panel_pram.Z_press==0)
										{
											TIM4->CNT = control_panel_pram.Z_Pulses_counter*4;
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=1;
										control_panel_pram.A_press=0;
										control_panel_pram.B_press=0;						
									}break;
									case 25:                                             //A�ᰴť����
									{
										SetButtonValue(2,25,1);         //A�ᱣ��ѡ��״̬
										TIM_Cmd(TIM4, ENABLE);          //����TIM4��ʱ��
										if(control_panel_pram.A_press==0)
										{
											TIM4->CNT = control_panel_pram.A_Pulses_counter*4;	
                      Pulses_counter=0;											
										}
										control_panel_pram.X_press=0;
										control_panel_pram.Y_press=0;
										control_panel_pram.Z_press=0;
										control_panel_pram.A_press=1;
										control_panel_pram.B_press=0;						
									}break;
									case 26:                                           //B�ᰴť����
									{
										  SetButtonValue(2,26,1);     //B�ᱣ��ѡ��״̬
											TIM_Cmd(TIM4, ENABLE);      //����TIM4��ʱ��
											if(control_panel_pram.B_press==0)
											{
												TIM4->CNT = control_panel_pram.B_Pulses_counter*4;
                        Pulses_counter=0;												
											}
											control_panel_pram.X_press=0;
											control_panel_pram.Y_press=0;
											control_panel_pram.Z_press=0;
											control_panel_pram.A_press=0;
											control_panel_pram.B_press=1;
									}break;
									default:break;				
							}
					}break;
					case 3: Work_Page_Status=Return_WorkPiece_Zero_Page;break;  //����3
					case 5: Work_Page_Status=Jump_Work_Page;break;              //����5
					case 6:                                                     //����6
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
					case 7: Work_Page_Status=Leading_In_Page;break;            //����7
					case 9: Work_Page_Status=Delete_Page;break;                //����8
					case 10: Work_Page_Status=Storage_View_Page;break;         //����10
					case 11:                                                   //����10
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
					case 12: Work_Page_Status=Choose_WiFi_Page;break;
					case 13: Work_Page_Status=Disconnet_and_SignIn_Page;break;
					case 14: Work_Page_Status=Disconnect_and_SignOut_Page;break;
					case 15: Work_Page_Status=Disconnect_Remind_Page;break;
					case 16: Work_Page_Status=SignOut_Remind_Page;break;
					case 20: Work_Page_Status=Leading_Out_Pgae;break;
					case 21:                                                  //����21
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
		sscanf((char *)str,"%f",&value);                                            //���ַ���ת��Ϊ���� 
     return  value;                                                                      
}                                                                                

