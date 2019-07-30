/*! 
*  \file hmi_driver.h
*  \brief 定时器中断
*  \version 1.0
*  \date 2012-2018
*/
#ifndef _ULITITY_H
#define _ULITITY_H

#include "hmi_user_uart.h"

void systicket_init(void);

void delay_ms(uint32 delay);


//time为要延时的us数.		    								   
void delay_us(uint16 time);


#endif
