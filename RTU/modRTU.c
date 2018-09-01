#include <stdio.h>
#include "dev.h"
#include "modRTU.h"
#include "modMaster.h"

//接收数据缓冲区结构.
ModRTU_RX_Struct RX_Struct;
//发送数据缓冲区结构.
ModRTU_TX_Struct TX_Struct;
//共用缓冲区
ModRTU_Buffer Data_Struct;

//通讯控制结构.
ModRTU_Status_Struct Modbus_Status_Struct;
//RS485端口初始化数据。
struct RS485Init_Struct RS485_InitStruct;

//RS485端口初始化.
//精英版RS485是USART2，复用到PA2和PA3.
//初始化的数据来自于RS485InitStruct。
void RS485_Init(struct RS485Init_Struct RS485InitStruct)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    USART_InitTypeDef  USART_InitStruct;
    NVIC_InitTypeDef  NVIC_InitStruct;

    //时钟使能:PA口和PD口
    //PA口是USART2口需要，而PD7口是RS485外围芯片收发选择需要。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);
    //USART2时钟使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    //PD7配置，用于控制RS485电平转换芯片收发使能。
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出。
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    //PA2和PA3复用配置
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;  //PA2是TX发送口，配置为复用推挽输出。
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    //GPIO_SetBits(GPIOA, GPIO_Pin_2); //把TX初始化为高电平
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PA3是RX接受口，配置为浮空输入。
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//复位串口2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//停止复位

    //USART2设置
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_BaudRate = RS485InitStruct.u32BoundRate;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1; //应该固定为1bit停止为，为了通用可以设置。
    switch(RS485InitStruct.u16Parity)
    {
        case 0:USART_InitStruct.USART_Parity = USART_Parity_No;break; //无校验
        case 1:USART_InitStruct.USART_Parity = USART_Parity_Odd;break; //奇校验
        case 2:USART_InitStruct.USART_Parity = USART_Parity_Even;break; //偶校验
    }
    USART_Init(USART2, &USART_InitStruct);


    //配置中断优先级
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //使能串口2接收中断
    USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);

    //使能串口2
    USART_Cmd(USART2, ENABLE);

    //t3.5和t1.5所用定时器初始化但暂不使能，用于监控.
    //TResponse定时器用于应答超时监测。
    T15_35_Init();
    TResponse_Init();

     //默认为接收模式
     RS485_RXENB;
}

//通讯控制用定时器初始化，但暂不使能。
//t1.5和t3.5共用TIM6,且计数时钟一样都是100kHz，0.01ms=10us.
//系统只使用了t3.5。
void T15_35_Init(void)
{
    //默认初始化为t3.5.
    Time6_Init2us(RS485_InitStruct.u16T3_5_us);
    TIM_Cmd(T3_5, DISABLE);
}

//应答超时定时器初始化。
//应答超时定时器计数时钟为10kHz,0.1ms.
void TResponse_Init(void)
{
    Time7_Init2ms(TRESPONSE_ms);
    TIM_Cmd(TRESPONSE, DISABLE);
}

//Modbus初始化
void Modbus_Init(void)
{
    float fOneByteTime;  //t3.5定时时间us.
    //RS485端口初始化:USART2及PD7口,t3.5和t1.5定时器初始化.
    RS485_InitStruct.u16DataBit = USART_WordLength_8b;  //8位数据位
    RS485_InitStruct.u16Parity = 0;                     //校验位无
    RS485_InitStruct.u16StopBit = USART_StopBits_1;     //停止位1位
    RS485_InitStruct.u32BoundRate = 9600;              //波特率

    //根据波特率计算t1.5和t3.5的定时时间。
    if(RS485_InitStruct.u32BoundRate <= 19200)
        fOneByteTime = (11/(float)RS485_InitStruct.u32BoundRate) * 1000000;  //一个字节发送的时间(us)。
    else
        fOneByteTime = 1750; //只有波特率大于19200时才使用固定定时。
    RS485_InitStruct.u16T1_5_us = 1.5 * fOneByteTime;
    RS485_InitStruct.u16T3_5_us = 3.5 * fOneByteTime;
    RS485_Init(RS485_InitStruct);


    //通讯缓冲区初始化.
    //接收缓冲区清零:非必要.
    for(int i = 0; i < FRAME_MAXLEN; i++)
    {
        RX_Struct.Buffer[i] = 0;
    }
    // 接收缓冲区当前索引清零.
    RX_Struct.u16Index = 0;

    //发送缓冲区清零:非必要.
    for(int i = 0; i < FRAME_MAXLEN; i++)
    {
        TX_Struct.Buffer[i] = 0;
    }
    // 接收缓冲区当前索引清零.
    TX_Struct.u16Index = 0;

    //通讯状态结构初始化
    Modbus_Status_Struct.bBusy = FALSE; //空闲
    Modbus_Status_Struct.bErr = FALSE; //无错
    Modbus_Status_Struct.u16CommErr = 0; //无错
    Modbus_Status_Struct.bFrame_ReadEnb = FALSE; //帧不可读
    Modbus_Status_Struct.bDone = FALSE;  //数据不可读
    Modbus_Status_Struct.bResponse_TimeOut = FALSE; //不超时
    Modbus_Status_Struct.iErrCount = 0;  //出错累积值清零

}


