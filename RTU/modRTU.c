#include <stdio.h>
#include "dev.h"
#include "modRTU.h"
#include "modMaster.h"

//�������ݻ������ṹ.
ModRTU_RX_Struct RX_Struct;
//�������ݻ������ṹ.
ModRTU_TX_Struct TX_Struct;
//���û�����
ModRTU_Buffer Data_Struct;

//ͨѶ���ƽṹ.
ModRTU_Status_Struct Modbus_Status_Struct;
//RS485�˿ڳ�ʼ�����ݡ�
struct RS485Init_Struct RS485_InitStruct;

//RS485�˿ڳ�ʼ��.
//��Ӣ��RS485��USART2�����õ�PA2��PA3.
//��ʼ��������������RS485InitStruct��
void RS485_Init(struct RS485Init_Struct RS485InitStruct)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    USART_InitTypeDef  USART_InitStruct;
    NVIC_InitTypeDef  NVIC_InitStruct;

    //ʱ��ʹ��:PA�ں�PD��
    //PA����USART2����Ҫ����PD7����RS485��ΧоƬ�շ�ѡ����Ҫ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);
    //USART2ʱ��ʹ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    //PD7���ã����ڿ���RS485��ƽת��оƬ�շ�ʹ�ܡ�
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; //���������
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    //PA2��PA3��������
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;  //PA2��TX���Ϳڣ�����Ϊ�������������
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    //GPIO_SetBits(GPIOA, GPIO_Pin_2); //��TX��ʼ��Ϊ�ߵ�ƽ
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //PA3��RX���ܿڣ�����Ϊ�������롣
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,ENABLE);//��λ����2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART2,DISABLE);//ֹͣ��λ

    //USART2����
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_BaudRate = RS485InitStruct.u32BoundRate;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1; //Ӧ�ù̶�Ϊ1bitֹͣΪ��Ϊ��ͨ�ÿ������á�
    switch(RS485InitStruct.u16Parity)
    {
        case 0:USART_InitStruct.USART_Parity = USART_Parity_No;break; //��У��
        case 1:USART_InitStruct.USART_Parity = USART_Parity_Odd;break; //��У��
        case 2:USART_InitStruct.USART_Parity = USART_Parity_Even;break; //żУ��
    }
    USART_Init(USART2, &USART_InitStruct);


    //�����ж����ȼ�
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //ʹ�ܴ���2�����ж�
    USART_ITConfig(USART2, USART_IT_RXNE,ENABLE);

    //ʹ�ܴ���2
    USART_Cmd(USART2, ENABLE);

    //t3.5��t1.5���ö�ʱ����ʼ�����ݲ�ʹ�ܣ����ڼ��.
    //TResponse��ʱ������Ӧ��ʱ��⡣
    T15_35_Init();
    TResponse_Init();

     //Ĭ��Ϊ����ģʽ
     RS485_RXENB;
}

//ͨѶ�����ö�ʱ����ʼ�������ݲ�ʹ�ܡ�
//t1.5��t3.5����TIM6,�Ҽ���ʱ��һ������100kHz��0.01ms=10us.
//ϵͳֻʹ����t3.5��
void T15_35_Init(void)
{
    //Ĭ�ϳ�ʼ��Ϊt3.5.
    Time6_Init2us(RS485_InitStruct.u16T3_5_us);
    TIM_Cmd(T3_5, DISABLE);
}

//Ӧ��ʱ��ʱ����ʼ����
//Ӧ��ʱ��ʱ������ʱ��Ϊ10kHz,0.1ms.
void TResponse_Init(void)
{
    Time7_Init2ms(TRESPONSE_ms);
    TIM_Cmd(TRESPONSE, DISABLE);
}

