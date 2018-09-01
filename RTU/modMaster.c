/*
*Modbus-RTU��վͨѶ
*���:azjiao
*�汾:0.1
*���ڣ�2018-07-25
*/
#include <stdio.h>
#include "modMaster.h"
#include "led.h"
#include "usart1.h"

//Modbus��վͨѶ����
//Mb_Addr:��վ��ַ
//bMode:ģʽ0����1д
//DataAddr:�����վ���ݵ�ַ
//Num:����byte����,�����Ƕ�ȡ��Ԫ��������Ҳ������д���Ԫ��������
//pTr:��վ�ش����ݻ�����������͵ĸ�ֵ���ݡ�
//Data_Addr:Ԫ����ַ����Ԫ�����͡�
void Modbus_Master(uint8_t MbAddr, bool bMode, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{

    //�ж�Ԫ������:HoldReg,��ʶΪ4xxxx.
    if(DataAddr >= 40000 && DataAddr < 50000)
    {
        //д����������д���HoldReg,Func=0x10
        if(bMode)
        {
            MdRTUFunc_0x10(MbAddr, DataAddr, Num, pTr);
        }
        //������������д���HoldReg,Func=0x03
        else{
            MdRTUFunc_0x03(MbAddr, DataAddr, Num, pTr);
        }
    }

    //�ж�Ԫ�����ͣ�AI����ʶΪ3xxxx.
    if(DataAddr >= 30000 && DataAddr < 40000)
    {
        //3xxxxֻ֧�ֶ�����
        if(!bMode)
        {
            MdRTUFunc_0x04(MbAddr, DataAddr, Num, pTr);
        }
    }

    //�ж�Ԫ�����ͣ�DI����ʶΪ1xxxx.
    if(DataAddr >= 10000 && DataAddr < 20000)
    {
        //1xxxxֻ֧�ֶ�����
        if(!bMode)
        {
            MdRTUFunc_0x02(MbAddr, DataAddr, Num, pTr);
        }
    }

    //�ж�Ԫ�����ͣ�DQ����ʶΪ0xxxx.
    //if(DataAddr >= (uint16_t)0 && DataAddr < 9999)
    if(DataAddr < 9999)  //�޷���������0�Ƚ������塣
    {
        //ǿ��DQ
        if(bMode)
        {
            MdRTUFunc_0x0F(MbAddr, DataAddr, Num, pTr);
        }
        //��DQ
        else{
            MdRTUFunc_0x01(MbAddr, DataAddr, Num, pTr);
        }
    }




}


//Function:0x10
//д��������ı��ּĴ�����
void MdRTUFunc_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ 4xxxx
    uint16_t  Addr = DataAddr - 40000;

    //�������ݴ��Ϊ����֡��
    EnCode_0x10(MbAddr, Addr, Num, pTr);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);

    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x10���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x10(&RX_Struct, MbAddr))
            {
                //test
                printf("F0x10����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ��
            else{
            //��λ��ɡ�д��������Ҫת�����ص��������ݡ�
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x10����֡CRC16У��ʧ�ܣ�\n");
    }
}

//����0x10
//�����Ľ�������TX_Struct�С�
//pTrָ��ֵ�ֽ����顣
bool EnCode_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    uint16_t CRC16;
    uint16_t j;
    if(Num > 123) return FALSE;  //Ԫ���������ܴ���123.

    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x10;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�
    //���ݳ���:Num*2
    TX_Struct.Buffer[6] = Num << 1;
    //������,����ȡ��������Num.
    for(int i = 0; i < TX_Struct.Buffer[6]; i++)
    {
        TX_Struct.Buffer[7 + i] = *(pTr + i);
    }

    //����CRC����
    //j��CRC��ſ�ʼ��Ԫ,���ֽ���ǰ��
    j = TX_Struct.Buffer[6] + 7;
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, j);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[j] = CRC16;
    TX_Struct.Buffer[j+1] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    return TRUE;
}

//�������ݽ���0x10
//Mb_Addr�Ǵ�վվ�š�
bool UnCode_0x10(ModRTU_RX_Struct * pRX_Struct, uint8_t MbAddr)
{

    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //ʡ�ԱȽϷ��ص��������ݣ������롢Ԫ����ַ��Ԫ��������
        //��վ����ȷ
        if(pRX_Struct->Buffer[0] == MbAddr)
            return TRUE;
        else
            return FALSE;
    }
    //�����վû�з�����ᳬʱ.
    //�����վ�쳣��
    else{
        return FALSE;
    }
}