//串口2接收中断服务函数
//当串口接收到一个字节时发生该中断.
void USART2_IRQHandler(void)
{
    u8 u8Data;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        //接收字节数据并自动清除中断标识.
        u8Data = USART_ReceiveData(USART2);
        Modbus_Status_Struct.bBusy = TRUE;  //忙
        Modbus_Status_Struct.bFrame_ReadEnb = FALSE; //帧不可读取。

        //如果出现接收错误,则设置错误信息.
        if(USART_GetFlagStatus(USART2, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
        {
           Modbus_Status_Struct.u16CommErr = 3;  //通讯接收错误
           Modbus_Status_Struct.bErr = TRUE;
        }
        else
        {
            Modbus_Status_Struct.u16CommErr = 0;
            Modbus_Status_Struct.bErr = FALSE;
        }

        //如果字节接收无错且接收缓冲区没有越限则接收转储数据,并重新启动t3.5定时器.
        //否则，则不储存数据到接收缓冲区，将导致CRC校验错误，也会让最终结果错误。
        if((Modbus_Status_Struct.u16CommErr == 0) && (RX_Struct.u16Index < FRAME_MAXLEN -1))
        {
            //转储接收到的字节数据.
            RX_Struct.Buffer[RX_Struct.u16Index] = u8Data;
            RX_Struct.u16Index++;

            //复位并重新启动t3.5定时器,监测数据帧是否结束.
            T3_5_Restart();
        }

    }
}

//复位并重启T3.5开始定时.
void T3_5_Restart(void)
{
    TIM_ClearITPendingBit(T3_5, TIM_IT_Update); //清除定时器中断更新标识.
    TIM_SetCounter(T3_5, 0); //复位向上计数器当前值为0.
    TIM_SetAutoreload(T3_5, RS485_InitStruct.u16T3_5_us/10 - 1);
    TIM_Cmd(T3_5, ENABLE); //使能定时器开始定时.
}

//t3.5定时器中断服务函数。
//t1.5用于监测字节接收是否超时,这个版本取消字节连续性监测。
//t3.5用于监测帧是否结束.
//但t3.5即用于接收监测帧结束也用于发送帧间延时.
//如果发生t3.5中断则表示接收帧结束或帧发送结束,转入空闲状态.
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //复位应答超时定时器并停止工作。
        TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update);
        TIM_SetCounter(TRESPONSE, 0);
        TIM_Cmd(TRESPONSE, DISABLE);
        Modbus_Status_Struct.bResponse_TimeOut = FALSE;

        //复位t3.5定时器并失能,停止定时监测.
        TIM_ClearITPendingBit(T3_5, TIM_IT_Update);
        TIM_SetCounter(T3_5, 0); //复位向上计数器当前值为0.
        TIM_Cmd(T3_5, DISABLE);  //失能定时器.

        //当处于接收时校验CRC16.
        if(Modbus_Status_Struct.bTxRx_Mode)
        {
            //判断数据帧的有效性.
            //只对通讯数据帧本身作判断，也即CRC校验。
            if(CRC16Check(RX_Struct))
            {
                Modbus_Status_Struct.bErr = FALSE;   //数据帧可读时bErr为FALSE，所以可以取消对bErr的判断。
                //设置系统状态进入空闲.
                Modbus_Status_Struct.bBusy = FALSE;
                Modbus_Status_Struct.bFrame_ReadEnb = TRUE;  //帧可读取。
                //如果帧可读，等待处理帧时不再接收数据，把模式改为发送,以免接收到的数据被添加到帧末造成问题。
                RS485_TXENB; //发送使能
            }
            //如果CRC失败，则丢弃本帧，重新接收。
            else
            {
                Modbus_Status_Struct.iErrCount++;//通讯错误次数统计。
                Modbus_Status_Struct.bErr = TRUE;
                Modbus_Status_Struct.u16CommErr = 2; //CRC校验失败。
                //丢弃本次数据帧，并重新启动接收。
                ReceiveFrame(&RX_Struct);
            }

        }
        //如果处于发送则不作校验。
        else
            //设置系统状态进入空闲.
            Modbus_Status_Struct.bBusy = FALSE;
    }
}

