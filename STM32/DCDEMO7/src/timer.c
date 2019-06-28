#include "timer.h"
#include "stm32f10x_tim.h"
#include "stm32f10x.h"
#include "hmi_user_uart.h"
#include "tft.h"
#include "hmi_driver.h"
#include "stdio.h"

extern int16 Pulses_counter;
extern Control_Panel_Pram control_panel_pram;
extern Return_Workpiece_Zero return_workpiece_zero;
extern uint8 Send_cooddinate_status; 
extern Devide_Set devide_set;  
//uint8 Counter_Dir=0;          //计数方向,0:正转，1：反转
uint8 TIME3_Counter=0;         //定时器3溢出计数
uint16 ABCD=1;


/*******************************************************************************  
* 函 数 名         : TIME2_Init  
* 函数功能         :  TIME2初始化  ,//200ms向主机发送过一次坐标 
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/ 
void TIME2_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);     //使能TIM2时钟 
	
	//配置定时器3
	TIM_TimeBaseStructure.TIM_Period = 1999;                    //设置自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //设置时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM 向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);             //②初始化 TIM2ALIENTEK MiniSTM32 V3.0 开发板教程

	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //③允许更新中断
	
	//中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;              //TIM2 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    //先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           //从优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);                              //初始化 NVIC 寄存器
	
	//TIM_Cmd(TIM2, ENABLE);                                       //使能 TIM2
}

/*******************************************************************************  
* 函 数 名         : TIME3_Init  
* 函数功能         :  TIME3初始化   
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/ 
void TIME3_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     //使能TIM3时钟 
	
	//配置定时器3
	TIM_TimeBaseStructure.TIM_Period = 1999;                     //设置自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =7199;                  //设置时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //设置时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM 向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             //②初始化 TIM3ALIENTEK MiniSTM32 V3.0 开发板教程

	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //③允许更新中断
	
	//中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //TIM3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;    //先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;           //从优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);                              //初始化 NVIC 寄存器
	
	//TIM_Cmd(TIM3, ENABLE);                                       //使能 TIM3
}



/*******************************************************************************  
* 函 数 名         : TIME4_Init  
* 函数功能         :  TIME4初始化   映射到PB6 PB7，计算脉冲数量    
* 输    入         : 无  
* 输    出         : 无  
*******************************************************************************/ 

void TIME4_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;      

	//PB6 ch1  A,PB7 ch2 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   //使能TIM4时钟  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟
	
	//配置IO口
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PB6 PB7浮空输入  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	
	//配置定时器4
	TIM_DeInit(TIM4);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0x0;                  // 预分频器 
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;                  //设定计数器自动重装值  //65535 
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //选择时钟分频：不分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	
	//配置对应寄存器为编码器接口模式,配置相关的输入捕获配置
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising ,TIM_ICPolarity_Rising);//使用编码器模式4，上升下降都计数
	TIM_ICStructInit(&TIM_ICInitStructure);    //将结构体中的内容缺省输入
	TIM_ICInitStructure.TIM_ICFilter = 6;      //选择输入比较滤波器 
	TIM_ICInit(TIM4, &TIM_ICInitStructure);    //将TIM_ICInitStructure中的指定参数初始化TIM4
	
	
	//清除相关中断，以及清除对应的计数器，并启动定时器
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);        //清除TIM4的更新标志位  
  //TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);   // 其实中断可以不用开，因为硬件自行对计数器进行加减。   
  TIM4->CNT = 0;                               //Reset counter
  TIM_Cmd(TIM4, ENABLE);                       //启动TIM4定时器
	//TIM_Cmd(TIM4, DISABLE);                    //关闭TIM4定时器

	
}

//定时器2中断函数
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查 TIM2 更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update ); //清除 TIM2 更新中断标志
	  Send_cooddinate_status=1;	                   //200ms向主机发送过一次坐标
	}
}

//定时器 3 中断服务程序
void TIM3_IRQHandler(void) //TIM3 中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查 TIM3 更新中断发生与否
	{
		TIME3_Counter++;
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update );                  //清除 TIM3 更新中断标志
		if(TIME3_Counter)
		{
		  Usart_SendString(USART2,"{\"pos\":null}\\r\\n");              //向主机询问工件坐标
		}
		if(TIME3_Counter==2)
		{
	    Usart_SendString(USART2,"{\"mpo\":null}\\r\\n");		           //向主机询问机械坐标
			TIME3_Counter=0;
		}
		
	}
}

//定时器 4 中断服务程序
void TIM4_IRQHandler(void)
{   
    if(TIM4->SR&0x0001)         //溢出中断
    {
		    
    }   
    TIM4->SR&=~(1<<0);//清除中断标志位     
}



//计算脉冲数量
int16 Get_Pulses_num(void)
{	
	int16 temp_count;
	temp_count=TIM_GetCounter(TIM4);
	return temp_count;
}

//脉冲计数寄存器清零
void Puless_count_clear(void)
{
	TIM4->CNT = 0;
	Pulses_counter = 0;
}

//所有轴脉冲数量清零
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


