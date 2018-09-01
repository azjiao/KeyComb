/*
 * 开发板板载资源：按键key.
 * KEY0:接到PE4端口，上拉输入；
 * KEY1:接到PE3端口，上拉输入；
 * RESET：接到NRST，TODO...
 * WK_UP:接到PA0端口，下拉输入；
 */
#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
#include "stdio.h"

//声明键盘结构变量。
KeypadType Keypad;
//声明键盘缓冲区结构变量。
//如果定义为static则具有内部链接，其他文件将不能访问KeyBuf.
KeyBufType KeyBuf, KeyBufRelease;

//板载Key初始化。
void key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //声明初始化数据结构。

    //GPIOA和GPIOE时钟使能。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    //KEY0和KEY1设置相同。
    GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_4 | GPIO_Pin_3);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; //上拉输入模式。
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    //WK_UP设置。
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD; //下拉输入模式。
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //键盘缓冲区初始化
    KeyBuf.u16Index = 0;
    KeyBufRelease.u16Index = 0;
    for(int i = 0; i < KEYBUFSIZE; i++)
    {
        KeyBuf.u32KeyBuffer[i] = 0x0000;
        KeyBufRelease.u32KeyBuffer[i] = 0x0000;
    }
}

//key0 扫描程序
//bSusKey==TRUE：连续按下可以扫描到连续多个值。
//bSusKey==FALSE：连续按下只扫描到一个按键值。
bool key0_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //按键上次状态。
    //经过防抖处理后检测到按键压下。
    bool bKey;
    //防抖动检测定时器。
    static TimerType timer;
    //返回值。
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // 防抖处理。
    bKey = TimeON(KEY0_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //沿检测。上下沿均为TRUE，非沿时为FALSE.
    //如果是连续按键模式。
    if(bSusKey)
        bsLastKey = FALSE;  //当是连续模式时，把上次按键状态置为FALSE。
    //如果是单按键模式。
    else
        bsLastKey = bKey;


    return bRet;
}

//key1 扫描程序
//bSusKey==TRUE：连续按下可以扫描到连续多个值。
//bSusKey==FALSE：连续按下只扫描到一个按键值。
bool key1_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //按键上次状态。
    //经过防抖处理后检测到按键压下。
    bool bKey;
    //防抖动检测定时器。
    static TimerType timer;
    //返回值。
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // 防抖处理。
    bKey = TimeON(KEY1_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //沿检测。上下沿均为TRUE，非沿时为FALSE.
    //如果是连续按键模式。
    if(bSusKey)
        bsLastKey = FALSE;  //当是连续模式时，把上次按键状态置为FALSE。
    //如果是单按键模式。
    else
        bsLastKey = bKey;

    return bRet;
}


//WKUP 扫描程序
//bSusKey==TRUE：连续按下可以扫描到连续多个值。
//bSusKey==FALSE：连续按下只扫描到一个按键值。
bool WKUP_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //按键上次状态。
    //经过防抖处理后检测到按键压下。
    bool bKey;
    //防抖动检测定时器。
    static TimerType timer;
    //返回值。
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // 防抖处理。
    bKey = TimeON(WKUP_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //沿检测。上下沿均为TRUE，非沿时为FALSE.
    //如果是连续按键模式。
    if(bSusKey)
        bsLastKey = FALSE;  //当是连续模式时，把上次按键状态置为FALSE。
    //如果是单按键模式。
    else
        bsLastKey = bKey;

    return bRet;
}

//生成键盘组合状态值，当key0按下时u16Keypad第0位为1，当key1按下时u16keypad第1位为1，同理第2位是WKUP的。
uint16_t genKeyCombVal(void)
{
    uint16_t u16KeyCombVal = 0;
    if(KEY0_CODE)
        u16KeyCombVal |= KP_KEY0;
    if(KEY1_CODE)
        u16KeyCombVal |= KP_KEY1;
    if(WKUP_CODE)
        u16KeyCombVal |= KP_WKUP;

    return u16KeyCombVal;
}

