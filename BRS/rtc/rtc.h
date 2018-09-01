/*********************************************************
功能： 实时时钟RTC头文件
描述： RTC初始化、RCT当前时间初始化、实时时间获取。
设计： azjiao
版本： 0.1
日期： 2018年08月09日
*********************************************************/
#ifndef __RTC_H
#define __RTC_H
#include "dev.h"
#include "template.h"

//日期时间结构.
typedef struct {
    uint16_t u16Year;
    uint16_t u16Mon;
    uint16_t u16Day;
    uint16_t u16Hour;
    uint16_t u16Min;
    uint16_t u16Sec;
}dtStruct;
extern dtStruct Calendar;

//闰年判断
//如果年份能被4整除 且 不能被100整除但年份可以被400整除，则为闰年。
//不能被100整除但可以被400整除，是因为按照四年一闰的做法导致400年中多出了3个闰年需要撤销。
bool Is_LeapYear(uint16_t u16Year);

//日期时间转换为日历时间(秒数)
//锚点是1970-1-1-0:0:0
//不作时区转换，一律是本地时间。
//函数不改变dt内容。
uint32_t Dt2Sec(dtStruct *dt);

//日历时间(秒数)转换为日期时间
//锚点是1970-1-1-0:0:0
//不作时区转换，一律看作本地时间。
//转换后的日期时间存入dt。
void Sec2Dt(uint32_t u32Sec, dtStruct* dt);

//设置RTC时间
//调用Dt2Sec()把日期时间转换为日历时间，然后写入RTC的CNT。
//函数内对dt内容进行判断，需要1900-1-1至2105-12-31之间的日期。
//写入正确返回0，否则返回非零错误码。
//函数不改变dt内容。
bool Set_RTC(dtStruct *dt);

//设置RTC的闹钟时间。
//基本用法同Set_RTC()，只不过写入的寄存器不同。
bool Set_Alarm(dtStruct *dt);


//RTC初始化.
bool RTC_Init(void);



#endif /* end of include guard: __RTC_H */
