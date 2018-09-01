/*********************************************************
功能： 实时时钟RTC功能实现
描述： RTC初始化、RTC当前时间初始化、实时时间获取。
设计： azjiao
版本： 0.1
日期： 2018年08月09日
*********************************************************/
#include "rtc.h"
#include <stdio.h>
//月份
//储存的是月份之前的天数。
//月份从0-11对应1-12月。
//2月按28天储存。
const uint16_t mon1_table[12]={0,31,59,90,120,151,181,212,243,273,304,334};
dtStruct Calendar;

//RTC 初始化。
//1、需要访问后备寄存器BKP.
//2、设置低速外部时钟LSE使用外部晶振(频率32.768kHz)而不是外部时钟源.
//3、设置RTC时钟源为LSE。
//4、设置RTC的预分频值为32767，以获得1Hz计数信号。
//5、设置RTC秒中断。
//正常返回0，否则返回1.
bool RTC_Init(void)
{
    uint8_t u8LSECfgDealy = 0; //设置LSE使用外部晶振后监测LSE起振时间。
    //使能PWR和BKP外设
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //使能后备寄存器访问
    PWR_BackupAccessCmd(ENABLE);

    //监测是否是第一次配置RTC，如果是则进行配置。
    //主要配置LES使用晶振还是外部时钟源、RTC时钟选择LSE还是LSI。
    //以及RTC的预分频值。
    if(BKP_ReadBackupRegister(BKP_DR1) != 0x5051)
    {
        //根据需要决定是否复位后备寄存器区域
        BKP_DeInit();

        //设置LSE使用外部晶振，而不是外部时钟。
        RCC_LSEConfig(RCC_LSE_ON);
        //等待LSE工作正常：LSE起振。
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET && u8LSECfgDealy < 250)
        {
            u8LSECfgDealy++;
            delay_ms(10);
        }
        //如果LSE配置后超过2.5s仍然不正常，则认为LSE晶振不工作了，返回0失败。
        if(u8LSECfgDealy >= 250)
            return 1;  

        //设置RTC时钟源RTCCLK为LSE
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        //使能RTC时钟.
        RCC_RTCCLKCmd(ENABLE);

        //等待上一次对RTC寄存器的写操作完成。
        RTC_WaitForLastTask();

        //由于需要对RTC_CRH 寄存器读以便设置秒中断，所以需要等待RTC寄存器同步完成.
        //一次同步后，以后就可以随时读取了，不必每次读时都同步。
        RTC_WaitForSynchro();
        
        //使能RTC秒中断。
        //RTC_ITConfig(RTC_IT_SEC, ENABLE);
        //RTC_WaitForLastTask();

        //设置RTC预分频值
        RTC_SetPrescaler(32767);
        RTC_WaitForLastTask();

        //设置RTC初始化时间，可以不用在此设置。
        dtStruct dt; //日期时间结构变量。
        //以编程时的日期时间为验证。
        dt.u16Year = 2018;
        dt.u16Mon = 8;
        dt.u16Day = 9;
        dt.u16Hour = 19;
        dt.u16Min = 40;
        dt.u16Sec = 0;
        Set_RTC(&dt);        
        RTC_WaitForLastTask(); 

        //向后备寄存器中的DR1写入标识数据，标志已经对RTC进行了配置。
        BKP_WriteBackupRegister(BKP_DR1, 0x5051);
    }
    //可以不用else，无论是否已经配置LSE、RTC预分频值都执行配置RTC中断。
    //因为中断配置数据不在BKP中也不在RTC核心里面。

    RTC_WaitForSynchro();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();

    //RTC中断优先级设置
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    return 0;
}

//RTC中断服务函数
void RTC_IRQHandler(void)
{
    //秒中断。
    if(RTC_GetITStatus(RTC_IT_SEC) == SET)
    {
       //获取时钟。
       uint32_t u32Sec = RTC_GetCounter();
       Sec2Dt(u32Sec, &Calendar);       
    }

    //闹钟中断
    //闹钟中断有单独的服务函数。
//    if(RTC_GetITStatus(RTC_IT_ALR) == SET)
//    {

//    }
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);
    RTC_WaitForLastTask();
}

