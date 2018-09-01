#ifndef __BEEP_H
#define __BEEP_H
#include "bitBand.h"
/*
 * 板载蜂鸣器。
 * beep接到PB8, 推挽输出。
 *
 */

//定义BEEP发声和禁声宏。
//当端口为1时时beep引脚放大管导通接GND，beep发声。
#define __BITBAND_BEEP
// 使用位带来操作蜂鸣器
#ifdef __BITBAND_BEEP
#define BEEP    PBout(8)
#define BEEP_ON  PBout(8) = 1
#define BEEP_OFF  PBout(8) = 0
// 使用普通库函数来操作蜂鸣器
#else
#define BEEP_ON  GPIO_SetBits( GPIOB, GPIO_Pin_8 )
#define BEEP_OFF  GPIO_ResetBits( GPIOB, GPIO_Pin_8 )
#endif

//BEEP初始化
void beep_Init(void);



#endif /* ifndef __BEEP_H */

