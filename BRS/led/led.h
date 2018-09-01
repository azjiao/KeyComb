#ifndef __LED_H
#define __LED_H
#include "bitBand.h"

//定义LED0和LED1的点亮和熄灭宏。
//板载LED0和LED1是开漏输出，输出0才能点亮，而输出1熄灭。
#define __BITBANDLED
// 使用位带操作LED。
#ifdef  __BITBANDLED
#define LED0 PBout(5)
#define LED1 PEout(5)

#define LED0_ON  LED0 = 0
#define LED0_OFF LED0 = 1
#define LED1_ON  LED1 = 0
#define LED1_OFF LED1 = 1


//使用普通库函数来操作LED。
#else
#define LED0_ON GPIO_ResetBits( GPIOB, GPIO_Pin_5 )
#define LED0_OFF GPIO_SetBits( GPIOB, GPIO_Pin_5 )
#define LED1_ON GPIO_ResetBits( GPIOE, GPIO_Pin_5 )
#define LED1_OFF GPIO_SetBits( GPIOE, GPIO_Pin_5 )
#endif

//板载led初始化。
void led_Init(void);


#endif /* ifndef __LED_H */
