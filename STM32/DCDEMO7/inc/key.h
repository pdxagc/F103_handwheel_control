#ifndef __KEY_H
#define __KEY_H


#define key_X  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)   //��ȡ����1
#define key_Y  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)   //��ȡ����2
#define key_Z  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)   //��ȡ����3 
#define key_A  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)   //��ȡ����4
#define key_B  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)   //��ȡ����5



//������ʼ������
void Key_Init(void);

//����ɨ�躯��
void Key_scan(void);



#endif
