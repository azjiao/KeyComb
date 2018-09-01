//�ⲿ�ж���Դ����

#include "extix.h"
#include "key.h"
#include "led.h"

// ���ذ����ⲿ�жϳ�ʼ����
// KEY0/KEY1/WKUP�����������жϳ�ʼ����
// ��������1.�����ⲿ�ж��ߡ�2.�����ж��ߵ�NVIC���ȼ���ʹ���жϡ�
void keyExti_Init(void)
{

    EXTI_InitTypeDef EXTI_InitStruct;  // �ⲿ�жϳ�ʼ�������ṹ��
    NVIC_InitTypeDef NVIC_InitStruct;  // �ж�������������ʼ�������ṹ��

    // ���ù���ʱ��ʹ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);


    // KEY0 �ж����ã�
    // KEY0:�ӵ�PE4�˿ڣ��������룻
    // ����PE4Ϊ��4���ж�Դ��
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);
    //�ж���4���½��ش�����
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // KEY1 �ж����ã�
    // KEY1: �ӵ�PE3�˿ڣ��������룻
    // ����PE3Ϊ��3���ж�Դ��
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    // �ж���3���½��ش�����
    EXTI_InitStruct.EXTI_Line = EXTI_Line3;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    // WKUP �ж����ã�
    // WKUP: �ӵ�PA0�˿ڣ��������룻
    // ����PA0Ϊ��0���ж�Դ��
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    // �ж���0�������ش�����
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;    
    EXTI_Init(&EXTI_InitStruct);

    // ����NVIC�ж����ȼ���ʹ���жϡ�
    // KEY0���ڵ��ⲿ�ж�ͨ��ΪEXTI4_IRQn��
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    // ��ռ���ȼ�Ϊ2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // ��Ӧ���ȼ�Ϊ0.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    // ʹ���ⲿ�ж�ͨ����
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // ���ݳ�ʼ�������ṹ��Ϣ��ʼ��NVIC.
    NVIC_Init(&NVIC_InitStruct);


    // KEY1���ڵ��ⲿ�ж�ͨ��ΪEXTI3_IRQn��
    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    // ��ռ���ȼ�Ϊ2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // ��Ӧ���ȼ�Ϊ1.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
    // ʹ���ⲿ�ж�ͨ����
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // ���ݳ�ʼ�������ṹ��Ϣ��ʼ��NVIC.
    NVIC_Init(&NVIC_InitStruct);

    // WKUP���ڵ��ⲿ�ж�ͨ��ΪEXTI0_IRQn��
    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    // ��ռ���ȼ�Ϊ2.
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
    // ��Ӧ���ȼ�Ϊ3.
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x03;
    // ʹ���ⲿ�ж�ͨ����
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    // ���ݳ�ʼ�������ṹ��Ϣ��ʼ��NVIC.
    NVIC_Init(&NVIC_InitStruct);
    
}


// �ⲿ�жϷ�������
// KEY0��Ӧ��EXTI4��������
void EXTI4_IRQHandler(void)    
{
    if(KEY0_CODE)
    {
        LED0_ON;
    }
    EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
}

// KEY1��Ӧ��EXTI3��������
void EXTI3_IRQHandler(void)
{
    if(KEY1_CODE)
    {
        LED1_ON;
    }
    EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE3�ϵ��жϱ�־λ  
}

// WKUP��Ӧ��EXTI0��������
void EXTI0_IRQHandler(void)
{
    if(WKUP_CODE)
    {
        LED0_OFF;
        LED1_OFF;
    }
    EXTI_ClearITPendingBit(EXTI_Line0);  //���LINE0�ϵ��жϱ�־λ  
}

//
