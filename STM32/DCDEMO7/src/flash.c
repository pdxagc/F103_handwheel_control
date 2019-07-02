#include "flash.h"
#include "stm32f10x_flash.h"
#include "hmi_user_uart.h"
#include "string.h"
#include "stm32f10x_it.h" 


//д��һ���ֽڵ�flash
void FLASH_WriteByte(uint32 addr,uint16 Data)
{
	
	FLASH_Status FLASHstatus = FLASH_COMPLETE;        //����һ��Flash״ָ̬ʾ��־λ
	FLASH_Unlock();                                   //��������(�ڶ�flash����д����ǰ�������)
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); //����������б�־
  FLASHstatus=FLASH_ErasePage(addr);          //����һ��flashҳ��
	if(FLASHstatus == FLASH_COMPLETE)                 ///flash�������
	{
		FLASHstatus = FLASH_ProgramHalfWord(addr, Data);
	}
	FLASH_Lock();
}



//uint32_t FlashRead(uint32_t addr, uint8_t *buffer, uint32_t length)  
//{  
//  memcpy(buffer, (void *)START_ADDR , length);  
//  
//  return length;  
//} 

//��flash�ж�ȡһ���ֽ�
uint8 FlashRead(uint32 addr)
{
  uint8 data;
	data = *(uint8_t *)addr;
	return data;

}