//Modbus��ʼ��
void Modbus_Init(void)
{
    float fOneByteTime;  //t3.5��ʱʱ��us.
    //RS485�˿ڳ�ʼ��:USART2��PD7��,t3.5��t1.5��ʱ����ʼ��.
    RS485_InitStruct.u16DataBit = USART_WordLength_8b;  //8λ����λ
    RS485_InitStruct.u16Parity = 0;                     //У��λ��
    RS485_InitStruct.u16StopBit = USART_StopBits_1;     //ֹͣλ1λ
    RS485_InitStruct.u32BoundRate = 9600;              //������

    //���ݲ����ʼ���t1.5��t3.5�Ķ�ʱʱ�䡣
    if(RS485_InitStruct.u32BoundRate <= 19200)
        fOneByteTime = (11/(float)RS485_InitStruct.u32BoundRate) * 1000000;  //һ���ֽڷ��͵�ʱ��(us)��
    else
        fOneByteTime = 1750; //ֻ�в����ʴ���19200ʱ��ʹ�ù̶���ʱ��
    RS485_InitStruct.u16T1_5_us = 1.5 * fOneByteTime;
    RS485_InitStruct.u16T3_5_us = 3.5 * fOneByteTime;
    RS485_Init(RS485_InitStruct);


    //ͨѶ��������ʼ��.
    //���ջ���������:�Ǳ�Ҫ.
    for(int i = 0; i < FRAME_MAXLEN; i++)
    {
        RX_Struct.Buffer[i] = 0;
    }
    // ���ջ�������ǰ��������.
    RX_Struct.u16Index = 0;

    //���ͻ���������:�Ǳ�Ҫ.
    for(int i = 0; i < FRAME_MAXLEN; i++)
    {
        TX_Struct.Buffer[i] = 0;
    }
    // ���ջ�������ǰ��������.
    TX_Struct.u16Index = 0;

    //ͨѶ״̬�ṹ��ʼ��
    Modbus_Status_Struct.bBusy = FALSE; //����
    Modbus_Status_Struct.bErr = FALSE; //�޴�
    Modbus_Status_Struct.u16CommErr = 0; //�޴�
    Modbus_Status_Struct.bFrame_ReadEnb = FALSE; //֡���ɶ�
    Modbus_Status_Struct.bDone = FALSE;  //���ݲ��ɶ�
    Modbus_Status_Struct.bResponse_TimeOut = FALSE; //����ʱ
    Modbus_Status_Struct.iErrCount = 0;  //�����ۻ�ֵ����

}


