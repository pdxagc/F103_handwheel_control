#include "exti.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"



void Counter_Init(void)
{	
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //PB0 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //���ó�����ģʽ
 GPIO_Init(GPIOB, &GPIO_InitStructure);					   //�����趨������ʼ��GPIOB0
	
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				   //PB1 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //���ó�����ģʽ
 GPIO_Init(GPIOB, &GPIO_InitStructure);


}


//�жϳ�ʼ��
void EXTIX_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);      //�ⲿ�жϣ���Ҫʹ�� AFIO ʱ��
	
	Counter_Init();                                     //����IO�ڳ�ʼ��
	
	//GPIOB.0 �ж����Լ��жϳ�ʼ������
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);  //ѡ���ź�Դ,�� GPIO �˿����ж���ӳ������
	EXTI_InitStructure.EXTI_Line=EXTI_Line0;                    //ѡ���ж���
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTIΪ�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;     //�����ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //ʹ���ж�
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB.1 �ж����Լ��жϳ�ʼ������
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);  //ѡ���ź�Դ,�� GPIO �˿����ж���ӳ������
	EXTI_InitStructure.EXTI_Line=EXTI_Line1;                    //ѡ���ж���
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTIΪ�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;     //�����ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //ʹ���ж�
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB.0�ж����ȼ�����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //ʹ�ܽ���������ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ� 2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //�����ȼ� 1	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);                              //���� NVIC_InitStruct ��ָ���Ĳ�����ʼ������ NVIC �Ĵ���
	
	//GPIOB.1�ж����ȼ�����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;             //ʹ�ܽ���������ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ� 2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //�����ȼ� 1	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);                              //���� NVIC_InitStruct ��ָ���Ĳ�����ʼ������ NVIC �Ĵ���



}

//�ж�ʹ��
//void EXTI0_enable(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
////	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
////	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ� 2��
////	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //�����ȼ� 1
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //��ֹ�ⲿ�ж�ͨ��
//	
//	NVIC_Init(&NVIC_InitStructure);

//}

//�жϽ�ֹ
void EXTI0_disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ� 2��
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //�����ȼ� 1
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;             //��ֹ�ⲿ�ж�ͨ��
	
	NVIC_Init(&NVIC_InitStructure);

}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET)              //�ж�ĳ�����ϵ��ж��Ƿ���
	{
		EXTI_ClearITPendingBit(EXTI_Line0);                 //��� LINE0 �ϵ��жϱ�־λ
	}

}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1)!=RESET)              //�ж�ĳ�����ϵ��ж��Ƿ���
	{
		EXTI_ClearITPendingBit(EXTI_Line1);                 //��� LINE0 �ϵ��жϱ�־λ
	}

}
