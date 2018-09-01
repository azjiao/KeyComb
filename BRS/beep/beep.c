/*
 * 板载蜂鸣器。
 * beep接到PB8, 推挽输出。
 * 当端口为1时时beep引脚放大管导通接GND，beep发声。
 *
 */

#include "stm32f10x.h"
#include "beep.h"

//BEEP初始化
void beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //声明初始化数据结构。

    //GPIOB 时钟使能。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

    //BEEP 端口设置.
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出。
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; //输出速度10MHz.
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //初始化BEEP 禁声。
    BEEP_OFF;

}
