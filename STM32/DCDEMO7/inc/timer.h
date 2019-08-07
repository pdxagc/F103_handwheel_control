#ifndef _TIMER_H
#define _TIMER_H

#include "hmi_user_uart.h"

void TIME2_Init(void);
void TIME3_Init(void);
void TIME4_Init(void);


void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);


//Âö³å¼ÆÊý¼Ä´æÆ÷ÇåÁã
void Puless_count_clear(void);



#endif

