// ������ʱ��ֻ��������TIM6��TIM7.

#ifndef __BASETIME_H
#define __BASETIME_H
#include "stm32f10x.h"

// ������ʱ��TIM6��ʼ����
// ��ʼ��TIM6��ʹ�ܸ����жϡ�
// u16ARR���Զ���װ��ֵ.
// u16PSC�Ƕ�ʱ��Ԥ��Ƶֵ��
void Time6_Init(u16 u16ARR, u16 u16PSC);

// ������ʱ��TIM6��ʼ����
// ��ʼ��TIM6��ʹ�ܸ����жϡ�
// u16ARR���Զ���װ��ֵ.
// u16PSC�Ƕ�ʱ��Ԥ��Ƶֵ��
void Time7_Init(u16 u16ARR, u16 u16PSC);

// TIM6��ʼ��Ϊ����us���̶���ʱ������ʱ��CK_CLKΪ100kHz,��������һ������ʱ��Ϊ0.01ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ719������ʹTIM6����ʱ��Ϊ100kHz.
// u16PtusΪ���趨ʱ�жϵ�usֵ��
// u16Ptus����Ϊ10us�ı�����
void Time6_Init2us(u16 u16Ptms);

// TIM7��ʼ��Ϊ����us���̶���ʱ������ʱ��CK_CLKΪ100kHz,��������һ������ʱ��Ϊ0.01ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ719������ʹTIM7����ʱ��Ϊ100kHz.
// u16PtusΪ���趨ʱ�жϵ�usֵ��
void Time7_Init2us(u16 u16Ptms);

// TIM7��ʼ��Ϊ����ms���̶���ʱ������ʱ��CK_CLKΪ10kHz,��������һ������ʱ��Ϊ0.1ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ7190������ʹTIM7����ʱ��Ϊ10kHz.
// u16PtusΪ���趨ʱ�жϵ�msֵ��
void Time7_Init2ms(u16 u16Ptms);

#endif /* end of include guard: __BASETIME_H */

