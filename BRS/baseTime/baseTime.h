// 基本定时器只有两个：TIM6和TIM7.

#ifndef __BASETIME_H
#define __BASETIME_H
#include "stm32f10x.h"

// 基本定时器TIM6初始化。
// 初始化TIM6并使能更新中断。
// u16ARR是自动重装载值.
// u16PSC是定时器预分频值。
void Time6_Init(u16 u16ARR, u16 u16PSC);

// 基本定时器TIM6初始化。
// 初始化TIM6并使能更新中断。
// u16ARR是自动重装载值.
// u16PSC是定时器预分频值。
void Time7_Init(u16 u16ARR, u16 u16PSC);

// TIM6初始化为所需us：固定定时计数器时钟CK_CLK为100kHz,即计数器一个计数时间为0.01ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为719即可以使TIM6计数时钟为100kHz.
// u16Ptus为所需定时中断的us值。
// u16Ptus必须为10us的倍数。
void Time6_Init2us(u16 u16Ptms);

// TIM7初始化为所需us：固定定时计数器时钟CK_CLK为100kHz,即计数器一个计数时间为0.01ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为719即可以使TIM7计数时钟为100kHz.
// u16Ptus为所需定时中断的us值。
void Time7_Init2us(u16 u16Ptms);

// TIM7初始化为所需ms：固定定时计数器时钟CK_CLK为10kHz,即计数器一个计数时间为0.1ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为7190即可以使TIM7计数时钟为10kHz.
// u16Ptus为所需定时中断的ms值。
void Time7_Init2ms(u16 u16Ptms);

#endif /* end of include guard: __BASETIME_H */

