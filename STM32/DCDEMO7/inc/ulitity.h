/*! 
*  \file hmi_driver.h
*  \brief ��ʱ���ж�
*  \version 1.0
*  \date 2012-2018
*/
#ifndef _ULITITY_H
#define _ULITITY_H

#include "hmi_user_uart.h"

void systicket_init(void);

void delay_ms(uint32 delay);


//timeΪҪ��ʱ��us��.		    								   
void delay_us(uint16 time);


#endif
