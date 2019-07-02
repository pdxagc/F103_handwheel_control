#include "exti.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"



void Counter_Init(void)
{	
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //PB0 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //设置成输入模式
 GPIO_Init(GPIOB, &GPIO_InitStructure);					   //根据设定参数初始化GPIOB0
	
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				   //PB1 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //设置成输入模式
 GPIO_Init(GPIOB, &GPIO_InitStructure);


}


//中断初始化
void EXTIX_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);      //外部中断，需要使能 AFIO 时钟
	
	Counter_Init();                                     //计数IO口初始化
	
	//GPIOB.0 中断线以及中断初始化配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);  //选择信号源,将 GPIO 端口与中断线映射起来
	EXTI_InitStructure.EXTI_Line=EXTI_Line0;                    //选择中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTI为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;     //上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //使能中断
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB.1 中断线以及中断初始化配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);  //选择信号源,将 GPIO 端口与中断线映射起来
	EXTI_InitStructure.EXTI_Line=EXTI_Line1;                    //选择中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTI为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;     //上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //使能中断
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB.0中断优先级设置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //使能接收脉冲的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2，
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //子优先级 1	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);                              //根据 NVIC_InitStruct 中指定的参数初始化外设 NVIC 寄存器
	
	//GPIOB.1中断优先级设置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;             //使能接收脉冲的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2，
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //子优先级 1	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);                              //根据 NVIC_InitStruct 中指定的参数初始化外设 NVIC 寄存器



}

//中断使能
//void EXTI0_enable(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
////	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //使能按键所在的外部中断通道
////	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2，
////	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //子优先级 1
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //禁止外部中断通道
//	
//	NVIC_Init(&NVIC_InitStructure);

//}

//中断禁止
void EXTI0_disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;             //使能按键所在的外部中断通道
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2，
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;        //子优先级 1
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;             //禁止外部中断通道
	
	NVIC_Init(&NVIC_InitStructure);

}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0)!=RESET)              //判断某个线上的中断是否发生
	{
		EXTI_ClearITPendingBit(EXTI_Line0);                 //清除 LINE0 上的中断标志位
	}

}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1)!=RESET)              //判断某个线上的中断是否发生
	{
		EXTI_ClearITPendingBit(EXTI_Line1);                 //清除 LINE0 上的中断标志位
	}

}
