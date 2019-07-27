#ifndef _EXTI_H
#define _EXTI_H


#define key_Estop   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)  //读取按键6 


//紧急停止按键初始化
void Estop_Init(void);
void EXTIX_Init(void);
void EXTI15_10_IRQHandler(void);





#endif