//����2�����жϷ�����
//�����ڽ��յ�һ���ֽ�ʱ�������ж�.
void USART2_IRQHandler(void)
{
    u8 u8Data;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        //�����ֽ����ݲ��Զ�����жϱ�ʶ.
        u8Data = USART_ReceiveData(USART2);
        Modbus_Status_Struct.bBusy = TRUE;  //æ
        Modbus_Status_Struct.bFrame_ReadEnb = FALSE; //֡���ɶ�ȡ��

        //������ֽ��մ���,�����ô�����Ϣ.
        if(USART_GetFlagStatus(USART2, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
        {
           Modbus_Status_Struct.u16CommErr = 3;  //ͨѶ���մ���
           Modbus_Status_Struct.bErr = TRUE;
        }
        else
        {
            Modbus_Status_Struct.u16CommErr = 0;
            Modbus_Status_Struct.bErr = FALSE;
        }

        //����ֽڽ����޴��ҽ��ջ�����û��Խ�������ת������,����������t3.5��ʱ��.
        //�����򲻴������ݵ����ջ�������������CRCУ�����Ҳ�������ս������
        if((Modbus_Status_Struct.u16CommErr == 0) && (RX_Struct.u16Index < FRAME_MAXLEN -1))
        {
            //ת�����յ����ֽ�����.
            RX_Struct.Buffer[RX_Struct.u16Index] = u8Data;
            RX_Struct.u16Index++;

            //��λ����������t3.5��ʱ��,�������֡�Ƿ����.
            T3_5_Restart();
        }

    }
}

//��λ������T3.5��ʼ��ʱ.
void T3_5_Restart(void)
{
    TIM_ClearITPendingBit(T3_5, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.
    TIM_SetCounter(T3_5, 0); //��λ���ϼ�������ǰֵΪ0.
    TIM_SetAutoreload(T3_5, RS485_InitStruct.u16T3_5_us/10 - 1);
    TIM_Cmd(T3_5, ENABLE); //ʹ�ܶ�ʱ����ʼ��ʱ.
}

//t3.5��ʱ���жϷ�������
//t1.5���ڼ���ֽڽ����Ƿ�ʱ,����汾ȡ���ֽ������Լ�⡣
//t3.5���ڼ��֡�Ƿ����.
//��t3.5�����ڽ��ռ��֡����Ҳ���ڷ���֡����ʱ.
//�������t3.5�ж����ʾ����֡������֡���ͽ���,ת�����״̬.
void TIM6_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        //��λӦ��ʱ��ʱ����ֹͣ������
        TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update);
        TIM_SetCounter(TRESPONSE, 0);
        TIM_Cmd(TRESPONSE, DISABLE);
        Modbus_Status_Struct.bResponse_TimeOut = FALSE;

        //��λt3.5��ʱ����ʧ��,ֹͣ��ʱ���.
        TIM_ClearITPendingBit(T3_5, TIM_IT_Update);
        TIM_SetCounter(T3_5, 0); //��λ���ϼ�������ǰֵΪ0.
        TIM_Cmd(T3_5, DISABLE);  //ʧ�ܶ�ʱ��.

        //�����ڽ���ʱУ��CRC16.
        if(Modbus_Status_Struct.bTxRx_Mode)
        {
            //�ж�����֡����Ч��.
            //ֻ��ͨѶ����֡�������жϣ�Ҳ��CRCУ�顣
            if(CRC16Check(RX_Struct))
            {
                Modbus_Status_Struct.bErr = FALSE;   //����֡�ɶ�ʱbErrΪFALSE�����Կ���ȡ����bErr���жϡ�
                //����ϵͳ״̬�������.
                Modbus_Status_Struct.bBusy = FALSE;
                Modbus_Status_Struct.bFrame_ReadEnb = TRUE;  //֡�ɶ�ȡ��
                //���֡�ɶ����ȴ�����֡ʱ���ٽ������ݣ���ģʽ��Ϊ����,������յ������ݱ���ӵ�֡ĩ������⡣
                RS485_TXENB; //����ʹ��
            }
            //���CRCʧ�ܣ�������֡�����½��ա�
            else
            {
                Modbus_Status_Struct.iErrCount++;//ͨѶ�������ͳ�ơ�
                Modbus_Status_Struct.bErr = TRUE;
                Modbus_Status_Struct.u16CommErr = 2; //CRCУ��ʧ�ܡ�
                //������������֡���������������ա�
                ReceiveFrame(&RX_Struct);
            }

        }
        //������ڷ�������У�顣
        else
            //����ϵͳ״̬�������.
            Modbus_Status_Struct.bBusy = FALSE;
    }
}

//����Ӧ��ʱ�������
//����TRESPONSE��ʱ����TIM7.
void RespTimeOut_Enb(void)
{
    Modbus_Status_Struct.bResponse_TimeOut = FALSE; //��λ��ʱ��ʶ��
    TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update); //�����ʱ���жϸ��±�ʶ.
    TIM_SetCounter(TRESPONSE, 0); //��λ���ϼ�������ǰֵΪ0.
    TIM_Cmd(TRESPONSE, ENABLE);   //������ʱ��ʱ����
}

//Ӧ��ʱ��ʱ���жϷ�����
//����ʱ�����ճ�ʱ��
void TIM7_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        Modbus_Status_Struct.bResponse_TimeOut = TRUE;
        //��λӦ��ʱ��ʱ�䲢ֹͣ������
        TIM_ClearITPendingBit(TRESPONSE, TIM_IT_Update);
        TIM_SetCounter(TRESPONSE, 0);
        TIM_Cmd(TRESPONSE, DISABLE);
    }
}

