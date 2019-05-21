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
void Pulses_num_Clear(void);



#endif