//闰年判断
//如果年份能被4整除 且 不能被100整除但年份可以被400整除，则为闰年。
//不能被100整除但可以被400整除，是因为按照四年一闰的做法导致400年中多出了3个闰年需要撤销。
bool Is_LeapYear(uint16_t u16Year)
{
    if((u16Year % 4 == 0) && ((u16Year %100 != 0) || (u16Year % 400 == 0)))
        return TRUE;
    else
        return FALSE;
}

//日期时间转换为日历时间
//锚点是1970-1-1-0:0:0
//日期范围：32位unsigned int 最大值是4294967295，该值表示秒数，约136年。
//表示为从锚点算的年份约为1970年到2106年某个日期(会因为闰年有差异)。
//可靠范围为1970年1月1日到2105年12月31日。
//不作时区转换，一律是本地时间。
uint32_t Dt2Sec(dtStruct *dt)
{
    uint16_t iYear1, iMon1, iDay1;
    uint32_t u32Sec1;

    iYear1 = dt->u16Year - 1;  //年份之前的整年数。
    //计算闰年数：几个闰年就意味着需要多加几天。
    iDay1= ((iYear1/4 - iYear1/100 + iYear1/400) + iYear1*365);

    iMon1 = dt->u16Mon - 1;
    //加上月份之前的整月天数。
    iDay1 = iDay1 + (mon1_table[iMon1]);
    //如果本年是闰年且是2月以后月份,天数加1.
    if(Is_LeapYear(dt->u16Year) && (iMon1 >= 2))
        iDay1++;

    //天数加上当月内日期之前的天数。
    iDay1 += (dt->u16Day - 1);
    //减去1970年前的所有天数。
    iDay1 -= 719162;

    //计算秒数。
    u32Sec1 = ((((iDay1 *24 + dt->u16Hour) * 60) + dt->u16Min) * 60) + dt->u16Sec;
    return u32Sec1;
}

//日历时间(秒数)转换为日期时间
//锚点是1970-1-1-0:0:0
//不作时区转换，一律看作本地时间。
void Sec2Dt(uint32_t u32Sec, dtStruct* dt)
{
    uint16_t u16Day, u16Mon;
    uint32_t u32Sec_Day;  //不足一天的秒数。
    uint16_t u16Year = 1970;

    //计算整天数。
    u16Day =(uint16_t)(u32Sec / 86400);
    //计算不足一天的秒数。
    u32Sec_Day = u32Sec % 86400;

    //计算年份.
    while(u16Day >= 365)
    {
        //如果是闰年
        if(Is_LeapYear(u16Year))
        {
            if(u16Day >= 366)
                u16Day -= 366;
            else
                break;  //剩余天数为365且是闰年，结束年份拟合。
        }
        //不是闰年
        else{
            u16Day -= 365;
        }
        u16Year++;
    }
    //年份赋值
    dt->u16Year = u16Year;

    //计算月份.
    //u16Day为一年内的天数.
    u16Mon = 1;
    bool bIsLeap = Is_LeapYear(u16Year);
    while((u16Mon <= 11) && (u16Day >= ((bIsLeap && (u16Mon >= 2))? mon1_table[u16Mon]+1 : mon1_table[u16Mon])))
    {
        u16Mon++;
    }
    //要么u16Mon是12,要么中途拟合上某个月份。
    //月份赋值
    dt->u16Mon = u16Mon;

    //计算月内天数。
    //计算当天之前的月内天数。
    //如果月份小于等于2 或 非闰年。
    if(dt->u16Mon <= 2 || !bIsLeap)
        u16Day -=  mon1_table[dt->u16Mon-1];
    //如果月份大于2 且 是闰年。
    else
        u16Day -= (mon1_table[dt->u16Mon-1] +1);

    //加上当天。
    u16Day++;
    //日期赋值
    dt->u16Day = u16Day;

    //计算时间
    //u32Sec_Day是不足一天的秒数。
    dt->u16Hour = (int)(u32Sec_Day / 3600);
    dt->u16Min =  (int)((u32Sec_Day % 3600) / 60);
    dt->u16Sec =  (int)((u32Sec_Day % 3600) % 60);
}

