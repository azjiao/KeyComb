#ifndef __MODRTU_H
#define __MODRTU_H
/*
*Modbus_RTU������ModbusͨѶ��֡��ʼ�ͽ�����Ϣ���Խ��յ�һ���ֽڿ�ʼ��t3.5������
*ͨѶ���ƶ�ʱʱ��
*TIM6��ʱ��Ϊ0.01ms��CPS=719��������ʱ��Ϊ100kHz.
*t3.5��֡����ʱ�俴�Ź�����׼�ǲ�С��3.5���ֽڷ���ʱ�䡣
*t1.5���ֽڴ����������Ź�����׼�ǲ�����1.5���ֽڷ���ʱ�䡣
*t3.5��t1.5ʹ��TIM6������ʱ������ʱʱ������Ϊ0.01ms.
*�������ʴ���19200ʱ��t1.5��t3.5Ϊ�̶�ֵ��t1.5=750us,t3.5=1.75ms.
*TIM7��ʱ��Ϊ0.1ms,������ʱ��CK_CLKΪ10kHz.
*TResponseΪTIM7�����ڳ�ʱӦ��ʱ��,��ʱʱ��Ϊ500ms.
*/

#include "template.h"

//MORSΪTRUE��ʾ��վ,ΪFALSE��ʾ��վ.
#define MORS   (bool)TRUE

//t3.5��t1.5��ʱus��.
//�궨���T3_5��T1_5�ĵ�λ��us��λ����Ҫ�ڶ�ʱ����ʼ����ת��ΪARRֵ��
//ȡ���궨�壬��Ϊ��ͨѶ��ʼ���и��ݲ�����������t1.5��t3.5�Ķ�ʱʱ�䡣
//#define T3_5_us    (u16)1750
//#define T1_5_us    (u16)750

#define T3_5       TIM6
//#define T1_5       TIM7
//����t1.5Ҳʹ��ͬһ��TIM6������ʱ����
#define T1_5  TIM6
//ͨѶ��ʱ��ʱ����TIM7
#define TRESPONSE_ms (u16)500
#define TRESPONSE   TIM7

//RS485��ʼ�������ṹ
struct RS485Init_Struct
{
    u32 u32BoundRate;  //�����ʡ�
    u16 u16DataBit;  //����λ�ֳ�
    u16 u16StopBit;  //ֹͣλ
    u16 u16Parity;  //У��λ:0��У�飬1��У�飬2żУ��
    u16 u16T1_5_us;  //t1.5��ʱʱ�䡣
    u16 u16T3_5_us;  //t3.5��ʱʱ�䡣
};
extern struct RS485Init_Struct RS485_InitStruct;

// ֡��󳤶�:�ֽ���.Modbus�涨һ֡���Ȳ�����256�ֽڡ�
#define FRAME_MAXLEN  256

// ����֡���ݽṹ
typedef struct
{
    u8 Buffer[FRAME_MAXLEN];  //���ջ�����
    u16 u16Index;    //���ջ������ֽ�����
}ModRTU_RX_Struct;
extern ModRTU_RX_Struct RX_Struct;

// ����֡���ݽṹ
typedef struct
{
    u8 Buffer[FRAME_MAXLEN]; //���ͻ�����
    u16 u16Index;   // ���ͻ������ֽ�����
}ModRTU_TX_Struct;
extern ModRTU_TX_Struct TX_Struct;

typedef struct
{
    u8 Buffer[FRAME_MAXLEN];
    u16 u16Len;
}ModRTU_Buffer;
extern ModRTU_Buffer Data_Struct;   //ͨѶ���û�����

//Modbus-RTUͨѶ״̬�ṹ
typedef  struct
{
    bool bTxRx_Mode;  //����0:���ͻ���1:����״̬.
    bool bResponse_TimeOut;  //Ӧ��ʱ��
    bool bBusy;         //æ�����ڽ��գ����տ�ʼ��һ��æ��
    bool bFrame_ReadEnb; //���յ���֡�ɶ�ȡ��ʶ��0�����ɶ�ȡ��1���ɶ�ȡ��
    bool bDone;  //��վһ��ͨѶ��ɣ����ݿɶ�ȡ��1:�ش������ݿ��Զ�ȡ��
    bool bErr;          //����֡�д���.
    u16 u16CommErr;     //ͨѶ������Ϣ.0:�޴�;1:�Ǳ�վ��Ϣ(��վ���ô���1);2:֡CRC����;3:�ֽڽ��ճ���.
    unsigned int iErrCount;  //У��ʧ�ܼ�����
}ModRTU_Status_Struct;
extern ModRTU_Status_Struct  Modbus_Status_Struct;

//RS485�˿ڳ�ʼ��.
//��Ӣ��RS485��USART2�����õ�PA2��PA3.
//��ʼ��������������RS485InitStruct��
void RS485_Init(struct RS485Init_Struct RS485InitStruct);

//RS485��дʹ�ܿ��ƣ�ʹ�õ���PD.7��λ��������1Ϊ��0Ϊ�ա�
#define RS485_TXENB  PDout(7)=1
#define RS485_RXENB  PDout(7)=0

void T15_35_Init(void);
void TResponse_Init(void);

//��λ������T3.5��ʼ��ʱ.
void T3_5_Restart(void);

//Modbus��ʼ��
void Modbus_Init(void);

//CRC16У��������
u16 CRC16Gen(u8* pData, u16 u16Len);

//CRCУ��
bool CRC16Check(ModRTU_RX_Struct RX_Struct);

//����֡����
void SendFrame(ModRTU_TX_Struct* pTX_Struct);
//����֡����
void ReceiveFrame(ModRTU_RX_Struct* pRX_Struct);
//����Ӧ��ʱ�������
void RespTimeOut_Enb(void);


#endif