//Function:0x03
//����������ı��ּĴ�����
void MdRTUFunc_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ:4xxxx
    uint16_t Addr = DataAddr - 40000;

    //�������ݴ��Ϊ����֡��
    EnCode_0x03(MbAddr, Addr, Num);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);
    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x03���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x03(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x03����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ�������Ѿ�ת����ָ���洢����
            else{
                //��λ���ݿɶ�ȡ��ʶ��
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x03����֡CRC16У��ʧ�ܣ�\n");
    }
}

//Function:0x03
//����0x03
//�����Ľ�������TX_Struct�С�
bool EnCode_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 125) return FALSE; //����ȡ������Ԫ���������ܳ���125����
    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x03;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = 8;
    return TRUE;
}

//�������ݽ���0x03
//Mb_Addr�Ǵ�վվ�š�
//pTr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool UnCode_0x03(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x03))
        {
            //�Խ��յ���HoldRegֵ����ת����ֵ������3��ʼ��
            uint8_t Num = pRX_Struct->Buffer[2];
            for(int i = 0; i < Num; i++)
            {
                *(pTr + i) = pRX_Struct->Buffer[3 + i];
            }
            return TRUE;
        }
        else
            return FALSE;
    }
    //�����վ�쳣
    else{
        return FALSE;
    }
}

//Function:0x04
//��AI:����洢��
void MdRTUFunc_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ:3xxxx
    uint16_t Addr = DataAddr - 30000;

    //�������ݴ��Ϊ����֡��
    EnCode_0x04(MbAddr, Addr, Num);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);
    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x04���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x04(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x04����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ�������Ѿ�ת����ָ���洢����
            else{
                //��λ���ݿɶ�ȡ��ʶ��
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x04����֡CRC16У��ʧ�ܣ�\n");
    }
}

//����0x04
//�����Ľ�������TX_Struct�С�
bool EnCode_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 125) return FALSE; //����ȡ������Ԫ���������ܳ���125����
    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x04;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = 8;
    return TRUE;
}

//�������ݽ���0x04
//pTr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool UnCode_0x04(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x04))
        {
            //�Խ��յ���AIֵ����ת����ֵ������3��ʼ��
            uint8_t Num = pRX_Struct->Buffer[2];
            for(int i = 0; i < Num; i++)
            {
                *(pTr + i) = pRX_Struct->Buffer[3 + i];
            }
            return TRUE;
        }
        else
            return FALSE;
    }
    //�����վ�쳣
    else{
        return FALSE;
    }
}


/*------------------------------------------------------------*/

//Function:0x02
//�����������������ɢ��DI
void MdRTUFunc_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ:1xxxx
    uint16_t Addr = DataAddr - 10000;

    //�������ݴ��Ϊ����֡��
    EnCode_0x02(MbAddr, Addr, Num);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);
    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x02���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x02(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x02����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ�������Ѿ�ת����ָ���洢����
            else{
                //��λ���ݿɶ�ȡ��ʶ��
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x02����֡CRC16У��ʧ�ܣ�\n");
    }
}

//����0x02
bool EnCode_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 2000) return FALSE; //����ȡ������Ԫ���������ܳ���2000����
    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x02;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = 8;
    return TRUE;
}

//�������ݽ���0x02
//pTr���ڴ�Ŷ�ȡ��DIֵ��
bool UnCode_0x02(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x02))
        {
            //�Խ��յ���DIֵ����ת����ֵ������3��ʼ��
            uint8_t Num = pRX_Struct->Buffer[2];
            for(int i = 0; i < Num; i++)
            {
                *(pTr + i) = pRX_Struct->Buffer[3 + i];
            }
            return TRUE;
        }
        else
            return FALSE;
    }
    //�����վ�쳣
    else{
        return FALSE;
    }
}

//Function:0x0F
//д��������������ɢ��DQ
void MdRTUFunc_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ
    uint16_t  Addr;
    //Ԫ����ַ 0xxxx
    Addr = DataAddr - 0;

    //�������ݴ��Ϊ����֡��
    EnCode_0x0F(MbAddr, Addr, Num, pTr);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);

    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x0F���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x0F(&RX_Struct, MbAddr))
            {
                //test
                printf("F0x0F����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ��
            else{
                //��λ��ɡ�д��������Ҫת�����ص��������ݡ�
                Modbus_Status_Struct.bDone = TRUE;                
            }                     
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x0F����֡CRC16У��ʧ�ܣ�\n");
    }
}

