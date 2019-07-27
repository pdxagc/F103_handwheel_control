#ifndef __KEY_H
#define __KEY_H


#define key_X  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)   //读取按键1
#define key_Y  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)   //读取按键2
#define key_Z  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)   //读取按键3 
#define key_A  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)   //读取按键4
#define key_B  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)   //读取按键5
#define key_Estop   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)//读取按键6 


//按键初始化函数
void Key_Init(void);

//按键扫描函数
void Key_scan(void);



#endif
