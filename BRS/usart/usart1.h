#ifndef __USART1_H
#define __USART1_H
#include <stm32f10x.h>

#define USART_REC_LENMAX  200  //一次接收的最大字节数。

// 声明外部变量，该变量在usart1.c中定义.
extern u8 BUsart1_RxBuffer[USART_REC_LENMAX];  // 接收缓冲区
extern u16 sUsart1_RxStatus; // 接收状态字

//串口1初始化
void Usart1_Init(u32 uiBound);//uiBound是波特率Hz.
//发送一个字节数据到串口1
void Usart_SendByte(USART_TypeDef * pUSARTx, u8 ch);
//发送半个字到串口1，即16位数据。
void Usart_SendHalfWord(USART_TypeDef * pUSARTx, u16 ch);
//发送一个字符串到串口，该字符串以NULL结束。
void Usart_SendString(USART_TypeDef * pUSARTx, u8 * str);
//发送任意长度的数据到串口。
void Usart_SendFrame(USART_TypeDef * pUSARTx, uint8_t * Ptr, uint16_t Len);

#endif /* end of include guard: __USART1_H */
