#ifndef __USART1_H
#define __USART1_H
#include <stm32f10x.h>

#define USART_REC_LENMAX  200  //һ�ν��յ�����ֽ�����

// �����ⲿ�������ñ�����usart1.c�ж���.
extern u8 BUsart1_RxBuffer[USART_REC_LENMAX];  // ���ջ�����
extern u16 sUsart1_RxStatus; // ����״̬��

//����1��ʼ��
void Usart1_Init(u32 uiBound);//uiBound�ǲ�����Hz.
//����һ���ֽ����ݵ�����1
void Usart_SendByte(USART_TypeDef * pUSARTx, u8 ch);
//���Ͱ���ֵ�����1����16λ���ݡ�
void Usart_SendHalfWord(USART_TypeDef * pUSARTx, u16 ch);
//����һ���ַ��������ڣ����ַ�����NULL������
void Usart_SendString(USART_TypeDef * pUSARTx, u8 * str);
//�������ⳤ�ȵ����ݵ����ڡ�
void Usart_SendFrame(USART_TypeDef * pUSARTx, uint8_t * Ptr, uint16_t Len);

#endif /* end of include guard: __USART1_H */
