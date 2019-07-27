#include "exti.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"
#include "ulitity.h"


extern uint8 Press_button;

//紧急停止按键初始化
void Estop_Init(void)
{	
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;    //PB12  急停按钮
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOB, &GPIO_InitStructure);        //初始化GPIOB12

}


//中断初始化
void EXTIX_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);      //外部中断，需要使能 AFIO 时钟
	
	Estop_Init();                                          //紧急停止按键初始化
	
	//GPIOB12 中断线以及中断初始化配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource12);  //选择信号源,将 GPIO 端口与中断线映射起来
	EXTI_InitStructure.EXTI_Line=EXTI_Line12;                    //选择中断线
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;         //EXTI为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;     //上升沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                   //使能中断
	EXTI_Init(&EXTI_InitStructure);
	
	//GPIOB12中断优先级设置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;             //使能接收脉冲的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; //抢占优先级 0，
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        //子优先级 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);                              //根据 NVIC_InitStruct 中指定的参数初始化外设 NVIC 寄存器
}



void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line12)!=RESET)              //判断某个线上的中断是否发生
	{
		delay_ms(10);                                       //消抖
	  if(key_Estop == 0)                                   //急停轴按钮
		{
			Press_button = CMD_EStop;
		}
		EXTI_ClearITPendingBit(EXTI_Line12);                 //清除 LINE12 上的中断标志位
	}

}
