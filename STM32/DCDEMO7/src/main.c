
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




volatile uint32  timer_tick_count = 0;               //¶¨Ê±Æ÷½ÚÅÄ

uint8 TX_Data[30]; //the sending package    ´æ´¢´®¿Ú3Òª·¢ËÍµÄÊı¾İ£¨Ö÷³ÌĞò·¢ËÍ£©
uint8 RX_Data[30]; //the receiving package  ½ÓÊÕ´®¿Ú3·¢À´µÄÊı¾İ£¨ÔÚÖĞ¶ÏÖĞ½ÓÊÕ£©
uint8 Pulses_check=1;              //Âö³åÍ¬²½´ÎÊı±êÖ¾Î»


uint8 Mark_10ms=0;                 //10ms¼ÆÊ±±ê¼ÇÎ»
uint8 Mark_20ms=0;                 //20ms¼ÆÊ±±ê¼ÇÎ»
uint8 Mark_30ms=0;                 //30ms¼ÆÊ±±ê¼ÇÎ»
uint8 Mark_200ms=0;                //200ms¼ÆÊ±±ê¼ÇÎ»
uint8 Mark_500ms=0;                //500ms¼ÆÊ±±ê¼ÇÎ»

uint8 Send_Estop_to_handwheel=1;   //¸øÊÖÂÖ·¢ËÍ½ô¼±Í£Ö¹ÏûÏ¢£¬1£º·¢ËÍ£¬0£º²»·¢ËÍ
uint8 Clear_Estop_massage=0;       //Çå³ıÊÖÂÖ½ô¼±Í£Ö¹ÏûÏ¢£»1£ºÇå³ı£¬0£º²»Çå³ı

uint8 Estop_button=Estop_Off;      //½ô¼±Í£Ö¹°´Å¥
uint8 Estop_Press_time=0;          //½ô¼±°´Å¥°´ÏÂ´ÎÊıÍ³¼Æ
uint8 Light_mark_button;           //°´¼ü´¥·¢±ê¼Ç 
uint8 last_time_light_mark_button; //¼ÇÂ¼ÉÏ´Î±³¹â´¥·¢±ê¼ÇÖµ

uint16 master_ask=0,master_send=0;

