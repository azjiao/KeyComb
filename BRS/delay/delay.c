#include <stm32f10x.h>
#include <core_cm3.h>
#include "delay.h"

// 以1ms设置SysTick.
void delay_Init(void)
{
    // SysTick配置。
    if (SysTick_Config(RELOAD_1MS))
    {
        while(1);
    }
}

// SysTick定时器中断
//每1ms中断一次，每次使bPlus_ms翻转一次，使uTimer_ms减1。
//所产生的脉冲周期为2ms，每1ms翻转一次。
void SysTick_Handler(void)
{
    Timer.bPlus_ms = !Timer.bPlus_ms;
    if(Timer.uTimer_ms != 0U)
        Timer.uTimer_ms--;
}


// ms级delay.
void delay_ms(u32 utime_ms)
{

    Timer.uTimer_ms = utime_ms;
    // 查询延时是否到，不到则继续查询。
    while( Timer.uTimer_ms != 0U )
        ;
}

// s级delay.
void delay_s(u32 utime_s)
{
    // 延时时间为utime_s倍的1000ms。
    for (int i = 0; i < utime_s; ++i)
    {
        delay_ms(1000U);
    }
}

// 延时接通定时器
// 当bEnb为TRUE时开始定时，定时单位为1ms。
// 当bEnb为FALSE时复位定时器。
// 当定时到达后如果没有复位定时器则定时器当前计数值uEt保持不变。
bool TimeON(bool bEnb, u32 uPt, TimerType *timer)
{
    if(!bEnb){
        timer->uEt = 0U;
        timer->bTemp = FALSE;
        timer->bQ = FALSE;
        return FALSE;
    }
    else{
        //if((timer->uEt < uPt) && ((Timer.bPlus_ms) & (Timer.bPlus_ms ^ timer->bTemp)))
        //每次检测到边沿(每1ms翻转一次)就加1.
        if((timer->uEt < uPt) && (Timer.bPlus_ms ^ timer->bTemp))
            timer->uEt++;

        timer->bTemp = Timer.bPlus_ms;

        if((timer->uEt >= uPt)){
            timer->bQ = TRUE;
            return TRUE;
        }
        else{
            timer->bQ = FALSE;
            return FALSE;
        }
    }
}

//时间累计器
//当bEnb为TRUE时计时开始，时间单位为ms。
//当bEnb为FALSE时复位计时器。
//时间累计值最大为32为无符号整数，为4294967295ms，约47天。
//当累计时间越限时自动复位为0.
uint16_t TimeACC(bool bEnb, TimerType *timer)
{
    if(!bEnb)
    {
        timer->uEt = 0U;
        timer->bTemp = FALSE;
        timer->bQ = FALSE;
    }
    else{
        if(Timer.bPlus_ms ^ timer->bTemp)
            timer->uEt++;

        timer->bTemp = Timer.bPlus_ms;
        //timer->bQ为TRUE意味着累计在进行。
        timer->bQ = TRUE;
    }

    return timer->uEt;
}

