#ifndef __MODRTU_H
#define __MODRTU_H
/*
*Modbus_RTU定义了Modbus通讯的帧开始和结束信息：以接收第一个字节开始以t3.5结束。
*通讯控制定时时间
*TIM6的时基为0.01ms：CPS=719，即计数时钟为100kHz.
*t3.5是帧间间隔时间看门狗，标准是不小于3.5个字节发送时间。
*t1.5是字节传输连续看门狗，标准是不大于1.5个字节发送时间。
*t3.5和t1.5使用TIM6基本定时器，定时时基设置为0.01ms.
*当波特率大于19200时，t1.5和t3.5为固定值：t1.5=750us,t3.5=1.75ms.
*TIM7的时基为0.1ms,计数器时钟CK_CLK为10kHz.
*TResponse为TIM7，用于超时应答定时器,定时时间为500ms.
*/

#include "template.h"

//MORS为TRUE表示主站,为FALSE表示从站.
#define MORS   (bool)TRUE

//t3.5和t1.5定时us数.
//宏定义的T3_5和T1_5的单位是us单位，需要在定时器初始化中转换为ARR值。
//取消宏定义，改为在通讯初始化中根据波特率来设置t1.5和t3.5的定时时间。
//#define T3_5_us    (u16)1750
//#define T1_5_us    (u16)750

#define T3_5       TIM6
//#define T1_5       TIM7
//定义t1.5也使用同一个TIM6基本定时器。
#define T1_5  TIM6
//通讯超时定时器：TIM7
#define TRESPONSE_ms (u16)500
#define TRESPONSE   TIM7

//RS485初始化参数结构
struct RS485Init_Struct
{
    u32 u32BoundRate;  //波特率。
    u16 u16DataBit;  //数据位字长
    u16 u16StopBit;  //停止位
    u16 u16Parity;  //校验位:0无校验，1奇校验，2偶校验
    u16 u16T1_5_us;  //t1.5定时时间。
    u16 u16T3_5_us;  //t3.5定时时间。
};
extern struct RS485Init_Struct RS485_InitStruct;

// 帧最大长度:字节数.Modbus规定一帧长度不超过256字节。
#define FRAME_MAXLEN  256

// 接收帧数据结构
typedef struct
{
    u8 Buffer[FRAME_MAXLEN];  //接收缓冲区
    u16 u16Index;    //接收缓冲区字节索引
}ModRTU_RX_Struct;
extern ModRTU_RX_Struct RX_Struct;

// 发送帧数据结构
typedef struct
{
    u8 Buffer[FRAME_MAXLEN]; //发送缓冲区
    u16 u16Index;   // 发送缓冲区字节索引
}ModRTU_TX_Struct;
extern ModRTU_TX_Struct TX_Struct;

typedef struct
{
    u8 Buffer[FRAME_MAXLEN];
    u16 u16Len;
}ModRTU_Buffer;
extern ModRTU_Buffer Data_Struct;   //通讯共用缓冲区

//Modbus-RTU通讯状态结构
typedef  struct
{
    bool bTxRx_Mode;  //处于0:发送还是1:接收状态.
    bool bResponse_TimeOut;  //应答超时。
    bool bBusy;         //忙。对于接收，接收开始不一定忙。
    bool bFrame_ReadEnb; //接收到的帧可读取标识。0：不可读取；1：可读取。
    bool bDone;  //主站一次通讯完成，数据可读取。1:回传的数据可以读取。
    bool bErr;          //接收帧有错误.
    u16 u16CommErr;     //通讯错误信息.0:无错;1:非本站信息(主站不用错误1);2:帧CRC错误;3:字节接收出错.
    unsigned int iErrCount;  //校验失败计数。
}ModRTU_Status_Struct;
extern ModRTU_Status_Struct  Modbus_Status_Struct;

//RS485端口初始化.
//精英版RS485是USART2，复用到PA2和PA3.
//初始化的数据来自于RS485InitStruct。
void RS485_Init(struct RS485Init_Struct RS485InitStruct);

//RS485读写使能控制：使用的是PD.7口位带操作，1为发0为收。
#define RS485_TXENB  PDout(7)=1
#define RS485_RXENB  PDout(7)=0

void T15_35_Init(void);
void TResponse_Init(void);

//复位并重启T3.5开始定时.
void T3_5_Restart(void);

//Modbus初始化
void Modbus_Init(void);

//CRC16校验码生成
u16 CRC16Gen(u8* pData, u16 u16Len);

//CRC校验
bool CRC16Check(ModRTU_RX_Struct RX_Struct);

//数据帧发送
void SendFrame(ModRTU_TX_Struct* pTX_Struct);
//数据帧接收
void ReceiveFrame(ModRTU_RX_Struct* pRX_Struct);
//接收应答超时检测启动
void RespTimeOut_Enb(void);


#endif
