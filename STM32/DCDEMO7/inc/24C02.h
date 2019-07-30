#ifndef __24C02_H
#define __24C02_H
#include "hmi_user_uart.h"

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
//使用的是24c02，所以定义EE_TYPE为AT24C02
#define EE_TYPE AT24C02

uint8 AT24CXX_ReadOneByte(uint16 ReadAddr);							//指定地址读取一个字节
void AT24CXX_WriteOneByte(uint16 WriteAddr,uint8 DataToWrite);		//指定地址写入一个字节

void AT24CXX_WriteLenByte(uint16 WriteAddr,uint32 DataToWrite,uint8 Len);//指定地址开始写入指定长度的数据
uint32 AT24CXX_ReadLenByte(uint16 ReadAddr,uint8 Len);					//指定地址开始读取指定长度数据

uint8 AT24CXX_Check(void);
void AT24CXX_Write(u16 WriteAddr,uint8 *pBuffer,uint16 NumToWrite);	//从指定地址开始写入指定长度的数据
void AT24CXX_Read(u16 ReadAddr,uint8 *pBuffer,uint16 NumToRead);   	//从指定地址开始读出指定长度的数据


#endif

