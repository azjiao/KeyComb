#ifndef __DEV_H
#define __DEV_H

#include "key.h"
#include "led.h"
#include "beep.h"
#include "delay.h"
#include "extix.h"
#include "bitBand.h"
#include "usart1.h"
#include "baseTime.h"
#include "iwdg.h"

//GPIO_Init()�⺯���Ķ��η�װ��
//����ǰ����ʹ���õ�GPIO����ʱ������ʹ�ܡ�
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed);

//��������ģʽ��GPIOӦ�ã�Ƶ�������õģ���C��֧��Ĭ�ϲ�������������궨����ʵ��Ĭ�ϲ�������������ģʽ�¿��Բ���дƵ�ʲ�����
#define My_GPIO_Init_Macro(GPIOx, GPIO_Pin, GPIO_Mode)    My_GPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode,  GPIO_Speed_10MHz)

#endif
