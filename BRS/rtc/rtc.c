/*********************************************************
���ܣ� ʵʱʱ��RTC����ʵ��
������ RTC��ʼ����RTC��ǰʱ���ʼ����ʵʱʱ���ȡ��
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��08��09��
*********************************************************/
#include "rtc.h"
#include <stdio.h>
//�·�
//��������·�֮ǰ��������
//�·ݴ�0-11��Ӧ1-12�¡�
//2�°�28�촢�档
const uint16_t mon1_table[12]={0,31,59,90,120,151,181,212,243,273,304,334};
dtStruct Calendar;

//RTC ��ʼ����
//1����Ҫ���ʺ󱸼Ĵ���BKP.
//2�����õ����ⲿʱ��LSEʹ���ⲿ����(Ƶ��32.768kHz)�������ⲿʱ��Դ.
//3������RTCʱ��ԴΪLSE��
//4������RTC��Ԥ��ƵֵΪ32767���Ի��1Hz�����źš�
//5������RTC���жϡ�
//��������0�����򷵻�1.
bool RTC_Init(void)
{
    uint8_t u8LSECfgDealy = 0; //����LSEʹ���ⲿ�������LSE����ʱ�䡣
    //ʹ��PWR��BKP����
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //ʹ�ܺ󱸼Ĵ�������
    PWR_BackupAccessCmd(ENABLE);

    //����Ƿ��ǵ�һ������RTC���������������á�
    //��Ҫ����LESʹ�þ������ⲿʱ��Դ��RTCʱ��ѡ��LSE����LSI��
    //�Լ�RTC��Ԥ��Ƶֵ��
    if(BKP_ReadBackupRegister(BKP_DR1) != 0x5051)
    {
        //������Ҫ�����Ƿ�λ�󱸼Ĵ�������
        BKP_DeInit();

        //����LSEʹ���ⲿ���񣬶������ⲿʱ�ӡ�
        RCC_LSEConfig(RCC_LSE_ON);
        //�ȴ�LSE����������LSE����
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET && u8LSECfgDealy < 250)
        {
            u8LSECfgDealy++;
            delay_ms(10);
        }
        //���LSE���ú󳬹�2.5s��Ȼ������������ΪLSE���񲻹����ˣ�����0ʧ�ܡ�
        if(u8LSECfgDealy >= 250)
            return 1;  

        //����RTCʱ��ԴRTCCLKΪLSE
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        //ʹ��RTCʱ��.
        RCC_RTCCLKCmd(ENABLE);

        //�ȴ���һ�ζ�RTC�Ĵ�����д������ɡ�
        RTC_WaitForLastTask();

        //������Ҫ��RTC_CRH �Ĵ������Ա��������жϣ�������Ҫ�ȴ�RTC�Ĵ���ͬ�����.
        //һ��ͬ�����Ժ�Ϳ�����ʱ��ȡ�ˣ�����ÿ�ζ�ʱ��ͬ����
        RTC_WaitForSynchro();
        
        //ʹ��RTC���жϡ�
        //RTC_ITConfig(RTC_IT_SEC, ENABLE);
        //RTC_WaitForLastTask();

        //����RTCԤ��Ƶֵ
        RTC_SetPrescaler(32767);
        RTC_WaitForLastTask();

        //����RTC��ʼ��ʱ�䣬���Բ����ڴ����á�
        dtStruct dt; //����ʱ��ṹ������
        //�Ա��ʱ������ʱ��Ϊ��֤��
        dt.u16Year = 2018;
        dt.u16Mon = 8;
        dt.u16Day = 9;
        dt.u16Hour = 19;
        dt.u16Min = 40;
        dt.u16Sec = 0;
        Set_RTC(&dt);        
        RTC_WaitForLastTask(); 

        //��󱸼Ĵ����е�DR1д���ʶ���ݣ���־�Ѿ���RTC���������á�
        BKP_WriteBackupRegister(BKP_DR1, 0x5051);
    }
    //���Բ���else�������Ƿ��Ѿ�����LSE��RTCԤ��Ƶֵ��ִ������RTC�жϡ�
    //��Ϊ�ж��������ݲ���BKP��Ҳ����RTC�������档

    RTC_WaitForSynchro();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();

    //RTC�ж����ȼ�����
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    return 0;
}

