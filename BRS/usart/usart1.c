/* ���ش���1
* USART1�ӿ�ΪPA9��PA10��
* ����TXD���Ϳ�ΪPA9,RXD���տ�ΪPA10��
* �������˿�Ϊ���ÿڡ�
*/

#include <stdio.h>
#include "usart1.h"

//����1���ջ�����:Ĭ�����Ϊ200���ֽ�.
u8 BUsart1_RxBuffer[USART_REC_LENMAX];
//����1����״̬��
u16 sUsart1_RxStatus = 0;

//fputc()��printf()�����õĺ�����ֻ���ض������ʹ��printf()������
//Ŀ��Ϊ����1:USART1.
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (u8)ch);
    //�ȴ��������
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
    return ch;
}

//����һ���ֽڵ�����.
void Usart_SendByte(USART_TypeDef * pUSARTx, u8 ch)
{
    USART_SendData(pUSARTx,ch);
    //�ȴ��������
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
}

//����һ��16λ��������.
void Usart_SendHalfWord(USART_TypeDef * pUSARTx, u16 ch)
{
    u8 temp_h, temp_l;
    temp_h = (ch & 0xFF00)>>8;
    temp_l = (ch & 0xFF);

    Usart_SendByte(pUSARTx, temp_h);
    Usart_SendByte(pUSARTx, temp_l);
}

//��������8λ�ֽ����ݵ����ڣ����Կ��������ַ�������NULL������
void Usart_SendString(USART_TypeDef * pUSARTx, u8 * str)
{
    unsigned int i = 0;
    do{
        Usart_SendByte(pUSARTx, *(str + i));
        i++;
    } while(*(str + i) != NULL);
    //�ȴ�ȫ���������ݷ�����Ϻ��˳�������
    while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) != SET);
}

//�������ⳤ�ȵ����ݵ����ڡ���������Ϊuint16_t��Χ��
void Usart_SendFrame(USART_TypeDef * pUSARTx, uint8_t * Ptr, uint16_t Len)
{
    
    for(uint16_t i = 0; i < Len; i++)
    {
        Usart_SendByte(pUSARTx, *(Ptr + i));
    }
    while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) != SET);
}

//����1��ʼ��
//8λ�ֳ���1λֹͣλ����У�顢��Ӳ�������ơ�
void Usart1_Init(u32 uiBound)
{
    GPIO_InitTypeDef GPIO_InitStruct;    // GPIO�˿ڳ�ʼ�������ṹ
    USART_InitTypeDef USART_InitStruct;  // ���ڳ�ʼ�������ṹ
    NVIC_InitTypeDef NVIC_InitStruct;    // NVIC��ʼ�������ṹ

    // ʱ��ʹ��.
    // ����1ʹ�õ�PA9,PA10��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    // ����1ʱ��ʹ��.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

    //�����Ǹ��ö˿ڣ��������ȸ����ֲ��ʼ���˿ڡ�
    // PA9��ʼ��:������1��TXD,���óɸ����������ģʽ��
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA10��ʼ��:������1��RXD�����óɸ�������ģʽ��
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //USART���ڳ�ʼ��
    USART_InitStruct.USART_BaudRate = uiBound;  // ������
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //Ӳ��������
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  //ģʽ�����ͺͽ���
    USART_InitStruct.USART_Parity = USART_Parity_No;  //��żУ��
    USART_InitStruct.USART_StopBits = USART_StopBits_1;  //ֹͣλ:1bit.
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;  //�ֳ�:8bit.
    USART_Init(USART1, &USART_InitStruct);

    // ʹ�ܴ���1
    USART_Cmd(USART1, ENABLE);

    // ��������1�����ж�
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // �ж�����������NVIC��ʼ��
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

// ����1�����жϷ�������
void USART1_IRQHandler(void)
{
    u8 BRes; //���ν��յ������ݣ�1byte.
    // ����ǽ����¼���
    if(USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        // ��ȡ���ν��յ������ݵ�BRes��1byte.
        BRes = USART_ReceiveData(USART1);
        // �����֡����δ���:״̬��bit15Ϊ0.
        if((sUsart1_RxStatus & 0x8000) == 0)
        {
            //����ϴ��Ѿ����յ�������Ϊ0x0d�س���:״̬��bit14Ϊ1��
            if(sUsart1_RxStatus & 0x4000)
            {
                //����������ݲ��ǻ��з�0x0a����������֡��Ч��״̬�ָ�λ���㡣��
                if(BRes != 0x0a)
                    sUsart1_RxStatus = 0;
                //������������ǻ��з�0x0a����������֡��Ч��������ɣ�״̬��bit15��1��
                else
                    sUsart1_RxStatus |= 0x8000;
            }
            // ����ϴλ�û�н��յ��س���0x0d��
            else
            {
                //������������ǻس���0x0d
                if(BRes == 0x0d)
                    sUsart1_RxStatus |= 0x4000;
                //���������Ч����֡���ݡ�
                else
                {
                    //������յ������ݵ���������
                    BUsart1_RxBuffer[sUsart1_RxStatus & 0x3FFF] = BRes;
                    sUsart1_RxStatus++;
                    //�ж��Ƿ񳬹�����������ƣ�������״̬�����㡣
                    if(sUsart1_RxStatus > (USART_REC_LENMAX - 1))
                        sUsart1_RxStatus = 0;
                    //�ѽ��յ��������ط���ȥ��
                    //������ڽ����ж��п��Է��͡�
                    //USART_SendData(USART1, BRes);
                    //while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
                }
            }
        }
    }
}




