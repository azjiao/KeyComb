/*
功能： 对库函数的二次封装。
描述： 封装GPIO_Init()函数为My_GPIO_Init()函数，初始化化结构放在函数内来实现，只需传入初始化数据不需要传入初始化结构。使外部调用初始化变得简单。
设计： azjiao
版本： 0.1
日期： 2018年09月06日
*/
#include <stm32f10x.h>
#include "dev.h"

//GPIO_Init()库函数的二次封装。
//调用前必须使所用的GPIO外设时钟总线使能。
void My_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIOMode_TypeDef GPIO_Mode, GPIOSpeed_TypeDef GPIO_Speed)
{
    GPIO_InitTypeDef GPIO_InitStruct; //声明初始化数据结构。
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed;
    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

