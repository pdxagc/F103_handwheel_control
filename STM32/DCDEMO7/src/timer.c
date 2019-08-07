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
extern uint8 Mark_10ms;                 //20ms��ʱ���λ
extern uint8 Mark_20ms;                 //20ms��ʱ���λ
extern uint8 Mark_30ms;                 //30ms��ʱ���λ
extern uint8 Mark_200ms;                 //60ms��ʱ���λ
extern uint8 Mark_500ms;                //500ms��ʱ���λ

uint8 Mark_10ms_Count;
uint8 Mark_20ms_Count;
uint8 Mark_30ms_Count;
uint8 Mark_200ms_Count;
uint8 Mark_500ms_Count;
uint8 TIME3_Counter=0;         //��ʱ��3�������



/*******************************************************************************  
* �� �� ��         : TIME2_Init  
* ��������         :  TIME2��ʼ��  ,//10ms��ʱ�ж�
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 
void TIME2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);     //ʹ��TIM2ʱ�� 
	
	//���ö�ʱ��3
	TIM_TimeBaseStructure.TIM_Period = 99;                    //�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //����ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //����ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM ���ϼ���
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);             //�ڳ�ʼ�� 

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //����������ж�
	
	//�ж����ȼ� NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2 �ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    //��ռ���ȼ� 1 ��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;           //�����ȼ� 3 ��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ ͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);                              //��ʼ�� NVIC �Ĵ���
	
	TIM_Cmd(TIM2, ENABLE);                                       //ʹ�� TIM2
}

/*******************************************************************************  
* �� �� ��         : TIME3_Init  
* ��������         :  TIME3��ʼ��() ,��ʱ3S
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 
void TIME3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     //ʹ��TIM3ʱ�� 
	
	//���ö�ʱ��3
	TIM_TimeBaseStructure.TIM_Period = 29999;                     //�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //����ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //����ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM ���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //�ڳ�ʼ��

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //����������ж�
	
	//�ж����ȼ� NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3 �ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;    //��ռ���ȼ� 0 ��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           //�����ȼ� 3 ��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ ͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);                              //��ʼ�� NVIC �Ĵ���
	
	//TIM_Cmd(TIM3, ENABLE);                                       //ʹ�� TIM3
}



/*******************************************************************************  
* �� �� ��         : TIME4_Init  
* ��������         :  TIME4��ʼ��   ӳ�䵽PB6 PB7��������������    
* ��    ��         : ��  
* ��    ��         : ��  
*******************************************************************************/ 

void TIME4_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;      

	//PB6 ch1  A,PB7 ch2 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   //ʹ��TIM4ʱ��  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ��
	
	//����IO��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PB6 PB7��������  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//���ö�ʱ��4
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;                  // Ԥ��Ƶ�� 
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;                  //�趨�������Զ���װֵ  //65535 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //ѡ��ʱ�ӷ�Ƶ������Ƶ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM���ϼ���  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	
	//���ö�Ӧ�Ĵ���Ϊ�������ӿ�ģʽ,������ص����벶������
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising ,TIM_ICPolarity_Rising);//ʹ�ñ�����ģʽ4�������½�������
	TIM_ICStructInit(&TIM_ICInitStructure);    //���ṹ���е�����ȱʡ����
	TIM_ICInitStructure.TIM_ICFilter = 4;      //ѡ������Ƚ��˲��� 
	TIM_ICInit(TIM4, &TIM_ICInitStructure);    //��TIM_ICInitStructure�е�ָ��������ʼ��TIM4
	
	
	//�������жϣ��Լ������Ӧ�ļ���������������ʱ��
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);        //���TIM4�ĸ��±�־λ  
  //TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);   // ��ʵ�жϿ��Բ��ÿ�����ΪӲ�����жԼ��������мӼ���   
  TIM4->CNT = 0;                               //Reset counter
  TIM_Cmd(TIM4, ENABLE);                       //����TIM4��ʱ��
	//TIM_Cmd(TIM4, DISABLE);                    //�ر�TIM4��ʱ��

	
}

//��ʱ��2�жϺ���
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //��� TIM2 �����жϷ������
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update ); //��� TIM2 �����жϱ�־
		Mark_20ms_Count++;
		Mark_30ms_Count++;
	  Mark_10ms=1;                  //Mark_10ms��1    
		if(Mark_20ms_Count==2)        //Mark_20ms��1
		{
			Mark_20ms_Count=0;
		  Mark_20ms = 1;
			Mark_200ms_Count++;
			if(Mark_200ms_Count==10)    //Mark_200ms��1
			{
				Mark_200ms_Count=0;
				Mark_200ms = 1;					
        Mark_500ms_Count++;		
        if(Mark_500ms_Count==25)  //Mark_500ms��1
				{
					Mark_500ms_Count=0;
					Mark_500ms=1;				
				}				
			}		
		}
		if(Mark_30ms_Count == 3)     //Mark_30ms��1
		{
			Mark_30ms=1;
			Mark_30ms_Count=0;
		}
	
	}
}

//��ʱ�� 3 �жϷ������
void TIM3_IRQHandler(void) //TIM3 �ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //��� TIM3 �����жϷ������
	{
		TIME3_Counter++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update );                  //��� TIM3 �����жϱ�־
//		if(TIME3_Counter==2)
//		{
//		  //SetLightLevel(200);
//			TIME3_Counter=0;
//		}		
	}
}

////��ʱ�� 4 �жϷ������
//void TIM4_IRQHandler(void)
//{   
//    if(TIM4->SR&0x0001)         //����ж�
//    {
//		    
//    }   
//    TIM4->SR&=~(1<<0);//����жϱ�־λ     
//}





//��������Ĵ�������
void Puless_count_clear(void)
{
	TIM4->CNT = 0;
	Pulses_counter = 0;
	
}