//键盘按键状态扫描。
//替代单独按键检测函数，可以检测多个按键或单个按键。
//具有按键防抖功能。
//支持连续按下同一组按键产生连续多个同一个键盘状态值，存放在Keypad.u16ValSus中。
//支持连续按下同一组按键只产生一个键盘状态值，存放在Keypad.u16ValOnce中。
//函数返回值为Keypad.u16DownTrg，是按下突变键,供简单独立按键使用。
uint16_t keypadScan(void)
{
    static uint16_t u16KeypadRTTmp = 0x0000; //上一次实时键盘状态暂存值。
    uint16_t u16KeypadRT;  //本次实时按键状态值。
    uint16_t u16KeypadRTTrg;   //键实时发生沿变化。

    static uint16_t u16KeypadStabValTemp = 0x0000; //上一次检出的键盘状态值。
    uint16_t u16KeypadStabTrg;  //键检出发生按下或释放标识：检测到沿变化。

    //防抖动检测定时器。
    static TimerType timer;
    //按下同一个键检测出多个按键录入缓冲区的定时器。
    static TimerType timer2;
    //键盘稳定时间计时器。
    static TimerType timer3;
    //按下同一个键检测出多次按键录入缓冲区的标识。
    static bool bMultiFlag = FALSE;
    //按下键值暂存
    static uint16_t u16ValSusTemp = 0x0000;

    //检测键盘实时状态值.
    u16KeypadRT = genKeyCombVal();

    //防抖处理：当前后两次检测到的键盘状态值相同时才会使定时器继续工作。
    //并持续150ms后检出。
    //当按键两次检测不一样时检出值为上一次的检出值.
    //这样，当键按下和释放时都会有150ms的延时。
    u16KeypadRTTrg = u16KeypadRT ^ u16KeypadRTTmp;
    TimeON(!u16KeypadRTTrg, KEYFILTIME, &timer);
    u16KeypadRTTmp = u16KeypadRT;
    if(timer.bQ)
    {
        Keypad.u16ValSus = u16KeypadRT;
    }
    //如果不需要释放防抖，则else句可以取消。
    else
        Keypad.u16ValSus = u16KeypadStabValTemp;

    //其上经过防抖，检出了稳定键盘值。
    //沿检测：发生了突变。
    u16KeypadStabTrg = (Keypad.u16ValSus ^ u16KeypadStabValTemp);
    //按下突变键检测。
    Keypad.u16DownTrg = Keypad.u16ValSus & u16KeypadStabTrg;
    //释放突变键检测。
    Keypad.u16UpTrg = ~Keypad.u16ValSus & u16KeypadStabTrg;

    //只触发一次检测,检出和上次不一样的状态值。用来实现连续按下同一组键时只能检出一个状态值。。
    if(u16KeypadStabTrg)
        Keypad.u16ValOnce = Keypad.u16ValSus;
    else
        Keypad.u16ValOnce = 0;

    u16KeypadStabValTemp = Keypad.u16ValSus;

    //键盘按下的键值存入暂存。
    if(Keypad.u16DownTrg)
        u16ValSusTemp = Keypad.u16ValSus;

    //键盘释放缓冲区写
    //检出一次键盘稳定动作过程.
    static bool bKeyAct = FALSE;
    bKeyAct = !u16KeypadStabTrg && Keypad.u16ValSus;
    //如果键盘检测到释放，则把键盘值及时间累计值写入键盘释放缓冲区。
    //if(Keypad.u16UpTrg)
    if(Keypad.u16UpTrg)
    {
        uint32_t KeyVal;
        KeyVal = (timer3.uEt << 16) | u16ValSusTemp;
        KeyBufW(KeyVal, FALSE, &KeyBufRelease);
        //键盘释放后的键值存入暂存。
        u16ValSusTemp = Keypad.u16ValSus;
    }

    //时间累计
    TimeACC(bKeyAct, &timer3);

    //键盘按下缓冲区写操作：
    //使用KeypadScan()扫描键盘，所以会产生粘连键组合为一个键值的情况，后续程序获取键盘缓冲区键值时可以处理此种情形。
    //一般用于单独按键来产生键值,即按下A释放再按其他键。不排除使用粘连组合的情形。
    //如果一个键被按下的时间超过了KEYSPACETIME时间，则被认为是需要重复录入的键值。

    //当检出长按录入重复一次键值到缓冲区并使定时器复位后重新开放检测。
    if(!timer2.bQ && bMultiFlag)
        bMultiFlag = FALSE;

    //按下同一个键（同一个键盘值）时检测是否录入多次键值。
    TimeON(!Keypad.u16DownTrg && !bMultiFlag && Keypad.u16ValSus, KEYSPACETIME, &timer2);
    //键盘缓冲区写:当按下时或长按把当前键盘值录入缓冲区。
    if(Keypad.u16DownTrg || timer2.bQ)
        KeyBufW(Keypad.u16ValSus, TRUE, &KeyBuf);

    //当检出长按录入重复一次键值到缓冲区后关闭定时器。
    if(timer2.bQ)
        bMultiFlag = TRUE;


    return Keypad.u16DownTrg;
}

//键盘缓冲区FIFO 写操作。
//向缓冲区尾部写入一个单元值。
//如果缓冲区满还没有被其他程序读出，则缓冲区内的首单元被移出缓冲区，会导致按键遗漏。需要用户程序尽快读取录入的键值,以免遗漏按键。
void KeyBufW(uint32_t KeyVal, bool bNext, KeyBufType* pKeyBuf)
{
    //如果是写下一个数据。
    if(bNext)
    {
        //如果键盘缓冲区索引越界，则首先把缓冲区向头部移动一个单元。
        if(pKeyBuf->u16Index >= KEYBUFSIZE)
        {
            for(uint16_t i = 0; i < (KEYBUFSIZE-1); i++)
                pKeyBuf->u32KeyBuffer[i] = pKeyBuf->u32KeyBuffer[i+1];
            pKeyBuf->u16Index--;
        }

        pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index++] = KeyVal;       
    }
    //如果是上一次的数据覆盖。
    else{
        if(pKeyBuf->u16Index > 0)
           pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index - 1] = KeyVal;
        //如果是第0单元，则特殊处理。
        else
            pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index++] = KeyVal;
    }

}

//键盘缓冲区FIFO 读操作。
//从缓冲区头部读出一个单元值并把缓冲区值向头部移动一个单元。
uint32_t KeyBufR(bool bNext, KeyBufType* pKeyBuf)
{
    uint32_t KeyVal;
        
    if(pKeyBuf->u16Index > 0)
    {
        KeyVal = pKeyBuf->u32KeyBuffer[0];

        //如果是正常出对，缓冲区向头部移动一个单元。
        //如果是重复读取头部单元，则不进行移动。
        if(bNext)
        {
            for(uint16_t i = 0; i < (pKeyBuf->u16Index-1); i++)
                pKeyBuf->u32KeyBuffer[i] = pKeyBuf->u32KeyBuffer[i+1];
            pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index-1] = 0x0000;
            pKeyBuf->u16Index--;
        }
    }
    else
        KeyVal = 0x0000;    
    
    return  KeyVal;

}
