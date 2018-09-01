/*
 * 开发版板载资源：led.
 * LED0: 接在PB5端口，开漏输出。
 * LED1: 接在PE5端口，开漏输出。
 *
 *
 */
#include "stm32f10x.h"
#include "led.h"


//板载led初始化。
void led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //声明初始化数据结构。

    //GPIOB和GPIOE时钟使能。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    //设置LED0。
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode =GPIO_Mode_Out_OD;   //开漏输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; //输出速度10MHz.
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //设置LED1.除了端口组不一样外，其他一样。
    GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	//初始化使led等熄灭。
	//如果不处理则led相应端口初始化输出为0，灯被点亮。
	LED0_OFF;
	LED1_OFF;
}

