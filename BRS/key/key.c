/*
 * 开发板板载资源：按键key.
 * KEY0:接到PE4端口，上拉输入；
 * KEY1:接到PE3端口，上拉输入；
 * RESET：接到NRST，TODO...
 * WK_UP:接到PA0端口，下拉输入；
 */
#include "stm32f10x.h"
//#include "key.h"
//#include "delay.h"
//#include "stdio.h"
#include "dev.h"

//声明键盘结构变量。
KeypadType Keypad;
//声明键盘缓冲区结构变量。
//如果定义为static则具有内部链接，其他文件将不能访问KeyBuf.
KeyBufType KeyBuf;

//板载Key初始化。
void key_Init(void)
{ 
    //GPIOA和GPIOE时钟使能。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    //使用二次封装的GPIO初始化函数。
    //对于输入方式，频率参数无用。    
    My_GPIO_Init_Macro(GPIOE, (GPIO_Pin_4 | GPIO_Pin_3), GPIO_Mode_IPU); //KEY0和KEY1初始化设置相同,上拉输入模式。
    My_GPIO_Init_Macro(GPIOA, GPIO_Pin_0, GPIO_Mode_IPD); //WK_UP初始化设置,下拉输入模式。
    
    //键盘缓冲区初始化
    KeyBuf.u16Index = 0;
    for(int i = 0; i < KEYBUFSIZE; i++)
    {
        KeyBuf.u32KeyBuffer[i] = 0x0;
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
    static uint16_t u16KeypadRTTmp = 0x0; //上一次实时键盘状态暂存值。
    uint16_t u16KeypadRT;  //本次实时按键状态值。
    uint16_t u16KeypadRTTrg;   //键实时发生沿变化。
    static uint16_t u16KeypadStabValTemp = 0x0; //上一次检出的键盘状态值。
    //防抖动检测定时器。
    static TimerType timer;

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
    Keypad.u16KeypadStabTrg = (Keypad.u16ValSus ^ u16KeypadStabValTemp);
    //按下突变键检测。
    Keypad.u16DownTrg = Keypad.u16ValSus & Keypad.u16KeypadStabTrg;
    //释放突变键检测。
    Keypad.u16UpTrg = ~Keypad.u16ValSus & Keypad.u16KeypadStabTrg;

    //只触发一次检测,检出和上次不一样的状态值。用来实现连续按下同一组键时只能检出一个状态值。。
    if(Keypad.u16KeypadStabTrg)
        Keypad.u16ValOnce = Keypad.u16ValSus;
    else
        Keypad.u16ValOnce = 0;

    u16KeypadStabValTemp = Keypad.u16ValSus;

    //键值状态生成。
    genKeyStatus();
    //状态机检测双击事件.
    KeyStatusMachine();
    return Keypad.u16DownTrg;
}

//键值状态生成。
//键值状态是32位无符号整数：低16位为键值原型，bit16-19是键值上个状态的记录，低20位合并成一个键值。
//bit20-23位是键值当前存在（按下）和不存在(释放)的状态。PRESS_STATUS为按下RELEASE_STATUS是释放。
//bit24-31保留。
//键值状态有：发生按下、按下超过序列键时间、按下超过长按时间、发生释放、释放超过序列键间隔时间（键盘彻底无键值）。
void genKeyStatus(void)
{
    #if ISREPEATKEY
    //按下同一个键检测出多个按键录入缓冲区的定时器。
    static TimerType timer2;
    //按下同一个键检测出多次按键录入缓冲区的标识。
    static bool bMultiFlag = FALSE;
    #endif

    //按下键值暂存
    /*static uint16_t u16ValSusTemp = 0x0;*/
    static bool bKeyAct = FALSE;  //键值稳定且不为0.
    //键盘稳定键值期时间计时器。
    static TimerType timer3;
    static bool bReset = FALSE;  //键值稳定定时器复位。

    //序列键间隔定时器。
    static TimerType timer4;
    static bool bSeqSpaceEnb = FALSE;  //序列键间隔定时使能。


    //键盘按下缓冲区写操作：
    //使用KeypadScan()扫描键盘，所以会产生粘连键组合为一个键值的情况，后续程序获取键盘缓冲区键值时可以处理此种情形。
    //一般用于单独按键来产生键值,即按下A释放再按其他键。不排除使用粘连组合的情形。

    //如果ISREPEATKEY为TRUE,一个键被按下的时间超过了KEYSPACETIME时间，则被认为是需要重复录入的键值。
    #if ISREPEATKEY
    //当检出长按录入重复一次键值到缓冲区并使定时器复位后重新开放检测。
    if(!timer2.bQ && bMultiFlag)
        bMultiFlag = FALSE;

    //按下同一个键（同一个键盘值）时检测是否录入多次键值。
    TimeON(!Keypad.u16DownTrg && !bMultiFlag && Keypad.u16ValSus, KEYSPACETIME, &timer2);
    //键盘缓冲区写:当按下时或长按把当前键盘值录入缓冲区。
    if(Keypad.u16DownTrg || timer2.bQ)
        KeyBufW(Keypad.u16ValSus, TRUE, &KeyBuf);
    if(Keypad.u16DownTrg)
    {
            //设置键值状态：存放在16-23位。
        Keypad.u32KeyStatus = Keypad.u16ValSus | (PRESS_STATUS << 16) | (PRESS_STATUS << 20);
    }
    //当检出长按录入重复一次键值到缓冲区后关闭定时器。
    if(timer2.bQ)
        bMultiFlag = TRUE;

    //如果ISREPEATEKEY为FLASE，一个键被按下无论多长时间都只录入一次键值。
    #else
        if(Keypad.u16DownTrg)
        {
            KeyBufW(Keypad.u16ValSus, TRUE, &KeyBuf);
            //设置键值状态：存放在16-23位。
            Keypad.u32KeyStatus = Keypad.u16ValSus | (PRESS_STATUS << 16) | (PRESS_STATUS << 20);
        }
    #endif

    //键值稳定且有值。
    bKeyAct = !Keypad.u16KeypadStabTrg && Keypad.u16ValSus;
    //当有键值突变时允许定时器工作。
    if(Keypad.u16KeypadStabTrg)
        bReset = FALSE;

    //键值稳定持续时间检测
    //特殊按键事件包含任何稳定的键盘值长按，无论是按下产生还是释放产生，但键值为0除外，也就是释放后键盘没有键值除外。
    //时间累计
    TimeACC(bKeyAct && !bReset, &timer3);

    //以下检测两个时间使用同一个定时器：序列键超时和长按超时，定时较短的序列键超时必须位于长按超时前面被执行，这样才能保证定时较长的长按键超时会覆盖序列键超时。
    //如果按键持续事件超过KEYSEQPR_TIME，则认为不是序列按键。
    if(timer3.uEt >= KEYSEQPR_TIME)
    {
        //设置键值状态,存放在16-23位。
        Keypad.u32KeyStatus = Keypad.u16ValSus | (NOSEQ_STATUS << 16) | (PRESS_STATUS << 20);
    }

    //长按事件。
    if(timer3.uEt >= LONGPR_TIME)
    {
        uint32_t KeyVal;
        KeyVal =  Keypad.u16ValSus| ((uint32_t)(LONGPR_STATUS << 16));
        KeyBufW(KeyVal, TRUE, &KeyBuf);
        //设置键值状态,存放在16-23位。
        Keypad.u32KeyStatus = KeyVal | (PRESS_STATUS << 20);
        bReset = TRUE;
    }

    //如果键值为0且键值状态处于非0状态，设置键值状态,键值维持之前的值。
    if(!Keypad.u16ValSus && Keypad.u32KeyStatus)
    {
        Keypad.u32KeyStatus &= 0xFFFFF; //保留低20位数据。
        Keypad.u32KeyStatus |= RELEASE_STATUS << 20;
        bSeqSpaceEnb = TRUE;        
    }
    else
        bSeqSpaceEnb = FALSE;
    TimeON(bSeqSpaceEnb, KEYSEQSPACE_TIME, &timer4);
    //如果键盘键值为0时间超过KEYSEQSPACE_TIME定义的序列键间隔时间，设置键值状态清零复位。
    if(timer4.bQ)
    {
        Keypad.u32KeyStatus = 0x0;
        bSeqSpaceEnb = FALSE;           
    }     

}

//状态机.
//双击数据结构：支持单键双击，当然也支持多键双击，不过恐怕你操作不过来。
struct {
        uint8_t u8State; //状态机状态。
        uint32_t uiKeyValOld; //上次键值。
        uint8_t u8Count;  //完整状态次数。
    }DoubleClick;

void KeyStatusMachine(void)
{
    uint16_t u16curKeyStatus; //当前键值状态。
    uint32_t uicurKeyVal;  //当前键值。
    static uint32_t u32KeyStatusOld = 0x0;  //上次键值状态字。

    //如果发生键值状态改变才执行状态机。
    if(u32KeyStatusOld ^ Keypad.u32KeyStatus)
    {
        uicurKeyVal = (Keypad.u32KeyStatus & 0xFFFFF);  //取得当前键值:20位复合键值。
        u16curKeyStatus = (Keypad.u32KeyStatus & 0xFFF00000)>>20; //取得当前键值状态.

        //判断第一个状态起始。
        if((DoubleClick.uiKeyValOld == 0x0) && (u16curKeyStatus == PRESS_STATUS))
        {
            DoubleClick.uiKeyValOld = uicurKeyVal;
            DoubleClick.u8Count = 0x0;
            DoubleClick.u8State = 0x1;
        }
        //判断结束状态。
        //除了完成双击或检测到非双击自动复位状态外，当键盘检测到无键值状态（无键值且持续超过序列键检测时间）自动复位。
        if(Keypad.u32KeyStatus == 0x0)
        {
            DoubleClick.uiKeyValOld = 0x0;
            DoubleClick.u8Count = 0x0;
            DoubleClick.u8State = 0x0;
        }

        switch(DoubleClick.u8State){
            case 0x01:
                //判断第一个状态
                if(!(uicurKeyVal ^ DoubleClick.uiKeyValOld) && (u16curKeyStatus == PRESS_STATUS))
                {
                    DoubleClick.uiKeyValOld = uicurKeyVal;
                    DoubleClick.u8State++;
                }
                //否则就是键值改变，双击无效。
                else {
                    //事件检测结束，复位状态字。
                    DoubleClick.uiKeyValOld = 0x0;
                    DoubleClick.u8Count = 0x0;
                    DoubleClick.u8State = 0x0;
                }
                break;

            case 0x02:
                //双击检测到第一个状态后开始第二状态检测.
                //检测是否是释放状态:键值一致且键值状态当前是释放。
                if(!(uicurKeyVal ^ DoubleClick.uiKeyValOld) && u16curKeyStatus == RELEASE_STATUS)
                {
                    DoubleClick.uiKeyValOld = uicurKeyVal;
                    DoubleClick.u8Count++;
                    DoubleClick.u8State = 0x1;
                    if(DoubleClick.u8Count == 0x02)
                    {
                        //把双击事件录入缓冲区。
                        uicurKeyVal &= 0xFFFF;  //取得键值原码。
                        KeyBufW(uicurKeyVal | DOUBLECLICK_EVENT << 16, TRUE, &KeyBuf);
                        //事件检测结束，复位状态字。
                        //为了不迟滞用户体验，即检测双击又检测三击最好另外设立三击状态检测字,不和双击共用。
                        DoubleClick.uiKeyValOld = 0x0;
                        DoubleClick.u8Count = 0x0;
                        DoubleClick.u8State = 0x0;
                    }
                }
                //否则，是按键超时。
                else {
                    //事件检测结束，复位状态字。
                    DoubleClick.uiKeyValOld = 0x0;
                    DoubleClick.u8Count = 0x0;
                    DoubleClick.u8State = 0x0;
                }
                break;
        }
    }
    u32KeyStatusOld = Keypad.u32KeyStatus;    
}

//键盘缓冲区FIFO 写操作。
//向缓冲区尾部写入一个单元值。
//如果缓冲区满还没有被其他程序读出，则缓冲区内的首单元被移出缓冲区，会导致按键遗漏。需要用户程序尽快读取录入的键值,以免遗漏按键。
//FIXME:增加bNext参数，实现对上一次值的覆盖和正常入队。
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
//FIXME：增加bNext参数，实现对首单元的重复读取和正常出对。
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
