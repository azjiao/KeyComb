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

//GPIO_Init()库函数的二次封装。
//调用前必须使所用的GPIO外设时钟总线使能。
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed);

//对于输入模式的GPIO应用，频率是无用的，但C不支持默认参数，这里给出宏定义来实现默认参数，这样输入模式下可以不用写频率参数。
#define My_GPIO_Init_Macro(GPIOx, GPIO_Pin, GPIO_Mode)    My_GPIO_Init(GPIOx, GPIO_Pin, GPIO_Mode,  GPIO_Speed_10MHz)

#endif
