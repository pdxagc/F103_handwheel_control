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
//ʹ�õ���24c02�����Զ���EE_TYPEΪAT24C02
#define EE_TYPE AT24C02

uint8 AT24CXX_ReadOneByte(uint16 ReadAddr);							//ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(uint16 WriteAddr,uint8 DataToWrite);		//ָ����ַд��һ���ֽ�

void AT24CXX_WriteLenByte(uint16 WriteAddr,uint32 DataToWrite,uint8 Len);//ָ����ַ��ʼд��ָ�����ȵ�����
uint32 AT24CXX_ReadLenByte(uint16 ReadAddr,uint8 Len);					//ָ����ַ��ʼ��ȡָ����������

uint8 AT24CXX_Check(void);
void AT24CXX_Write(u16 WriteAddr,uint8 *pBuffer,uint16 NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,uint8 *pBuffer,uint16 NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����


#endif

