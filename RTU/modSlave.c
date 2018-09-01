/*********************************************************
���ܣ� Modbus-RTU��վͨѶ
������ ʵ���˴�վͨѶ�ļ���������
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��07��25��
*********************************************************/
#include <stdio.h>
#include "dev.h"
#include "modSlave.h"
#include "modRTU.h"
#include "usart1.h"

//���������Դ
Type_0xxxx    DQ_0xxxx;
Type_1xxxx    DI_1xxxx;
Type_3xxxx    AI_3xxxx;
Type_4xxxx    HR_4xxxx;

//��վ������
//Modbus��ʼ��ʱ485��Ϊ����״̬��
void Modbus_Slave(void)
{
    //������յ�������֡(����֡�ɶ�ʱ�˿�Ϊ����״̬����ֹ���յ��޹�����)��
    if(Modbus_Status_Struct.bFrame_ReadEnb)
    {
        //����޴������
        //��֡�ɶ�ʱ��ʵbErrΪFALSE������ȡ���жϡ�
        if(!Modbus_Status_Struct.bErr)
        {
            //�����жϴ�վ��ַ�Ƿ������
            //վ��ַ���������,�������ա�
            if(RX_Struct.Buffer[0] != MBSLAVE_ADDR){                
                ReceiveFrame(&RX_Struct);
                return;
            }

            //��ȡ���������ж�
            switch(RX_Struct.Buffer[1]){
                case 0x01:SlaveFunc_0x01();  //�����DQ_0xxxx
                          break;
                case 0x0F:SlaveFunc_0x0F();  //д���DQ_0xxxx
                          break;
                case 0x02:SlaveFunc_0x02();  //�����DI_1xxxx
                          break;
                case 0x04:SlaveFunc_0x04();  //�����AI_3xxxx
                          break;
                case 0x03:SlaveFunc_0x03();  //�����HoldReg_4xxxx
                          break;
                case 0x10:SlaveFunc_0x10();  //д���HoldReg_4xxxx
                          break;
                default:  Default_NonSupport();  //��֧�ֵĹ��ܴ���

            }
            //������ת����ա�
            ReceiveFrame(&RX_Struct);
        }
    }
}

//Function:0x01
//��ȡ���������DQ��0xxxx
//ִ�к���ʱ��վ��ַ�Ѿ������
void SlaveFunc_0x01(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x01;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000(0x07D0)��DQ����
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= (DATA_MAXLEN * 8))
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            TX_Struct.Buffer[2] = u8ByteNum;

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ�ֽڿ�ʼ������
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ�ֽ�λ������ʼֵ��
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ�ֽڳ�ʼ����.
            uint16_t u16Tx_Index = 0;  //�����ֽ�������ʼֵ��
            uint8_t u8TxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8TxMask = 0x01;  //TX�ֽڳ�ʼ���롣

            //����Ҫ��Ӧ������ֵ��ֵ��TX_Struct.Buffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DQBit_Index >= 8)
                {
                    u16DQ_Index++;
                    u8DQBit_Index = 0;
                    u8DQMask = 0x01;
                }

                if(u8TxBit_Index >= 8)
                {
                    u16Tx_Index++;
                    u8TxBit_Index = 0;
                    u8TxMask = 0x01;
                }

                if(DQ_0xxxx.u8Data[u16DQ_Index] & u8DQMask)
                    TX_Struct.Buffer[u16Tx_Index + 3] |= u8TxMask;
                else
                    TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8DQMask <<= 1;
                u8TxMask <<= 1;

                u8DQBit_Index++;
                u8TxBit_Index++;
            }
            //���һ��TX�����ֽڸ�λ���0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            j = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] |= 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);
}

