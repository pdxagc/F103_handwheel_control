#ifndef _EXTI_H
#define _EXTI_H


#define key_Estop   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)  //��ȡ����6 


//����ֹͣ������ʼ��
void Estop_Init(void);
void EXTIX_Init(void);
void EXTI15_10_IRQHandler(void);





#endif