//CRC16У��������
//Dataָ���У�����ݵ�ָ�룬Len�������ֽڳ���.
//����ԭ��16λCRC�����ֽ���ǰ��
u16 CRC16Gen(u8* pByteData, u16 u16Len)
{
    u16 CRC16 = 0xFFFF;  //CRCԤ��ֵ
    for(u16 i = 0; i < u16Len; i++)  //���δ����������ݡ�
    {
        CRC16 ^= *(pByteData + i);   //�������

        for(u8 bit = 0; bit < 8; bit++)
        {
            //��������Ƴ�λΪ1�������ƺ�������ʽ��
            if(CRC16 & 0x0001)
            {
                CRC16 >>= 1;
                CRC16 ^= 0xA001;  //У�����ʽ��CRC16_Modbus��׼��
            }
            //����ֻ������һλ��
            else{
                CRC16 >>= 1;
            }
        }
    }
    return (CRC16);
}

//CRC16У��
bool CRC16Check(ModRTU_RX_Struct RX_Struct)
{
    /*
    u16 CRC16, CRC16_RX = 0x0;
    //ȡ��RX_Struct���ݵ�У���롣
    CRC16_RX |= RX_Struct.Buffer[RX_Struct.u16Index-1]; //���ֽڡ�
    CRC16_RX <<= 8;
    CRC16_RX |= RX_Struct.Buffer[RX_Struct.u16Index-2]; //���ֽڡ�

    CRC16 = CRC16Gen(RX_Struct.Buffer, RX_Struct.u16Index-2);  //����CRC16,���ֽ���ǰ��
    if(CRC16 == CRC16_RX)
        return TRUE;
    else
        return FALSE; */


    //ʹ�����°Ѻ�CRC��������У�飬������ɵ�CRCΪ0x00����ȷ��������ȷ��
    if(CRC16Gen(RX_Struct.Buffer, RX_Struct.u16Index) == (uint16_t)0)
        return TRUE;
    else
        return FALSE;
}



//��������֡
void SendFrame(ModRTU_TX_Struct* pTX_Struct)
{
    RS485_TXENB; //����ʹ��
    //����״̬,�Ա��ڷ��ͺ�У��CRC.
    Modbus_Status_Struct.bTxRx_Mode = FALSE;

    for(u8 i = 0; i < pTX_Struct->u16Index; i++)
    {
        Modbus_Status_Struct.bBusy = TRUE;  //æ
        USART_SendData(USART2,pTX_Struct->Buffer[i]);
        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
    }
    //�ȴ�ȫ���������ݷ�����ϡ�
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET);

    //֡����ʱ������t3.5��
    //��ʱ������ʹbBusy��λ��
    T3_5_Restart();
    //�ȴ�֡���������
    while(Modbus_Status_Struct.bBusy);
}

//��������֡
void ReceiveFrame(ModRTU_RX_Struct* pRX_Struct)
{
    //��ս��ջ�������
    pRX_Struct->u16Index = (uint16_t)0;
    //��λ֡�ɶ���ʶ
    Modbus_Status_Struct.bFrame_ReadEnb = FALSE;
    //��λ���ݿɶ�ȡ��ʶ��
    Modbus_Status_Struct.bDone = FALSE;
    RS485_RXENB; //����ʹ��
    //���ڽ���״̬,�Ա��ڽ���һ֡��У��CRC.
    Modbus_Status_Struct.bTxRx_Mode = TRUE;
    //����ϵͳ״̬�������.
    Modbus_Status_Struct.bBusy = FALSE;

    //����֡���պ���Զ������ʱ��
    //���ս������Զ���λæ��ʶ����λ֡�ɶ���ʶ��
}





