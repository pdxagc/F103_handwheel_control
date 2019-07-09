#include "timer.h"
#include "stm32f10x_tim.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"
#include "tft.h"
#include "hmi_driver.h"
#include "stdio.h"

extern uint16 Pulses_counter;
extern Control_Panel_Pram control_panel_pram;
extern Return_Workpiece_Zero return_workpiece_zero;
extern Devide_Set devide_set; 
extern uint8 Mark_10ms;                 //20msº∆ ±±Íº«Œª
extern uint8 Mark_10ms_Count;
extern uint8 Mark_20ms;                 //20msº∆ ±±Íº«Œª
extern uint8 Mark_20ms_Count;           //20msº∆ ±“Á≥ˆÕ≥º∆Œª
extern uint8 Mark_60ms;                //100msº∆ ±±Íº«Œ
//uint8 Counter_Dir=0;          //º∆ ˝∑ΩœÚ,0:’˝◊™£¨1£∫∑¥◊™
uint8 TIME3_Counter=0;         //∂® ±∆˜3“Á≥ˆº∆ ˝
uint16 ABCD=1;


/*******************************************************************************  
* ∫Ø  ˝ √˚         : TIME2_Init  
* ∫Ø ˝π¶ƒ‹         :  TIME2≥ı ºªØ  ,//20ms  À¢–¬LCD∆¡ ˝æ›
*  ‰    »Î         : Œﬁ  
*  ‰    ≥ˆ         : Œﬁ  
*******************************************************************************/ 
void TIME2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);     // πƒ‹TIM2 ±÷” 
	
	//≈‰÷√∂® ±∆˜3
	TIM_TimeBaseStructure.TIM_Period = 199;                    //…Ë÷√◊‘∂Ø÷ÿ◊∞‘ÿºƒ¥Ê∆˜÷‹∆⁄µƒ÷µ
	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //…Ë÷√ ±÷”∆µ¬ ≥˝ ˝µƒ‘§∑÷∆µ÷µ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //…Ë÷√ ±÷”∑÷∏Ó
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM œÚ…œº∆ ˝
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);             //¢⁄≥ı ºªØ TIM2ALIENTEK MiniSTM32 V3.0 ø™∑¢∞ÂΩÃ≥Ã

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //¢€‘ –Ì∏¸–¬÷–∂œ
	
	//÷–∂œ”≈œ»º∂ NVIC …Ë÷√
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2 ÷–∂œ
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    //œ»’º”≈œ»º∂  º∂
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;           //¥””≈œ»º∂ 3 º∂
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ Õ®µ¿±ª πƒ‹
	NVIC_Init(&NVIC_InitStructure);                              //≥ı ºªØ NVIC ºƒ¥Ê∆˜
	
	TIM_Cmd(TIM2, ENABLE);                                       // πƒ‹ TIM2
}

/*******************************************************************************  
* ∫Ø  ˝ √˚         : TIME3_Init  
* ∫Ø ˝π¶ƒ‹         :  TIME3≥ı ºªØ  £¨(œÚ÷˜ª˙—ØŒ ◊¯±Í) 
*  ‰    »Î         : Œﬁ  
*  ‰    ≥ˆ         : Œﬁ  
*******************************************************************************/ 
//void TIME3_Init(void)
//{
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     // πƒ‹TIM3 ±÷” 
//	
//	//≈‰÷√∂® ±∆˜3
//	TIM_TimeBaseStructure.TIM_Period = 1999;                     //…Ë÷√◊‘∂Ø÷ÿ◊∞‘ÿºƒ¥Ê∆˜÷‹∆⁄µƒ÷µ
//	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //…Ë÷√ ±÷”∆µ¬ ≥˝ ˝µƒ‘§∑÷∆µ÷µ
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //…Ë÷√ ±÷”∑÷∏Ó
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM œÚ…œº∆ ˝
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //¢⁄≥ı ºªØ TIM3ALIENTEK MiniSTM32 V3.0 ø™∑¢∞ÂΩÃ≥Ã

//	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //¢€‘ –Ì∏¸–¬÷–∂œ
//	
//	//÷–∂œ”≈œ»º∂ NVIC …Ë÷√
//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3 ÷–∂œ
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    //œ»’º”≈œ»º∂ 0 º∂
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           //¥””≈œ»º∂ 3 º∂
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ Õ®µ¿±ª πƒ‹
//	NVIC_Init(&NVIC_InitStructure);                              //≥ı ºªØ NVIC ºƒ¥Ê∆˜
//	
//	//TIM_Cmd(TIM3, ENABLE);                                       // πƒ‹ TIM3
//}



/*******************************************************************************  
* ∫Ø  ˝ √˚         : TIME4_Init  
* ∫Ø ˝π¶ƒ‹         :  TIME4≥ı ºªØ   ”≥…‰µΩPB6 PB7£¨º∆À„¬ˆ≥Â ˝¡ø    
*  ‰    »Î         : Œﬁ  
*  ‰    ≥ˆ         : Œﬁ  
*******************************************************************************/ 

