/*
*功能：基本定时器TIM6/TIM7驱动
*描述：基本定时器只有两个：TIM6和TIM7.计数模式只有向上计数一种模式，不用配置计数模式。
*设计：azjiao
*版本：0.1
*日期：2018-07-18
*
*/

#include "baseTime.h"
#include "led.h"

// 基本定时器TIM6初始化。
// 初始化TIM6并使能更新中断。
// u16ARR是自动重装载值.
// u16PSC是定时器预分频值。
void Time6_Init(u16 u16ARR, u16 u16PSC)
{
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //定时器初始化参数结构。
    NVIC_InitTypeDef         NVIC_InitStruct;  // 中断向量初始化参数结构。

    //TIM6时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    //TIM6初始化
    TIM_InitStrut.TIM_Period = u16ARR; // 自动装载寄存器值.
    TIM_InitStrut.TIM_Prescaler = u16PSC; //定时器时钟预分频值，作为计数器时钟用。
    // TIM6是基本定时器，只能使用向上计数模式，不用设置。
    TIM_TimeBaseInit(TIM6, &TIM_InitStrut);

    //使能TIM6中断，允许中断更新。
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update); //清除中断更新标识.

    //设置TIM6中断的优先级。
    NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //使能TIM6。
    //TIM_Cmd(TIM6, ENABLE);
}

// 基本定时器TIM7初始化。
// 初始化TIM7并使能更新中断。
// u16ARR是自动重装载值.
// u16PSC是定时器预分频值。
void Time7_Init(u16 u16ARR, u16 u16PSC)
{
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //定时器初始化参数结构。
    NVIC_InitTypeDef         NVIC_InitStruct;  // 中断向量初始化参数结构。

    //TIM7时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    //TIM7初始化
    TIM_InitStrut.TIM_Period = u16ARR; // 自动装载寄存器值.
    TIM_InitStrut.TIM_Prescaler = u16PSC; //定时器时钟预分频值，作为计数器时钟用。
    // TIM6是基本定时器，只能使用向上计数模式，不用设置。
    TIM_TimeBaseInit(TIM7, &TIM_InitStrut);

    //使能TIM7中断，允许中断更新。
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //清除中断更新标识.

    //设置TIM7中断的优先级。配置为TIM7的响应优先级比TIM6低。
    NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //使能TIM7。在外部使能启动定时。
    //TIM_Cmd(TIM7, ENABLE);
}

// TIM6初始化为所需us：固定定时计数器时钟CK_CLK为100kHz,即计数器一个计数时间为0.01ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为719即可以使TIM6计数时钟为100kHz.
// u16Ptus为所需定时中断的us值。
// u16Ptus必须为10us的倍数。
void Time6_Init2us(u16 u16Ptus)
{
    u16 u16ARR;
    // 把u16Ptms转换为自动重装载寄存器所需的值。
    u16ARR = u16Ptus / 10 - 1;
    Time6_Init(u16ARR, (u16)719);
}

// TIM7初始化为所需us：固定定时计数器时钟CK_CLK为100kHz,即计数器一个计数时间为0.01ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为719即可以使TIM7计数时钟为100kHz.
// u16Ptus为所需定时中断的us值。
void Time7_Init2us(u16 u16Ptus)
{
    u16 u16ARR;
    // 把u16Ptms转换为自动重装载寄存器所需的值。
    u16ARR = u16Ptus / 10 - 1;
    Time7_Init(u16ARR, (u16)719);
}

// TIM7初始化为所需ms：固定定时计数器时钟CK_CLK为10kHz,即计数器一个计数时间为0.1ms.
// 系统初始化时APB1分频值为2，为36MHz,则TIM6Clk为72MHz.
// 设置PSC为7199即可以使TIM7计数时钟为10kHz.
// u16Ptus为所需定时中断的ms值。
void Time7_Init2ms(u16 u16Ptms)
{
    u16 u16ARR;
    // 把u16Ptms转换为自动重装载寄存器所需的值。
    u16ARR = u16Ptms * 10 - 1;
    Time7_Init(u16ARR, (u16)7199);
}

/*
//定时器TIM6中断服务函数
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)  //是否发生中断更新。
    {
        LED0 = !LED0; //LED0取反
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update); //清除中断更新标识.
    }
}

//定时器TIM7中断服务函数
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)  //是否发生中断更新。
    {
        LED1 = !LED1; //LED1取反
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //清除中断更新标识.
    }
}
*/