//����
bool EnCode_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    uint16_t CRC16;
    uint16_t j;
    if(Num > 1976) return FALSE;  //Ԫ���������ܴ���1976.

    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x0F;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�
    //���ݳ���:(Num%8)?Num/8+1:Num/8;
    TX_Struct.Buffer[6] = (Num%8)? Num/8+1: Num/8;
    //������,����ȡ��������Num.
    for(int i = 0; i < TX_Struct.Buffer[6]; i++)
    {
        TX_Struct.Buffer[7 + i] = *(pTr + i);
    }

    //����CRC����
    //j��CRC��ſ�ʼ��Ԫ,���ֽ���ǰ��
    j = TX_Struct.Buffer[6] + 7;
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, j);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[j] = CRC16;
    TX_Struct.Buffer[j+1] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = j + 2;
    return TRUE;
}

//�������ݽ���
//pTr���ڴ�Ŷ�ȡ��DIֵ��
bool UnCode_0x0F(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //ʡ�ԱȽϷ��ص��������ݣ������롢Ԫ����ַ��Ԫ��������
        //��վ����ȷ
        if(pRX_Struct->Buffer[0] == MbAddr)
            return TRUE;
        else
            return FALSE;
    }
    //�����վû�з�����ᳬʱ.
    //�����վ�쳣��
    else{
        return FALSE;
    }
}

//Function:0x01
//��ȡ������������ɢ��DQ
void MdRTUFunc_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //Ԫ����ַ:0xxxx
    uint16_t Addr = DataAddr - 00000;

    //�������ݴ��Ϊ����֡��
    EnCode_0x01(MbAddr, Addr, Num);
    //����
    SendFrame(&TX_Struct);
    //���ͺ�ת����ա�
    ReceiveFrame(&RX_Struct);
    //Ӧ��ʱ���ʹ�ܡ�
    RespTimeOut_Enb();
    //�ȴ����գ�ת����ղ�һ��ͨѶ�ӿ�æ�����Ի���������Ƿ�ɶ���
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x01���ճ�ʱ!\n");

    //���պ����
    //���Ӧ��ʱ�򲻽���,�����¸������ط��������һ֡���ͣ���
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //����޴����֡���н��롣
        if(!Modbus_Status_Struct.bErr)
        {
            //����������ݲ�����
            if(!UnCode_0x01(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x01����֡��������Ϣ���������վ����֡ \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //������ȷ�������Ѿ�ת����ָ���洢����
            else{
                //��λ���ݿɶ�ȡ��ʶ��
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //�������CRCУ��ʧ�ܡ�
        else
            printf("F0x01����֡CRC16У��ʧ�ܣ�\n");
    }
}

//����
bool EnCode_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 2000) return FALSE; //����ȡ������Ԫ���������ܳ���2000����
    //���ͻ���������
    TX_Struct.u16Index = 0;
    //��վ��ַ
    TX_Struct.Buffer[0] = MbAddr;
    //������
    TX_Struct.Buffer[1] = 0x01;
    //Ԫ����ַ
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //Ԫ������
    TX_Struct.Buffer[4] = Num >> 8; //���ֽ�
    TX_Struct.Buffer[5] = Num;  //���ֽ�

    //����CRC����
    //���ɵ�CRC16���ֽ���ǰ��
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //���CRC16,���ֽ���ǰ
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //֡�����ֽ�����
    TX_Struct.u16Index = 8;
    return TRUE;
}

//�������ݽ���
bool UnCode_0x01(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //������յ��Ĺ������λ��Ϊ1�������������
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //��վ����ȷ�ҹ�������ȷ
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x01))
        {
            //�Խ��յ���DIֵ����ת����ֵ������3��ʼ��
            uint8_t Num = pRX_Struct->Buffer[2];
            for(int i = 0; i < Num; i++)
            {
                *(pTr + i) = pRX_Struct->Buffer[3 + i];
            }
            return TRUE;
        }
        else
            return FALSE;
    }
    //�����վ�쳣
    else{
        return FALSE;
    }
}



