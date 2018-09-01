/* 板载串口1
* USART1接口为PA9和PA10。
* 其中TXD发送口为PA9,RXD接收口为PA10。
* 这两个端口为复用口。
*/

#include <stdio.h>
#include "usart1.h"

//串口1接收缓冲区:默认最大为200个字节.
u8 BUsart1_RxBuffer[USART_REC_LENMAX];
//串口1接收状态字
u16 sUsart1_RxStatus = 0;

//fputc()是printf()所调用的函数，只有重定义才能使用printf()函数。
//目标为串口1:USART1.
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (u8)ch);
    //等待发送完毕
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
    return ch;
}

//发送一个字节到串口.
void Usart_SendByte(USART_TypeDef * pUSARTx, u8 ch)
{
    USART_SendData(pUSARTx,ch);
    //等待发送完毕
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
}

//发送一个16位数到串口.
void Usart_SendHalfWord(USART_TypeDef * pUSARTx, u16 ch)
{
    u8 temp_h, temp_l;
    temp_h = (ch & 0xFF00)>>8;
    temp_l = (ch & 0xFF);

    Usart_SendByte(pUSARTx, temp_h);
    Usart_SendByte(pUSARTx, temp_l);
}

//发送连续8位字节数据到串口：可以看作发送字符串，以NULL结束。
void Usart_SendString(USART_TypeDef * pUSARTx, u8 * str)
{
    unsigned int i = 0;
    do{
        Usart_SendByte(pUSARTx, *(str + i));
        i++;
    } while(*(str + i) != NULL);
    //等待全部连续数据发送完毕后退出函数。
    while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) != SET);
}

//发送任意长度的数据到串口。长度限制为uint16_t范围。
void Usart_SendFrame(USART_TypeDef * pUSARTx, uint8_t * Ptr, uint16_t Len)
{
    
    for(uint16_t i = 0; i < Len; i++)
    {
        Usart_SendByte(pUSARTx, *(Ptr + i));
    }
    while(USART_GetFlagStatus(pUSARTx, USART_FLAG_TC) != SET);
}

//串口1初始化
//8位字长、1位停止位、无校验、无硬件流控制。
void Usart1_Init(u32 uiBound)
{
    GPIO_InitTypeDef GPIO_InitStruct;    // GPIO端口初始化参数结构
    USART_InitTypeDef USART_InitStruct;  // 串口初始化参数结构
    NVIC_InitTypeDef NVIC_InitStruct;    // NVIC初始化参数结构

    // 时钟使能.
    // 串口1使用到PA9,PA10。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    // 串口1时钟使能.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

    //串口是复用端口，所以首先根据手册初始化端口。
    // PA9初始化:即串口1的TXD,配置成复用推挽输出模式。
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA10初始化:即串口1的RXD，配置成浮空输入模式。
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //USART串口初始化
    USART_InitStruct.USART_BaudRate = uiBound;  // 波特率
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //硬件流控制
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  //模式：发送和接收
    USART_InitStruct.USART_Parity = USART_Parity_No;  //奇偶校验
    USART_InitStruct.USART_StopBits = USART_StopBits_1;  //停止位:1bit.
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;  //字长:8bit.
    USART_Init(USART1, &USART_InitStruct);

    // 使能串口1
    USART_Cmd(USART1, ENABLE);

    // 开启串口1接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 中断向量控制器NVIC初始化
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStruct);
}

// 串口1接收中断服务函数。
void USART1_IRQHandler(void)
{
    u8 BRes; //本次接收到的数据：1byte.
    // 如果是接收事件。
    if(USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        // 读取本次接收到的数据到BRes：1byte.
        BRes = USART_ReceiveData(USART1);
        // 如果本帧接收未完成:状态字bit15为0.
        if((sUsart1_RxStatus & 0x8000) == 0)
        {
            //如果上次已经接收到的数据为0x0d回车符:状态字bit14为1。
            if(sUsart1_RxStatus & 0x4000)
            {
                //如果本次数据不是换行符0x0a，表明数据帧无效，状态字复位清零。。
                if(BRes != 0x0a)
                    sUsart1_RxStatus = 0;
                //如果本次数据是换行符0x0a，表明数据帧有效，接收完成，状态字bit15置1。
                else
                    sUsart1_RxStatus |= 0x8000;
            }
            // 如果上次还没有接收到回车符0x0d。
            else
            {
                //如果本次数据是回车符0x0d
                if(BRes == 0x0d)
                    sUsart1_RxStatus |= 0x4000;
                //否则就是有效数据帧数据。
                else
                {
                    //储存接收到的数据到缓冲区。
                    BUsart1_RxBuffer[sUsart1_RxStatus & 0x3FFF] = BRes;
                    sUsart1_RxStatus++;
                    //判断是否超过最大数据限制，超过则状态字清零。
                    if(sUsart1_RxStatus > (USART_REC_LENMAX - 1))
                        sUsart1_RxStatus = 0;
                    //把接收到的数据重发出去。
                    //结果：在接收中断中可以发送。
                    //USART_SendData(USART1, BRes);
                    //while(USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
                }
            }
        }
    }
}