//Function:0x0F
//ǿ�ƶ��������DQ��0xxxx
void SlaveFunc_0x0F(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������
    uint8_t u8ByteNum;  //�����ֽ�����
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //����������ֽ�������Ȼ����֡���溬���ֽ���(��6�ֽ�)������ȻҪ�Ƚ��Ƿ���ȷ��
    u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x0F;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000��DQ�����������Ԫ�����ʹ�����ֽ���ƥ�䡣
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0) && (u8ByteNum == RX_Struct.Buffer[6]))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN * 8)
        {
            //�������֡
            //Ԫ����ַ
            TX_Struct.Buffer[2] = RX_Struct.Buffer[2];
            TX_Struct.Buffer[3] = RX_Struct.Buffer[3];
            //��Ӧ��DQԪ������.
            TX_Struct.Buffer[4] = RX_Struct.Buffer[4];
            TX_Struct.Buffer[5] = RX_Struct.Buffer[5];

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ�ֽڿ�ʼ������
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ�ֽ�λ������ʼֵ��
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ�ֽڳ�ʼ����.
            uint16_t u16Rx_Index = 7;  //�����ֽ�������ʼֵ��
            uint8_t u8RxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8RxMask = 0x01;  //����RX�ֽڳ�ʼ���롣

            //����Ҫǿ�Ƶ�����ֵ(�ӵ�7�ֽڿ�ʼ)��ֵ��DQ_0xxxx��Ӧ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DQBit_Index >= 8)
                {
                    u16DQ_Index++;
                    u8DQBit_Index = 0;
                    u8DQMask = 0x01;
                }

                if(u8RxBit_Index >= 8)
                {
                    u16Rx_Index++;
                    u8RxBit_Index = 0;
                    u8RxMask = 0x01;
                }

                //if(DQ_0xxxx.u8Data[u16DQ_Index] & u8DQMask)
                if(RX_Struct.Buffer[u16Rx_Index] &u8RxMask)
                    DQ_0xxxx.u8Data[u16DQ_Index] |= u8DQMask;
                    //TX_Struct.Buffer[u16Tx_Index + 3] |= u8TxMask;
                else
                    DQ_0xxxx.u8Data[u16DQ_Index] &= (u8DQMask ^ 0xFF);
                    //TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8DQMask <<= 1;
                u8RxMask <<= 1;

                u8DQBit_Index++;
                u8RxBit_Index++;

                //DQ_0xxxx.u8Data[i + u16DataAddr] = RX_Struct.Buffer[7 + i];
            }

            j = 6; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] |= 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);
}

//Function:0x02
//��ȡ���������������ɢ��DI,1xxxx
void SlaveFunc_0x02(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������
    uint16_t u16ByteIndex;  //��ַ�����ֽ���������0��ʼ���ֽ�������
    uint8_t u8BitIndex;  //��ַ�����ֽڵĿ�ʼλ��������0��ʼ��λ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x02;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���2000(0x07D0)��DQ����
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN * 8)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            TX_Struct.Buffer[2] = u8ByteNum;

            //��λu16DataAddr�����ֽ�������
            u16ByteIndex = u16DataAddr/8;
            //��λ�����ֽڵĿ�ʼλ������
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DI_Index = u16ByteIndex;  //DI�ֽڿ�ʼ������
            uint8_t u8DIBit_Index = u8BitIndex;  //DI�ֽ�λ������ʼֵ��
            uint8_t u8DIMask = 0x01 << u8DIBit_Index;  //DI�ֽڳ�ʼ����.
            uint16_t u16Tx_Index = 0;  //�����ֽ�������ʼֵ��
            uint8_t u8TxBit_Index = 0; //�����ֽ�λ������ʼֵ.
            uint8_t u8TxMask = 0x01;  //TX�ֽڳ�ʼ���롣

            //����Ҫ��Ӧ������ֵ��ֵ��TX_Struct.Buffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u16Num; i++)
            {
                if(u8DIBit_Index >= 8)
                {
                    u16DI_Index++;
                    u8DIBit_Index = 0;
                    u8DIMask = 0x01;
                }

                if(u8TxBit_Index >= 8)
                {
                    u16Tx_Index++;
                    u8TxBit_Index = 0;
                    u8TxMask = 0x01;
                }

                if(DI_1xxxx.u8Data[u16DI_Index] & u8DIMask)
                    TX_Struct.Buffer[u16Tx_Index + 3] |= u8TxMask;
                else
                    TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8DIMask <<= 1;
                u8TxMask <<= 1;

                u8DIBit_Index++;
                u8TxBit_Index++;
            }
            //���һ��TX�����ֽڸ�λ���0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            j = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);
}

//Function:0x04
//��ȡ�������������Ĵ���AI,3xxxx
void SlaveFunc_0x04(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ.
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x04;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���125��AI����
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        //�ṩ��AI_3xxxx��16bit���飬����һ����Ԫ��һ��Ԫ�����ݡ�
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����:Ԫ������*2.
            //����ʹ��u16Num<<1,��Ϊ���ܳ���125�����Դ˴�u16Num��8λ��0.
            u8ByteNum = RX_Struct.Buffer[5] << 1;
            TX_Struct.Buffer[2] = u8ByteNum;
            //����Ҫ��Ӧ������ֵ��ֵ��TX_Struct.Buffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16λAI��Դ���ݷֲ𣺸��ֽ���ǰ��
                TX_Struct.Buffer[3 + i] = AI_3xxxx.u16Data[i/2 + u16DataAddr] >> 8; //16λ���ݸ�8λ�ֽڡ�
                TX_Struct.Buffer[3 + i + 1] = AI_3xxxx.u16Data[i/2 + u16DataAddr];  //16λ���ݵ�8λ�ֽڡ�
            }

            j = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);

}

