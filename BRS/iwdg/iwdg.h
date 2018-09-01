/*********************************************************
功能： 独立看门狗
描述： 板载资源独立看门狗
设计： azjiao
版本： 0.1
日期： 2018年08月04日
*********************************************************/
#ifndef __IWDG_H
#define __IWDG_H
#include "dev.h"

//独立看门狗初始化。
void Iwdg_Init(uint8_t u8Prer, uint16_t u16Rlr);
//喂狗:复位看门狗计数器并重新开始。
void Iwdg_Feed(void);



#endif /* end of include guard: __IWDG_H */