void TIME4_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;      

	//PB6 ch1  A,PB7 ch2 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   // πƒ‹TIM4 ±÷”  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // πƒ‹GPIOB ±÷”
	
	//≈‰÷√IOø⁄
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PB6 PB7∏°ø’ ‰»Î  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//≈‰÷√∂® ±∆˜4
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;                  // ‘§∑÷∆µ∆˜ 
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;                  //…Ë∂®º∆ ˝∆˜◊‘∂Ø÷ÿ◊∞÷µ  //65535 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //—°‘Ò ±÷”∑÷∆µ£∫≤ª∑÷∆µ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIMœÚ…œº∆ ˝  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	
	//≈‰÷√∂‘”¶ºƒ¥Ê∆˜Œ™±‡¬Î∆˜Ω”ø⁄ƒ£ Ω,≈‰÷√œ‡πÿµƒ ‰»Î≤∂ªÒ≈‰÷√
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising ,TIM_ICPolarity_Rising);// π”√±‡¬Î∆˜ƒ£ Ω4£¨…œ…˝œ¬Ωµ∂ºº∆ ˝
	TIM_ICStructInit(&TIM_ICInitStructure);    //Ω´Ω·ππÃÂ÷–µƒƒ⁄»›»± ° ‰»Î
	TIM_ICInitStructure.TIM_ICFilter = 4;      //—°‘Ò ‰»Î±»Ωœ¬À≤®∆˜ 
	TIM_ICInit(TIM4, &TIM_ICInitStructure);    //Ω´TIM_ICInitStructure÷–µƒ÷∏∂®≤Œ ˝≥ı ºªØTIM4
	
	
	//«Â≥˝œ‡πÿ÷–∂œ£¨“‘º∞«Â≥˝∂‘”¶µƒº∆ ˝∆˜£¨≤¢∆Ù∂Ø∂® ±∆˜
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);        //«Â≥˝TIM4µƒ∏¸–¬±Í÷æŒª  
  //TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);   // ∆‰ µ÷–∂œø…“‘≤ª”√ø™£¨“ÚŒ™”≤º˛◊‘––∂‘º∆ ˝∆˜Ω¯––º”ºı°£   
  TIM4->CNT = 0;                               //Reset counter
  TIM_Cmd(TIM4, ENABLE);                       //∆Ù∂ØTIM4∂® ±∆˜
	//TIM_Cmd(TIM4, DISABLE);                    //πÿ±’TIM4∂® ±∆˜

	
}

//∂® ±∆˜2÷–∂œ∫Ø ˝
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //ºÏ≤È TIM2 ∏¸–¬÷–∂œ∑¢…˙”Î∑Ò
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update ); //«Â≥˝ TIM2 ∏¸–¬÷–∂œ±Í÷æ
//	  Mark_10ms=1;
//    Mark_10ms_Count++;		//10ms£¨≤È—ØLCD√¸¡Ó
//		if(Mark_10ms_Count == 2)
//		{
		  Mark_20ms = 1;
			Mark_10ms_Count = 0;
			Mark_20ms_Count++;
			if(Mark_20ms_Count == 3)
			{
				Mark_60ms = 1;
				Mark_20ms_Count=0;			
			}
		
	}
}

//∂® ±∆˜ 3 ÷–∂œ∑˛ŒÒ≥Ã–Ú
//void TIM3_IRQHandler(void) //TIM3 ÷–∂œ
//{
//	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //ºÏ≤È TIM3 ∏¸–¬÷–∂œ∑¢…˙”Î∑Ò
//	{
//		TIME3_Counter++;
//		TIM_ClearITPendingBit(TIM3, TIM_IT_Update );                  //«Â≥˝ TIM3 ∏¸–¬÷–∂œ±Í÷æ
//		if(TIME3_Counter)
//		{
//		  Usart_SendString(USART2,"{\"pos\":null}\\r\\n");              //œÚ÷˜ª˙—ØŒ π§º˛◊¯±Í
//		}
//		if(TIME3_Counter==2)
//		{
//	    Usart_SendString(USART2,"{\"mpo\":null}\\r\\n");		           //œÚ÷˜ª˙—ØŒ ª˙–µ◊¯±Í
//			TIME3_Counter=0;
//		}
//		
//	}
//}

////∂® ±∆˜ 4 ÷–∂œ∑˛ŒÒ≥Ã–Ú
//void TIM4_IRQHandler(void)
//{   
//    if(TIM4->SR&0x0001)         //“Á≥ˆ÷–∂œ
//    {
//		    
//    }   
//    TIM4->SR&=~(1<<0);//«Â≥˝÷–∂œ±Í÷æŒª     
//}



//º∆À„¬ˆ≥Â ˝¡ø
void Get_Pulses_num(void)
{	
	uint16 temp_count;
	temp_count=TIM_GetCounter(TIM4);
	if(temp_count%4==0)
	{
  	Pulses_counter= temp_count;
	}
	else
	{
	  Pulses_counter=temp_count+(4-temp_count%4);
	}
}

//¬ˆ≥Âº∆ ˝ºƒ¥Ê∆˜«Â¡„
void Puless_count_clear(void)
{
	TIM4->CNT = 0;
	Pulses_counter = 0;
	
}

//À˘”–÷·¬ˆ≥Â ˝¡ø«Â¡„
void Pulses_num_Clear(void)
{
  Puless_count_clear();
	
	control_panel_pram.X_value=0;
	control_panel_pram.Y_value=0;
	control_panel_pram.Z_value=0;
	control_panel_pram.A_value=0;
	control_panel_pram.B_value=0;
	
	control_panel_pram.Temp_save_Xvalue=0;
  control_panel_pram.Temp_save_Yvalue=0;
	control_panel_pram.Temp_save_Zvalue=0;
	control_panel_pram.Temp_save_Avalue=0;
	control_panel_pram.Temp_save_Bvalue=0;
	
	return_workpiece_zero.Re_X_Value=0;
	return_workpiece_zero.Re_Y_Value=0;
	return_workpiece_zero.Re_Z_Value=0;
	return_workpiece_zero.Re_A_Value=0;
	return_workpiece_zero.Re_B_Value=0;
}


