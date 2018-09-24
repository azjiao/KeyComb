/*
*���ܣ�������ʱ��TIM6/TIM7����
*������������ʱ��ֻ��������TIM6��TIM7.����ģʽֻ�����ϼ���һ��ģʽ���������ü���ģʽ��
*��ƣ�azjiao
*�汾��0.1
*���ڣ�2018-07-18
*
*/

#include "baseTime.h"
#include "led.h"

// ������ʱ��TIM6��ʼ����
// ��ʼ��TIM6��ʹ�ܸ����жϡ�
// u16ARR���Զ���װ��ֵ.
// u16PSC�Ƕ�ʱ��Ԥ��Ƶֵ��
void Time6_Init(u16 u16ARR, u16 u16PSC)
{
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //��ʱ����ʼ�������ṹ��
    NVIC_InitTypeDef         NVIC_InitStruct;  // �ж�������ʼ�������ṹ��

    //TIM6ʱ��ʹ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    //TIM6��ʼ��
    TIM_InitStrut.TIM_Period = u16ARR; // �Զ�װ�ؼĴ���ֵ.
    TIM_InitStrut.TIM_Prescaler = u16PSC; //��ʱ��ʱ��Ԥ��Ƶֵ����Ϊ������ʱ���á�
    // TIM6�ǻ�����ʱ����ֻ��ʹ�����ϼ���ģʽ���������á�
    TIM_TimeBaseInit(TIM6, &TIM_InitStrut);

    //ʹ��TIM6�жϣ������жϸ��¡�
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update); //����жϸ��±�ʶ.

    //����TIM6�жϵ����ȼ���
    NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //ʹ��TIM6��
    //TIM_Cmd(TIM6, ENABLE);
}

// ������ʱ��TIM7��ʼ����
// ��ʼ��TIM7��ʹ�ܸ����жϡ�
// u16ARR���Զ���װ��ֵ.
// u16PSC�Ƕ�ʱ��Ԥ��Ƶֵ��
void Time7_Init(u16 u16ARR, u16 u16PSC)
{
    TIM_TimeBaseInitTypeDef  TIM_InitStrut; //��ʱ����ʼ�������ṹ��
    NVIC_InitTypeDef         NVIC_InitStruct;  // �ж�������ʼ�������ṹ��

    //TIM7ʱ��ʹ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    //TIM7��ʼ��
    TIM_InitStrut.TIM_Period = u16ARR; // �Զ�װ�ؼĴ���ֵ.
    TIM_InitStrut.TIM_Prescaler = u16PSC; //��ʱ��ʱ��Ԥ��Ƶֵ����Ϊ������ʱ���á�
    // TIM6�ǻ�����ʱ����ֻ��ʹ�����ϼ���ģʽ���������á�
    TIM_TimeBaseInit(TIM7, &TIM_InitStrut);

    //ʹ��TIM7�жϣ������жϸ��¡�
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //����жϸ��±�ʶ.

    //����TIM7�жϵ����ȼ�������ΪTIM7����Ӧ���ȼ���TIM6�͡�
    NVIC_InitStruct.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //ʹ��TIM7�����ⲿʹ��������ʱ��
    //TIM_Cmd(TIM7, ENABLE);
}

// TIM6��ʼ��Ϊ����us���̶���ʱ������ʱ��CK_CLKΪ100kHz,��������һ������ʱ��Ϊ0.01ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ719������ʹTIM6����ʱ��Ϊ100kHz.
// u16PtusΪ���趨ʱ�жϵ�usֵ��
// u16Ptus����Ϊ10us�ı�����
void Time6_Init2us(u16 u16Ptus)
{
    u16 u16ARR;
    // ��u16Ptmsת��Ϊ�Զ���װ�ؼĴ��������ֵ��
    u16ARR = u16Ptus / 10 - 1;
    Time6_Init(u16ARR, (u16)719);
}

// TIM7��ʼ��Ϊ����us���̶���ʱ������ʱ��CK_CLKΪ100kHz,��������һ������ʱ��Ϊ0.01ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ719������ʹTIM7����ʱ��Ϊ100kHz.
// u16PtusΪ���趨ʱ�жϵ�usֵ��
void Time7_Init2us(u16 u16Ptus)
{
    u16 u16ARR;
    // ��u16Ptmsת��Ϊ�Զ���װ�ؼĴ��������ֵ��
    u16ARR = u16Ptus / 10 - 1;
    Time7_Init(u16ARR, (u16)719);
}

// TIM7��ʼ��Ϊ����ms���̶���ʱ������ʱ��CK_CLKΪ10kHz,��������һ������ʱ��Ϊ0.1ms.
// ϵͳ��ʼ��ʱAPB1��ƵֵΪ2��Ϊ36MHz,��TIM6ClkΪ72MHz.
// ����PSCΪ7199������ʹTIM7����ʱ��Ϊ10kHz.
// u16PtusΪ���趨ʱ�жϵ�msֵ��
void Time7_Init2ms(u16 u16Ptms)
{
    u16 u16ARR;
    // ��u16Ptmsת��Ϊ�Զ���װ�ؼĴ��������ֵ��
    u16ARR = u16Ptms * 10 - 1;
    Time7_Init(u16ARR, (u16)7199);
}

/*
//��ʱ��TIM6�жϷ�����
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)  //�Ƿ����жϸ��¡�
    {
        LED0 = !LED0; //LED0ȡ��
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update); //����жϸ��±�ʶ.
    }
}

//��ʱ��TIM7�жϷ�����
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)  //�Ƿ����жϸ��¡�
    {
        LED1 = !LED1; //LED1ȡ��
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update); //����жϸ��±�ʶ.
    }
}
*/
