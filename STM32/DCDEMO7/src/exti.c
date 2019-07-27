#include "exti.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"
#include "ulitity.h"


extern uint8 Press_button;

//����ֹͣ������ʼ��
void Estop_Init(void)
{	
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;    //PB12  ��ͣ��ť
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOB, &GPIO_InitStructure);        //��ʼ��GPIOB12

}


//�жϳ�ʼ��
void EXTIX_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);      //�ⲿ�жϣ���Ҫʹ�� AFIO ʱ��
	
	Estop_Init();                                          //����ֹͣ������ʼ��
	
	//GPIOB12 �ж����Լ��жϳ�ʼ������
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);  //ѡ���ź�Դ,�� GPIO �˿����ж���ӳ������
	EXTI_InitStructure.EXTI_Line=EXTI_Line12;                    //ѡ���ж���
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTIΪ�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //�����ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //ʹ���ж�
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB12�ж����ȼ�����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;             //ʹ�ܽ���������ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //��ռ���ȼ� 0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        //�����ȼ� 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);                              //���� NVIC_InitStruct ��ָ���Ĳ�����ʼ������ NVIC �Ĵ���
}



void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line12)!=RESET)              //�ж�ĳ�����ϵ��ж��Ƿ���
	{
		delay_ms(10);                                       //����
	  if(key_Estop == 0)                                   //��ͣ�ᰴť
		{
			Press_button = CMD_EStop;
		}
		EXTI_ClearITPendingBit(EXTI_Line12);                 //��� LINE12 �ϵ��жϱ�־λ
	}

}
