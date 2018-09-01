/*********************************************************
功能： 独立看门狗
描述： 板载资源独立看门狗
设计： azjiao
版本： 0.1
日期： 2018年08月04日
*********************************************************/
#include "iwdg.h"

//初始化独立看门狗
//u8Prer:预分频系数0～7,u8Prer对应的实际预分频值为4*2^u8Prer,最大预分频值是256.
//u16Rlr:重转载寄存器值，只有低12位有效，最大0xFFF,即4095.
//精英板的低速内部时钟LSI是40KHz,则一个脉冲计时时间为：(4*2^u8Prer)/40 ms.
//所需看门狗定时为：Tout(ms) = (4*2^u8Prer)/40 *u16Rlr.
void Iwdg_Init(uint8_t u8Prer, uint16_t u16Rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //去除PR和RLR的写保护,访问允许。
    IWDG_SetPrescaler(u8Prer); //设置预分频系数。
    IWDG_SetReload(u16Rlr);  //设置重转载寄存器值。
    IWDG_ReloadCounter();   //加载重转载寄存器值到IWDG计数器。
    IWDG_Enable();   //使能IWDG，开始工作。
}

void Iwdg_Feed(void)
{
    IWDG_ReloadCounter();  //重装载RLR值到IWDG计数器。
}
