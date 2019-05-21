#include "flash.h"
#include "stm32f10x_flash.h"
#include "hmi_user_uart.h"
#include "string.h"
#include "stm32f10x_it.h" 


//写入一个字节单flash
void FLASH_WriteByte(uint32 addr,uint16 Data)
{
	
	FLASH_Status FLASHstatus = FLASH_COMPLETE;        //定义一个Flash状态指示标志位
	FLASH_Unlock();                                   //解锁函数(在对flash进行写操作前必须解锁)
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); //清除所有已有标志
  FLASHstatus=FLASH_ErasePage(addr);          //擦除一个flash页面
	if(FLASHstatus == FLASH_COMPLETE)                 ///flash操作完成
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

//从flash中读取一个字节
uint8 FlashRead(uint32 addr)
{
  uint8 data;
	data = *(uint8_t *)addr;
	return data;

}