//接收应答超时检测启动
//启动TRESPONSE定时器：TIM7.
void RespTimeOut_Enb(void)
{
    Modbus_Status_Struct.bResponse_TimeOut = FALSE; //复位超时标识。
    TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update); //清除定时器中断更新标识.
    TIM_SetCounter(TRESPONSE, 0); //复位向上计数器当前值为0.
    TIM_Cmd(TRESPONSE, ENABLE);   //启动超时定时器。
}

//应答超时定时器中断服务函数
//发生时：接收超时。
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        Modbus_Status_Struct.bResponse_TimeOut = TRUE;
        //复位应答超时定时其并停止工作。
        TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update);
        TIM_SetCounter(TRESPONSE, 0);
        TIM_Cmd(TRESPONSE, DISABLE);
    }
}

//CRC16校验码生成
//Data指向待校验数据的指针，Len是数据字节长度.
//返回原生16位CRC，高字节在前。
u16 CRC16Gen(u8* pByteData, u16 u16Len)
{
    u16 CRC16 = 0xFFFF;  //CRC预置值
    for(u16 i = 0; i < u16Len; i++)  //依次处理所有数据。
    {
        CRC16 ^= *(pByteData + i);   //异或数据

        for(u8 bit = 0; bit < 8; bit++)
        {
            //如果待右移出位为1，则右移后异或多项式。
            if(CRC16 & 0x0001)
            {
                CRC16 >>= 1;
                CRC16 ^= 0xA001;  //校验多项式，CRC16_Modbus标准。
            }
            //否则只作右移一位。
            else{
                CRC16 >>= 1;
            }
        }
    }
    return (CRC16);
}

//CRC16校验
bool CRC16Check(ModRTU_RX_Struct RX_Struct)
{
    /*
    u16 CRC16, CRC16_RX = 0x0;
    //取得RX_Struct数据的校验码。
    CRC16_RX |= RX_Struct.Buffer[RX_Struct.u16Index-1]; //高字节。
    CRC16_RX <<= 8;
    CRC16_RX |= RX_Struct.Buffer[RX_Struct.u16Index-2]; //低字节。

    CRC16 = CRC16Gen(RX_Struct.Buffer, RX_Struct.u16Index-2);  //生成CRC16,高字节在前。
    if(CRC16 == CRC16_RX)
        return TRUE;
    else
        return FALSE; */


    //使用重新把含CRC的数据再校验，如果生成的CRC为0x00则正确，否则不正确。
    if(CRC16Gen(RX_Struct.Buffer, RX_Struct.u16Index) == (uint16_t)0)
        return TRUE;
    else
        return FALSE;
}



//发送数据帧
void SendFrame(ModRTU_TX_Struct* pTX_Struct)
{
    RS485_TXENB; //发送使能
    //发送状态,以便在发送后不校验CRC.
    Modbus_Status_Struct.bTxRx_Mode = FALSE;

    for(u8 i = 0; i < pTX_Struct->u16Index; i++)
    {
        Modbus_Status_Struct.bBusy = TRUE;  //忙
        USART_SendData(USART2,pTX_Struct->Buffer[i]);
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
    }
    //等待全部连续数据发送完毕。
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);

    //帧间延时，开启t3.5。
    //延时结束会使bBusy复位。
    T3_5_Restart();
    //等待帧间隔结束。
    while(Modbus_Status_Struct.bBusy);
}

//接收数据帧
void ReceiveFrame(ModRTU_RX_Struct* pRX_Struct)
{
    //清空接收缓冲区。
    pRX_Struct->u16Index = (uint16_t)0;
    //复位帧可读标识
    Modbus_Status_Struct.bFrame_ReadEnb = FALSE;
    //复位数据可读取标识。
    Modbus_Status_Struct.bDone = FALSE;
    RS485_RXENB; //接收使能
    //处于接收状态,以便在接收一帧后校验CRC.
    Modbus_Status_Struct.bTxRx_Mode = TRUE;
    //设置系统状态进入空闲.
    Modbus_Status_Struct.bBusy = FALSE;

    //数据帧接收后会自动添加延时。
    //接收结束会自动复位忙标识并置位帧可读标识。
}





