#ifndef _FLASH_H
#define _FLASH_H

#include "stm32f10x_it.h"     //根据用户MCU进行修改
#include "hmi_user_uart.h"

//#define uchar    unsigned char
//#define uint8    unsigned char
//#define uint16   unsigned short int
//#define uint32   unsigned long
//#define int16    short int
//#define int32    long


void FLASH_WriteByte(uint32 addr,uint16 Data);

uint8 FlashRead(uint32 addr);







#endif

