#ifndef _TIMER_H
#define _TIMER_H

#include "hmi_user_uart.h"

void TIME2_Init(void);
void TIME3_Init(void);
void TIME4_Init(void);


void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);

void Get_Pulses_num(void);

//脉冲计数寄存器清零
void Puless_count_clear(void);

//所有轴脉冲数量清零
void Pulses_num_Clear(void);



#endif