//Function:0x03
//��ȡ��������ı��ּĴ���HoldReg,4xxxx
void SlaveFunc_0x03(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x03;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���125��HR����
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        //�ṩ��AI_3xxxx��16bit���飬����һ����Ԫ��һ��Ԫ�����ݡ�
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            uint8_t u8ByteNum;  //�����ֽ�����

            //�������֡
            //��Ӧ�������ֽ�����:Ԫ������*2.
            //����ʹ��u16Num<<1,��Ϊ���ܳ���125�����Դ˴�u16Num��8λ��0.
            u8ByteNum = RX_Struct.Buffer[5] << 1;
            TX_Struct.Buffer[2] = u8ByteNum;
            //����Ҫ��Ӧ������ֵ��ֵ��TX_Struct.Buffer[3]��ʼ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16λAI��Դ���ݷֲ𣺸��ֽ���ǰ��
                TX_Struct.Buffer[3 + i] = HR_4xxxx.u16Data[i/2 + u16DataAddr] >> 8; //16λ���ݸ�8λ�ֽڡ�
                TX_Struct.Buffer[3 + i + 1] = HR_4xxxx.u16Data[i/2 + u16DataAddr];  //16λ���ݵ�8λ�ֽڡ�
            }

            j = u8ByteNum + 3; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);
}

//Function:0x10
//д��������ı��ּĴ���HoldReg,4xxxx
void SlaveFunc_0x10(void)
{
    uint16_t u16Num;  //Ԫ������
    uint16_t u16DataAddr;  //Ԫ����ַ
    uint16_t u16CRC;
    uint16_t j; //CRCװ�ص�Ԫ������
    uint8_t u8ByteNum;  //�����ֽ�����

    //��ȡԪ����ַ��������
    //Ԫ����ַ�ڵ�2��3�ֽڣ����ֽ���ǰ��
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //�����ڵ�4��5�ֽ�,���ֽ���ǰ
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //����������ֽ�������Ȼ����֡���溬���ֽ���(��6�ֽ�)������ȻҪ�Ƚ��Ƿ���ȷ��
    u8ByteNum = u16Num << 1;

    //ȡ��ʵ��Ԫ����ַ
    //���Ԫ����ַ����1�������Ԫ����ַΪ0�����һ��Ϊ���ֵ0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = 0x10;

    //�ж�Ԫ�������Ƿ����
    //���ܳ���123��HR�����������Ԫ�����ʹ�����ֽ���ƥ�䡣
    if((u16Num >= 0x0001) && (u16Num <= 0x007B) && (u8ByteNum == RX_Struct.Buffer[6]))
    {
        //�ж�Ԫ����ַ�������Ƿ���ʡ�
        //��վ��Ԫ��ַ��0��ʼ��(�����޸�Ϊ1��ʼ�������������)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            //�������֡
            //Ԫ����ַ
            TX_Struct.Buffer[2] = RX_Struct.Buffer[2];
            TX_Struct.Buffer[3] = RX_Struct.Buffer[3];
            //��Ӧ��HRԪ������.
            TX_Struct.Buffer[4] = RX_Struct.Buffer[4];
            TX_Struct.Buffer[5] = RX_Struct.Buffer[5];

            //�Ѹ�ֵ������(�ӵ�7�ֽڿ�ʼ)��ֵ��HR_4xxxx��Ӧ�ĵ�Ԫ��
            for(uint16_t i = 0; i < u8ByteNum; i+=2)
            {
                HR_4xxxx.u16Data[i/2 + u16DataAddr] = RX_Struct.Buffer[7 + i]; //���ֽ�
                HR_4xxxx.u16Data[i/2 + u16DataAddr] <<= 8;
                HR_4xxxx.u16Data[i/2 + u16DataAddr] |= RX_Struct.Buffer[7 + i + 1]; //���ֽ�
            }

            j = 6; //jΪCRC���ڵ�Ԫ��

        }
        else{
            //�����쳣02����ַ�Ƿ�
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //�����쳣03�����ݷǷ�
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    //���ʹ�����֡
    SendFrame(&TX_Struct);
}

//�쳣01,����Ĺ��ܲ�֧�֡�
void Default_NonSupport(void)
{
    uint16_t u16CRC;
    //װ������:
    //��վ��ַ
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //������
    TX_Struct.Buffer[1] = RX_Struct.Buffer[1] + 0x80;
    //�쳣��01
    TX_Struct.Buffer[2] = 0x01;

    //����CRC16��
    u16CRC = CRC16Gen(TX_Struct.Buffer, 3);
    TX_Struct.Buffer[3] = u16CRC;
    TX_Struct.Buffer[4] = u16CRC >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = 5;
    //���ʹ�����֡

    SendFrame(&TX_Struct);
    printf("This functiosn nonSupport!---%d\n",TX_Struct.Buffer[1]);
}
