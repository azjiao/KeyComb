/*
���ܣ� �Կ⺯���Ķ��η�װ��
������ ��װGPIO_Init()����ΪMy_GPIO_Init()��������ʼ�����ṹ���ں�������ʵ�֣�ֻ�贫���ʼ�����ݲ���Ҫ�����ʼ���ṹ��ʹ�ⲿ���ó�ʼ����ü򵥡�
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��09��06��
*/
#include <stm32f10x.h>
#include "dev.h"

//GPIO_Init()�⺯���Ķ��η�װ��
//����ǰ����ʹ���õ�GPIO����ʱ������ʹ�ܡ�
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed)
{
    GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed;
    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