/******************************************************************************************************/
/*!                                                                                 
*  \brief  ³ÌĞòÈë¿Ú                                                                 
*/ 
/*******************************************************************************************************/
int main()                                                                          
 { 
	char buf4[20];
  uint16 time_conuter=0;	
  char bufrec[20],buf1[20];
  uint16 Recdata1,Recdata2,RecPulses;	 
	uint8  check_time=0;      //Âö³åÍ¬²½¼ì²é´ÎÊı
	 
	Set_System();             //ÅäÖÃÊ±ÖÓ                                                                                                                                                                                                                                                              
	systicket_init();         //ÅäÖÃÊ±ÖÓ½ÚÅÄ
	Usart1_Init(115200);      //´®¿Ú1³õÊ¼»¯(Óëµñ¿Ì»úÍ¨Ñ¶)
  Usart2_Init(115200);      //´®¿Ú2³õÊ¼»¯(ÓëTFTÆÁÍ¨Ñ¶) 	 
	queue_reset();            //Çå¿Õ´®¿Ú½ÓÊÕ»º³åÇø 
	TIME2_Init();             //¶¨Ê±Æ÷2³õÊ¼»¯
	Key_Init();               //°´¼ü³õÊ¼»¯	
	//EXTIX_Init();             //ÖĞ¶Ï³õÊ¼»¯
	TIME3_Init();             //¶¨Ê±Æ÷3³õÊ¼»¯(±³¹âÁÁ¶È¿ØÖÆ)
  TIME4_Init();             //¶¨Ê±Æ÷4³õÊ¼»¯(¼ÆËãÊÖÂÖÂö³å)
  IIC_Init();               //IIC³õÊ¼»¯
	delay_ms(300);            //ÑÓÊ±µÈ´ı´®¿ÚÆÁ³õÊ¼»¯Íê±Ï,±ØĞëµÈ´ı300ms
   
	//Usart3_Init(115200);      //´®¿Ú3³õÊ¼»¯
	
	 
	Power_On_Set();                 //´®¿ÚÆÁ¿ª»ú¶¯»­ºÍ²ÎÊı³õÊ¼»¯ÉèÖÃ	
	Get_Setting_page_pram();        //»ñÈ¡´®¿ÚÆÁÉèÖÃÒ³ÃæÏà¹Ø²ÎÊıÖµ£¨±£´æÔÚflash£©	
	Return_last_status();           //»Ö¸´ÉÏÒ»´Î´®¿ÚÆÁÉèÖÃÒ³ÃæÏà¹Ø²ÎÊıÖµ

		//    MCU²»ÒªÆµ·±Ïò´®¿ÚÆÁ·¢ËÍÊı¾İ£¬·ñÔò´®¿ÚÆÁµÄÄÚ²¿»º´æÇø»áÂú£¬´Ó¶øµ¼ÖÂÊı¾İ¶ªÊ§(»º³åÇø´óĞ¡£º±ê×¼ĞÍ8K£¬»ù±¾ĞÍ4.7K)
		//    1) Ò»°ãÇé¿öÏÂ£¬¿ØÖÆMCUÏò´®¿ÚÆÁ·¢ËÍÊı¾İµÄÖÜÆÚ´óÓÚ100ms£¬¾Í¿ÉÒÔ±ÜÃâÊı¾İ¶ªÊ§µÄÎÊÌâ£»
		//    2) Èç¹ûÈÔÈ»ÓĞÊı¾İ¶ªÊ§µÄÎÊÌâ£¬ÇëÅĞ¶Ï´®¿ÚÆÁµÄBUSYÒı½Å£¬Îª¸ßÊ±²»ÄÜ·¢ËÍÊı¾İ¸ø´®¿ÚÆÁ¡£
		
	while(Pulses_check)        //¿ª»úÍ¬²½Ö÷»úÂö³å
	{
		
		if(RX_Data[1] == CMD_UPDATE_MACH3_NUMBER)  //½ÓÊÕµ½×ø±êÊı¾İ
		{	
			Recdata1=RX_Data[18];
			Recdata2=RX_Data[19];			 
      RecPulses= (Recdata1<<8)+Recdata2;  //»ñÈ¡Âö³åÖµ
      if(TIM4->CNT==RecPulses)
				check_time++;
			else 
				TIM4->CNT=RecPulses;
      if(check_time>5)
        Pulses_check=0;
			
//      sprintf(bufrec,"%u",RecPulses);	
//			SetTextValue(0,23,(uchar *)bufrec);     //ÏÔÊ¾Âö³åÖµ			
		}
	}

   

//   while(AT24CXX_Check())   //¼ì²â24C02ÊÇ·ñÕı³£
//	 {
//	   SetTextValue(0,26,(uchar *)"¼ì²âÊ§°Ü");	 
//	 }
//	   SetTextValue(0,26,(uchar *)"¼ì²â³É¹¦");
	//event generator
	// event queue 
	// StateMachine
		
	while(1)                                                                        
	{				
			
		
		Pulses_Count_Process();               //¼ÆËãÊÖÂÖÂö³å	 
		TFT_Page_handle();                    //³ÌĞò½øÈë²»Í¬µÄ¹¤×÷Ò³Ãæ,´¦ÀíÏà¹ØÈÎÎñ	
		Communication_Data_handle();          //´®¿Ú1£¨µñ¿Ì»úÊı¾İ£©Êı¾İ½ÓÊÕ´¦Àí
		
		if(Mark_10ms)
		{	
      			
			Estop_button_process();               //½ô¼±°´Å¥´¥·¢´¦Àíº¯Ê
			TFT_command_analyse();               //·ÖÎöTFTÆÁµÄÃüÁî£¬´¥·¢ÁËÊ²Ã´°´Å¥ ı
		  Mark_10ms=0;
			sprintf(buf4,"master_ask:%u",master_ask);	
			SetTextValue(3,29,(uchar *)buf4);
			sprintf(buf4,"master_send:%u",master_send);	
			SetTextValue(3,49,(uchar *)buf4);
		}
		if(Mark_20ms) 
		{
			Key_scan();                         //ÊÖÂÖÎïÀí°´¼üÉ¨Ãè
			
      light_Control_Process();             //±³¹âÁÁ¶È¿ØÖÆ		
			Mark_20ms=0;
		}
		if(Mark_200ms)                         //¶¨Ê±Âú200ms
		{
			
			TFT_Show_coordanate_value();         //ÏÔÊ¾¹¤¼ş×ø±êºÍ»úĞµ×ø±ê 
			Work_state_control();                //µñ¿Ì»ú¹¤×÷×´Ì¬ÏÔÊ¾£¨Î´Ôö¼ÓÀ¶ÑÀ£¬WiFi×´Ì¬£©
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

