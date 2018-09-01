/*
 * 板载SysTick定时器。
 * 时钟源：采用FCLK，为AHB，72MHz.
 * 定时单位为1ms,装载值为 72000000/1000=72000UL
 * 不作1us定时。
 * 定时中断中将产生1ms计数值，为static unsigned int 类型，详见 结构体。
 * 可以使用来作为暂停延时（采用查询等待方式），也可以使用来作为不暂停的定时器使用。
 */

#ifndef __DELAY_H
#define __DELAY_H
#include "template.h"

// 1ms装载值。
// 板载时钟SystemCoreClock为72MHz,本系统预置AHB等于SystemCoreClock,SysTick时钟源设置为AHB.
// 72000000/1000=72000.
#define RELOAD_1MS   SystemCoreClock/1000U
// 500us装载值。
// 500us装载值是1ms装载值的一半。
// 72000000/2000=36000.
#define RELOAD_500US    SystemCoreClock/2000U

// 延时器,配合SysTick使用。
static struct {
    u32 uTimer_ms;  // ms延时器,用于查询延时。
    bool bPlus_ms;  // ms脉冲信号,0.5msON,0.5msOFF.
}Timer;

// 脉冲型定时器结构
typedef struct {
    u32  uEt; // 定时计数当前值。
    bool bTemp; //脉冲暂存信号。
    bool bQ;    //定时时间到标识。
}TimerType;


// 定时器初始化。
void delay_Init(void);

// ms级delay。
void delay_ms(u32 utime_ms);
// s级delay.
void delay_s(u32 utime_s);

// 延时接通定时器
// 当bEnb为TRUE时开始定时，定时单位为1ms。
// 当bEnb为FALSE时复位定时器。
// 当定时到达后如果没有复位定时器则定时器当前计数值uEt保持不变。
bool TimeON(bool bEnb, u32 uPt, TimerType *timer);

//时间累计器
//当bEnb为TRUE时计时开始，时间单位为ms。
//当bEnb为FALSE时复位计时器。
//时间累计值最大为32为无符号整数，为4294967295ms，约47天。
//当累计时间越限时自动复位为0.
uint16_t TimeACC(bool bEnb, TimerType *timer);

#endif /* end of include guard: __DELAY_H */
