//外部中断资源定义

#include "extix.h"
#include "key.h"
#include "led.h"

// 板载按键外部中断初始化。
// KEY0/KEY1/WKUP三个按键的中断初始化。
// 分两步：1.设置外部中断线。2.设置中断线的NVIC优先级并使能中断。
void keyExti_Init(void)
{

    EXTI_InitTypeDef EXTI_InitStruct;  // 外部中断初始化参数结构。
    NVIC_InitTypeDef NVIC_InitStruct;  // 中断向量控制器初始化参数结构。

    // 复用功能时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);


    // KEY0 中断设置：
    // KEY0:接到PE4端口，上拉输入；
    // 设置PE4为线4的中断源。
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
    //中断线4、下降沿触发。
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // KEY1 中断设置：
    // KEY1: 接到PE3端口，上拉输入；
    // 设置PE3为线3的中断源。
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    // 中断线3、下降沿触发。
    EXTI_InitStruct.EXTI_Line = EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // WKUP 中断设置：
    // WKUP: 接到PA0端口，下拉输入；
    // 设置PA0为线0的中断源。
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    // 中断线0、上升沿触发。
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;    
    EXTI_Init(&EXTI_InitStruct);

    // 设置NVIC中断优先级及使能中断。
    // KEY0所在的外部中断通道为EXTI4_IRQn。
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    // 抢占优先级为2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // 响应优先级为0.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    // 使能外部中断通道。
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // 根据初始化参数结构信息初始化NVIC.
    NVIC_Init(&NVIC_InitStruct);


    // KEY1所在的外部中断通道为EXTI3_IRQn。
    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    // 抢占优先级为2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // 响应优先级为1.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    // 使能外部中断通道。
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // 根据初始化参数结构信息初始化NVIC.
    NVIC_Init(&NVIC_InitStruct);

    // WKUP所在的外部中断通道为EXTI0_IRQn。
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    // 抢占优先级为2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // 响应优先级为3.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x03;
    // 使能外部中断通道。
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // 根据初始化参数结构信息初始化NVIC.
    NVIC_Init(&NVIC_InitStruct);
    
}


// 外部中断服务函数。
// KEY0对应的EXTI4服务函数。
void EXTI4_IRQHandler(void)    
{
    if(KEY0_CODE)
    {
        LED0_ON;
    }
    EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
}

// KEY1对应的EXTI3服务函数。
void EXTI3_IRQHandler(void)
{
    if(KEY1_CODE)
    {
        LED1_ON;
    }
    EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
}

// WKUP对应的EXTI0服务函数。
void EXTI0_IRQHandler(void)
{
    if(WKUP_CODE)
    {
        LED0_OFF;
        LED1_OFF;
    }
    EXTI_ClearITPendingBit(EXTI_Line0);  //清除LINE0上的中断标志位  
}

//