//RTC�жϷ�����
void RTC_IRQHandler(void)
{
    //���жϡ�
    if(RTC_GetITStatus(RTC_IT_SEC) == SET)
    {
       //��ȡʱ�ӡ�
       uint32_t u32Sec = RTC_GetCounter();
       Sec2Dt(u32Sec, &Calendar);       
    }

    //�����ж�
    //�����ж��е����ķ�������
//    if(RTC_GetITStatus(RTC_IT_ALR) == SET)
//    {

//    }
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);
    RTC_WaitForLastTask();
}

//�����ж�
//�������ܱ�4���� �� ���ܱ�100��������ݿ��Ա�400��������Ϊ���ꡣ
//���ܱ�100���������Ա�400����������Ϊ��������һ�����������400���ж����3��������Ҫ������
bool Is_LeapYear(uint16_t u16Year)
{
    if((u16Year % 4 == 0) && ((u16Year %100 != 0) || (u16Year % 400 == 0)))
        return TRUE;
    else
        return FALSE;
}

//����ʱ��ת��Ϊ����ʱ��
//ê����1970-1-1-0:0:0
//���ڷ�Χ��32λunsigned int ���ֵ��4294967295����ֵ��ʾ������Լ136�ꡣ
//��ʾΪ��ê��������ԼΪ1970�굽2106��ĳ������(����Ϊ�����в���)��
//�ɿ���ΧΪ1970��1��1�յ�2105��12��31�ա�
//����ʱ��ת����һ���Ǳ���ʱ�䡣
uint32_t Dt2Sec(dtStruct *dt)
{
    uint16_t iYear1, iMon1, iDay1;
    uint32_t u32Sec1;

    iYear1 = dt->u16Year - 1;  //���֮ǰ����������
    //�����������������������ζ����Ҫ��Ӽ��졣
    iDay1= ((iYear1/4 - iYear1/100 + iYear1/400) + iYear1*365);

    iMon1 = dt->u16Mon - 1;
    //�����·�֮ǰ������������
    iDay1 = iDay1 + (mon1_table[iMon1]);
    //�����������������2���Ժ��·�,������1.
    if(Is_LeapYear(dt->u16Year) && (iMon1 >= 2))
        iDay1++;

    //�������ϵ���������֮ǰ��������
    iDay1 += (dt->u16Day - 1);
    //��ȥ1970��ǰ������������
    iDay1 -= 719162;

    //����������
    u32Sec1 = ((((iDay1 *24 + dt->u16Hour) * 60) + dt->u16Min) * 60) + dt->u16Sec;
    return u32Sec1;
}

