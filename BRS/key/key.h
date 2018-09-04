/*********************************************************
功能： 键盘驱动
描述： 键的录入分为独立按键和组合按键，组合按键的方式有两种：组合粘连键和序列按键。
       粘连键是按下A不放再按下B形成的组合键，序列键是按下A释放再按下B释放形成的序列按键。
       序列键需要在一定的短暂时间内触发的按键序列才被认为是序列键。
设计： azjiao
版本： 0.1
日期： 2018年08月30日
*********************************************************/

#ifndef __KEY_H
#define __KEY_H
#include "template.h"
#include "bitBand.h"

#define  __BITBAND_KEY
// 使用位带操作KEY。
#ifdef  __BITBAND_KEY
#define KEY0    PEin(4)
#define KEY1    PEin(3)
#define WKUP    PAin(0)
#else
//定义KEY0、KEY1、Wk_UP三个宏为相应key的输入真值。
//此为真值，KEY0和KEY1按下时值为0，WKUP按下时值为1.
#define KEY0   GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)
#define KEY1   GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define WKUP  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#endif

//键盘缓冲区尺寸
#define KEYBUFSIZE    16
//以下是使用键盘扫描功能keypadScan(非单独按键功能)所需键值定义。
//必须和genKeypadVal()所产生的键值保持一致。不能使键值出现多位为1的情况。故最多可以有16个按键。
//如果一个键值有多位为1的情况，那么当出现粘连键同时动作时就无法区分是多键还是一个键。
//如果需要超过16个按键，请扩展键值表示的16进制数或避免同时按下多个键而且检测顺序。
//改为枚举类型定义键值。
typedef enum {
    KP_KEY0  =  0x0001,
    KP_KEY1  =  0x0002,
    KP_WKUP  =  0x0004
}KeyValType;

//#define KP_KEY0    0x0001
//#define KP_KEY1    0x0002
//#define KP_WKUP    0x0004

//通过直接读取按键端口真值来获取按键编码值。
//定义三个键按下时的独立编码值.可以用于按下释放检测也可以用于键编码值检测。
//注意：键的真值由KEY0/KEY1/WKUP三个宏来定义.
#define KEY0_CODE   !KEY0?  KP_KEY0: 0x0000
#define KEY1_CODE   !KEY1?  KP_KEY1: 0x0000
#define WKUP_CODE   WKUP?   KP_WKUP: 0x0000

//按键防抖动滤波常数：ms
#define KEYFILTIME   100U
//按下同一键值超过KEYSPACETIME的时间是否产生重复键值录入缓冲区。TRUE:允许重复，FALSE：只算一次。
#define ISREPEATKEY    FALSE

#if ISREPEATKEY
//同一个键连续按键的间隔时间，超过此时间才产生下一个键值，否则认为是同一次按键。此时间应大于防抖时间。
#define KEYSPACETIME    500U
#endif

//按键状态定义:必须只有4bit定义，共可以定义15种状态，已经足够了。
//和键值状态组合时占据16-19bit，和0-16bit键值组合成复合键值。20-23bit是真正的键值状态.24-31bit是键值出现次数。
//按下
#define PRESS_STATUS    0x1
//键盘全释放为0
#define RELEASE_STATUS    0x2
//非序列键
#define NOSEQ_STATUS   0x3
//长按
#define LONGPR_STATUS   0x4
//双击事件
#define DOUBLECLICK_EVENT    0x05


//长按状态检测时间:2000ms,即2s.
#define LONGPR_TIME    2000U
//序列按键独立键值最长按键时间：超过此时间认为不符合要求。
#define KEYSEQPR_TIME    1000U
//序列键：序列键间隔时间，只有在此时间内的序列键才认为是组合键，否则不认为是组合键。
//该值根据用户体验，最好设置成1000U,即1s，太长会影响操作的响应，用户体验到迟钝。太短又会在序列按键（非双击）时对用户操作的速度的要求过高。
#define KEYSEQSPACE_TIME    1000U

//键盘结构
typedef struct
{
    uint16_t u16ValSus; //当前键盘状态值(连续状态值).
    uint16_t u16ValOnce; //连续按下同一组键只产生一个值.
    uint16_t u16KeypadStabTrg; //稳定键值突变。
    uint16_t u16DownTrg;  //键盘按下突变键,其实和u16ValOnce是同一逻辑。
    uint16_t u16UpTrg;  //键盘释放突变键
    uint32_t u32KeyStatus; //键值状态，低16位是键值,bit16-bit19是键值状态，低20位构成复合键值。bit20-23位是当前键值是按下还是释放，高8位保留。
}KeypadType;
extern KeypadType Keypad;

//键盘缓冲区结构:FIFO.
//缓冲区存放的是单个按键的键值序列。
//单个按键的生成：检出稳定的键值，如果稳定的键值超过了预定义的间隔时间仍然有效则算作第二个键值。
//以此类推，每个连续的同一个键值的间隔时间为Keypad.u16SpaceTime.
typedef struct
{
    uint32_t u32KeyBuffer[KEYBUFSIZE];
    uint16_t u16Index; //缓冲区尾指针。
} KeyBufType;
//键盘按下缓冲区.
extern KeyBufType KeyBuf;

//板载Key初始化。
void key_Init(void);

//Key0扫描程序。
//bSusKey==TRUE:连续按下触发多次。
//bSusKey==FALSE:连续按下当作一次按下处理。
bool key0_Scan( bool bSusKey );
//Key1扫描程序。
bool key1_Scan( bool bSusKey );
//WK_UP扫描程序。
bool WKUP_Scan( bool bSusKey );

//生成键盘组合状态值，当key0按下时KEYPAD第0位为1，当key1按下时KEYPAD第1位为1，同理第3位是WKUP的。
uint16_t genKeyCombVal(void);

//键盘按键状态检测。
//替代单独按键检测函数，可以检测多个按键或单个按键。
//具有按键防抖功能。
uint16_t keypadScan(void);
//键值状态生成。
void genKeyStatus(void);
//状态机.
void KeyStatusMachine(void);

//键盘缓冲区FIFO 写操作。
void KeyBufW(uint32_t KeyVal, bool bNext, KeyBufType* KeyBuf);
//键盘缓冲区FIFO 读操作。
uint32_t KeyBufR(bool bNext, KeyBufType* KeyBuf);


//根据项目需要定义所需按键组合，如果是单个按键就不必另外定义了。
//组合键如果有顺序要求在用于时必须使用Keypad.u16ValOnce而不是Keypad.u16ValSus来判断.
//KEY0-WKUP组合，且后按下WKUP。
//FIXME:本宏定义值部分不能被()包围，因为是先用&&前面的部分和键值比较再判断“与”的。
#define KEY0_WKUP    (KP_KEY0 + KP_WKUP) && (Keypad.u16DownTrg == KP_WKUP)
//KEY1-KEY0组合，且后按下KEY0.
//FIXME:本宏定义值部分不能被()包围，因为是先用&&前面的部分和键值比较再判断“与”的。
#define KEY1_KEY0    (KP_KEY1 + KP_KEY0) && (Keypad.u16DownTrg == KP_KEY0)
//KEY0-KEY1组合，无顺序要求。
#define KEY0_KEY1    ((KP_KEY0 + KP_KEY1))
//KEY0双击
#define KEY0_DOUBLE    (KP_KEY0 | DOUBLECLICK_EVENT<<16)
//KEY1双击
#define KEY1_DOUBLE    (KP_KEY1 | DOUBLECLICK_EVENT<<16)

#endif /* ifndef __KEY_H */