//设置RTC时间
//调用Dt2Sec()把日期时间转换为日历时间，然后写入RTC的CNT。
//函数内对dt内容进行判断，需要1970-1-1至2105-12-31之间的日期。
//写入正确返回TRUE，否则返回FALSE。
//函数不改变dt内容。
bool Set_RTC(dtStruct *dt)
{
    uint16_t u16Day_Lim;
    //判断dt所指内容的合法性。
    if(dt->u16Mon == 1 || dt->u16Mon == 3 || dt->u16Mon == 5 || dt->u16Mon == 7 || dt->u16Mon == 8 || dt->u16Mon ==10 || dt->u16Mon == 12)
        u16Day_Lim = 31;
    else
        u16Day_Lim = 30;

    if(dt->u16Mon == 2 && (Is_LeapYear(dt->u16Year)))
        u16Day_Lim = 29;
    if(dt->u16Mon == 2 && (!Is_LeapYear(dt->u16Year)))
        u16Day_Lim = 28;

    if(!((dt->u16Year >= 1970 && dt->u16Year <= 2105) &&
                 (dt->u16Mon >=1 && dt->u16Mon <= 12) &&
                 (dt->u16Day >= 1 && dt->u16Day <= u16Day_Lim)))    
                return FALSE; //输入的日期错误。
    if(!(dt->u16Hour >= 0 && dt->u16Hour <= 23) &&
          (dt->u16Min >= 0 && dt->u16Min <= 59) &&
          (dt->u16Sec >= 0 && dt->u16Sec <= 59))
                return FALSE; //输入的时间错误。

    //输入的日期时间正确。
    //使能PWR和BKP外设。
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //使能对RTC和后备寄存器的写访问。
    PWR_BackupAccessCmd(ENABLE);

    //计算日期时间对应的日历时间秒数后写入RTC_CNT。
    RTC_SetCounter(Dt2Sec(dt));
    //等待对RTC寄存器的写操作完成。
    RTC_WaitForLastTask();

    return TRUE;
}

//设置RTC的闹钟时间。
//基本用法同Set_RTC()，只不过写入的寄存器不同。
bool Set_Alarm(dtStruct *dt)
{
    uint16_t u16Day_Lim;
    //判断dt所指内容的合法性。
    if(dt->u16Mon == 1 || dt->u16Mon == 3 || dt->u16Mon == 5 || dt->u16Mon == 7 || dt->u16Mon == 8 || dt->u16Mon ==10 || dt->u16Mon == 12)
        u16Day_Lim = 31;
    else
        u16Day_Lim = 30;

    if(dt->u16Mon == 2 && (Is_LeapYear(dt->u16Year)))
        u16Day_Lim = 29;
    if(dt->u16Mon == 2 && (!Is_LeapYear(dt->u16Year)))
        u16Day_Lim = 28;

    if(!((dt->u16Year >= 1970 && dt->u16Year <= 2105) &&
                 (dt->u16Mon >=1 && dt->u16Mon <= 12) &&
                 (dt->u16Day >= 1 && dt->u16Day <= u16Day_Lim)))
        return FALSE; //输入的日期错误。
    if(!(dt->u16Hour >= 0 && dt->u16Hour <= 23) &&
          (dt->u16Min >= 0 && dt->u16Min <= 59) &&
          (dt->u16Sec >= 0 && dt->u16Sec <= 59))
        return FALSE; //输入的时间错误。

    //输入的日期时间正确。
    //使能PWR和BKP外设。
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //使能对RTC和后备寄存器的写访问。
    PWR_BackupAccessCmd(ENABLE);

    //计算日期时间对应的日历时间秒数后写入RTC_CNT。
    RTC_SetAlarm(Dt2Sec(dt));
    //等待对RTC寄存器的写操作完成。
    RTC_WaitForLastTask();

    return TRUE;
}
