#include <stm32f10x.h>
#include <core_cm3.h>
#include "delay.h"

// ��1ms����SysTick.
void delay_Init(void)
{
    // SysTick���á�
    if (SysTick_Config(RELOAD_1MS))
    {
        while(1);
    }
}

// SysTick��ʱ���ж�
//ÿ1ms�ж�һ�Σ�ÿ��ʹbPlus_ms��תһ�Σ�ʹuTimer_ms��1��
//����������������Ϊ2ms��ÿ1ms��תһ�Ρ�
void SysTick_Handler(void)
{
    Timer.bPlus_ms = !Timer.bPlus_ms;
    if(Timer.uTimer_ms != 0U)
        Timer.uTimer_ms--;
}


// ms��delay.
void delay_ms(u32 utime_ms)
{

    Timer.uTimer_ms = utime_ms;
    // ��ѯ��ʱ�Ƿ񵽣������������ѯ��
    while( Timer.uTimer_ms != 0U )
        ;
}

// s��delay.
void delay_s(u32 utime_s)
{
    // ��ʱʱ��Ϊutime_s����1000ms��
    for (int i = 0; i < utime_s; ++i)
    {
        delay_ms(1000U);
    }
}

// ��ʱ��ͨ��ʱ��
// ��bEnbΪTRUEʱ��ʼ��ʱ����ʱ��λΪ1ms��
// ��bEnbΪFALSEʱ��λ��ʱ����
// ����ʱ��������û�и�λ��ʱ����ʱ����ǰ����ֵuEt���ֲ��䡣
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
        //ÿ�μ�⵽����(ÿ1ms��תһ��)�ͼ�1.
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

//ʱ���ۼ���
//��bEnbΪTRUEʱ��ʱ��ʼ��ʱ�䵥λΪms��
//��bEnbΪFALSEʱ��λ��ʱ����
//ʱ���ۼ�ֵ���Ϊ32Ϊ�޷���������Ϊ4294967295ms��Լ47�졣
//���ۼ�ʱ��Խ��ʱ�Զ���λΪ0.
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
        //timer->bQΪTRUE��ζ���ۼ��ڽ��С�
        timer->bQ = TRUE;
    }

    return timer->uEt;
}

