/*
 * ����SysTick��ʱ����
 * ʱ��Դ������FCLK��ΪAHB��72MHz.
 * ��ʱ��λΪ1ms,װ��ֵΪ 72000000/1000=72000UL
 * ����1us��ʱ��
 * ��ʱ�ж��н�����1ms����ֵ��Ϊstatic unsigned int ���ͣ���� �ṹ�塣
 * ����ʹ������Ϊ��ͣ��ʱ�����ò�ѯ�ȴ���ʽ����Ҳ����ʹ������Ϊ����ͣ�Ķ�ʱ��ʹ�á�
 */

#ifndef __DELAY_H
#define __DELAY_H
#include "template.h"

// 1msװ��ֵ��
// ����ʱ��SystemCoreClockΪ72MHz,��ϵͳԤ��AHB����SystemCoreClock,SysTickʱ��Դ����ΪAHB.
// 72000000/1000=72000.
#define RELOAD_1MS   SystemCoreClock/1000U
// 500usװ��ֵ��
// 500usװ��ֵ��1msװ��ֵ��һ�롣
// 72000000/2000=36000.
#define RELOAD_500US    SystemCoreClock/2000U

// ��ʱ��,���SysTickʹ�á�
static struct {
    u32 uTimer_ms;  // ms��ʱ��,���ڲ�ѯ��ʱ��
    bool bPlus_ms;  // ms�����ź�,0.5msON,0.5msOFF.
}Timer;

// �����Ͷ�ʱ���ṹ
typedef struct {
    u32  uEt; // ��ʱ������ǰֵ��
    bool bTemp; //�����ݴ��źš�
    bool bQ;    //��ʱʱ�䵽��ʶ��
}TimerType;


// ��ʱ����ʼ����
void delay_Init(void);

// ms��delay��
void delay_ms(u32 utime_ms);
// s��delay.
void delay_s(u32 utime_s);

// ��ʱ��ͨ��ʱ��
// ��bEnbΪTRUEʱ��ʼ��ʱ����ʱ��λΪ1ms��
// ��bEnbΪFALSEʱ��λ��ʱ����
// ����ʱ��������û�и�λ��ʱ����ʱ����ǰ����ֵuEt���ֲ��䡣
bool TimeON(bool bEnb, u32 uPt, TimerType *timer);

//ʱ���ۼ���
//��bEnbΪTRUEʱ��ʱ��ʼ��ʱ�䵥λΪms��
//��bEnbΪFALSEʱ��λ��ʱ����
//ʱ���ۼ�ֵ���Ϊ32Ϊ�޷���������Ϊ4294967295ms��Լ47�졣
//���ۼ�ʱ��Խ��ʱ�Զ���λΪ0.
uint16_t TimeACC(bool bEnb, TimerType *timer);

#endif /* end of include guard: __DELAY_H */