//����ʱ��(����)ת��Ϊ����ʱ��
//ê����1970-1-1-0:0:0
//����ʱ��ת����һ�ɿ�������ʱ�䡣
void Sec2Dt(uint32_t u32Sec, dtStruct* dt)
{
    uint16_t u16Day, u16Mon;
    uint32_t u32Sec_Day;  //����һ���������
    uint16_t u16Year = 1970;

    //������������
    u16Day =(uint16_t)(u32Sec / 86400);
    //���㲻��һ���������
    u32Sec_Day = u32Sec % 86400;

    //�������.
    while(u16Day >= 365)
    {
        //���������
        if(Is_LeapYear(u16Year))
        {
            if(u16Day >= 366)
                u16Day -= 366;
            else
                break;  //ʣ������Ϊ365�������꣬���������ϡ�
        }
        //��������
        else{
            u16Day -= 365;
        }
        u16Year++;
    }
    //��ݸ�ֵ
    dt->u16Year = u16Year;

    //�����·�.
    //u16DayΪһ���ڵ�����.
    u16Mon = 1;
    bool bIsLeap = Is_LeapYear(u16Year);
    while((u16Mon <= 11) && (u16Day >= ((bIsLeap && (u16Mon >= 2))? mon1_table[u16Mon]+1 : mon1_table[u16Mon])))
    {
        u16Mon++;
    }
    //Ҫôu16Mon��12,Ҫô��;�����ĳ���·ݡ�
    //�·ݸ�ֵ
    dt->u16Mon = u16Mon;

    //��������������
    //���㵱��֮ǰ������������
    //����·�С�ڵ���2 �� �����ꡣ
    if(dt->u16Mon <= 2 || !bIsLeap)
        u16Day -=  mon1_table[dt->u16Mon-1];
    //����·ݴ���2 �� �����ꡣ
    else
        u16Day -= (mon1_table[dt->u16Mon-1] +1);

    //���ϵ��졣
    u16Day++;
    //���ڸ�ֵ
    dt->u16Day = u16Day;

    //����ʱ��
    //u32Sec_Day�ǲ���һ���������
    dt->u16Hour = (int)(u32Sec_Day / 3600);
    dt->u16Min =  (int)((u32Sec_Day % 3600) / 60);
    dt->u16Sec =  (int)((u32Sec_Day % 3600) % 60);
}

//����RTCʱ��
//����Dt2Sec()������ʱ��ת��Ϊ����ʱ�䣬Ȼ��д��RTC��CNT��
//�����ڶ�dt���ݽ����жϣ���Ҫ1970-1-1��2105-12-31֮������ڡ�
//д����ȷ����TRUE�����򷵻�FALSE��
//�������ı�dt���ݡ�
bool Set_RTC(dtStruct *dt)
{
    uint16_t u16Day_Lim;
    //�ж�dt��ָ���ݵĺϷ��ԡ�
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
                return FALSE; //��������ڴ���
    if(!(dt->u16Hour >= 0 && dt->u16Hour <= 23) &&
          (dt->u16Min >= 0 && dt->u16Min <= 59) &&
          (dt->u16Sec >= 0 && dt->u16Sec <= 59))
                return FALSE; //�����ʱ�����

    //���������ʱ����ȷ��
    //ʹ��PWR��BKP���衣
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //ʹ�ܶ�RTC�ͺ󱸼Ĵ�����д���ʡ�
    PWR_BackupAccessCmd(ENABLE);

    //��������ʱ���Ӧ������ʱ��������д��RTC_CNT��
    RTC_SetCounter(Dt2Sec(dt));
    //�ȴ���RTC�Ĵ�����д������ɡ�
    RTC_WaitForLastTask();

    return TRUE;
}

//����RTC������ʱ�䡣
//�����÷�ͬSet_RTC()��ֻ����д��ļĴ�����ͬ��
bool Set_Alarm(dtStruct *dt)
{
    uint16_t u16Day_Lim;
    //�ж�dt��ָ���ݵĺϷ��ԡ�
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
        return FALSE; //��������ڴ���
    if(!(dt->u16Hour >= 0 && dt->u16Hour <= 23) &&
          (dt->u16Min >= 0 && dt->u16Min <= 59) &&
          (dt->u16Sec >= 0 && dt->u16Sec <= 59))
        return FALSE; //�����ʱ�����

    //���������ʱ����ȷ��
    //ʹ��PWR��BKP���衣
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    //ʹ�ܶ�RTC�ͺ󱸼Ĵ�����д���ʡ�
    PWR_BackupAccessCmd(ENABLE);

    //��������ʱ���Ӧ������ʱ��������д��RTC_CNT��
    RTC_SetAlarm(Dt2Sec(dt));
    //�ȴ���RTC�Ĵ�����д������ɡ�
    RTC_WaitForLastTask();

    return TRUE;
}
