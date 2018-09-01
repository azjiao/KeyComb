/*
*Modbus-RTU主站通讯
*设计:azjiao
*版本:0.1
*日期：2018-07-25
*/
#include <stdio.h>
#include "modMaster.h"
#include "led.h"
#include "usart1.h"

//Modbus主站通讯控制
//Mb_Addr:从站地址
//bMode:模式0读或1写
//DataAddr:所需从站数据地址
//Num:数据byte长度,可能是读取的元件数量，也可能是写入的元件数量。
//pTr:从站回传数据缓冲区或待发送的赋值数据。
//Data_Addr:元件基址，含元件类型。
void Modbus_Master(uint8_t MbAddr, bool bMode, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{

    //判断元件类型:HoldReg,标识为4xxxx.
    if(DataAddr >= 40000 && DataAddr < 50000)
    {
        //写操作：连续写多个HoldReg,Func=0x10
        if(bMode)
        {
            MdRTUFunc_0x10(MbAddr, DataAddr, Num, pTr);
        }
        //读操作：连续写多个HoldReg,Func=0x03
        else{
            MdRTUFunc_0x03(MbAddr, DataAddr, Num, pTr);
        }
    }

    //判断元件类型：AI，标识为3xxxx.
    if(DataAddr >= 30000 && DataAddr < 40000)
    {
        //3xxxx只支持读操作
        if(!bMode)
        {
            MdRTUFunc_0x04(MbAddr, DataAddr, Num, pTr);
        }
    }

    //判断元件类型：DI，标识为1xxxx.
    if(DataAddr >= 10000 && DataAddr < 20000)
    {
        //1xxxx只支持读操作
        if(!bMode)
        {
            MdRTUFunc_0x02(MbAddr, DataAddr, Num, pTr);
        }
    }

    //判断元件类型：DQ，标识为0xxxx.
    //if(DataAddr >= (uint16_t)0 && DataAddr < 9999)
    if(DataAddr < 9999)  //无符号整数和0比较无意义。
    {
        //强制DQ
        if(bMode)
        {
            MdRTUFunc_0x0F(MbAddr, DataAddr, Num, pTr);
        }
        //读DQ
        else{
            MdRTUFunc_0x01(MbAddr, DataAddr, Num, pTr);
        }
    }




}


//Function:0x10
//写多个连续的保持寄存器。
void MdRTUFunc_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址 4xxxx
    uint16_t  Addr = DataAddr - 40000;

    //发送数据打包为数据帧。
    EnCode_0x10(MbAddr, Addr, Num, pTr);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);

    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x10接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x10(&RX_Struct, MbAddr))
            {
                //test
                printf("F0x10返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确。
            else{
            //置位完成。写操作不需要转储返回的其他数据。
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x10返回帧CRC16校验失败！\n");
    }
}

//编码0x10
//打包后的结果存放在TX_Struct中。
//pTr指向赋值字节数组。
bool EnCode_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    uint16_t CRC16;
    uint16_t j;
    if(Num > 123) return FALSE;  //元件数量不能大于123.

    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x10;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节
    //数据长度:Num*2
    TX_Struct.Buffer[6] = Num << 1;
    //数据域,长度取决于数量Num.
    for(int i = 0; i < TX_Struct.Buffer[6]; i++)
    {
        TX_Struct.Buffer[7 + i] = *(pTr + i);
    }

    //数据CRC生成
    //j是CRC存放开始单元,低字节在前。
    j = TX_Struct.Buffer[6] + 7;
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, j);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[j] = CRC16;
    TX_Struct.Buffer[j+1] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    return TRUE;
}

//接收数据解析0x10
//Mb_Addr是从站站号。
bool UnCode_0x10(ModRTU_RX_Struct * pRX_Struct, uint8_t MbAddr)
{

    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //省略比较返回的其他数据：功能码、元件基址、元件数量。
        //从站号正确
        if(pRX_Struct->Buffer[0] == MbAddr)
            return TRUE;
        else
            return FALSE;
    }
    //如果从站没有返回则会超时.
    //如果从站异常。
    else{
        return FALSE;
    }
}

//Function:0x03
//读多个连续的保持寄存器。
void MdRTUFunc_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址:4xxxx
    uint16_t Addr = DataAddr - 40000;

    //发送数据打包为数据帧。
    EnCode_0x03(MbAddr, Addr, Num);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);
    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x03接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x03(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x03返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确，数据已经转储在指定存储区。
            else{
                //置位数据可读取标识。
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x03返回帧CRC16校验失败！\n");
    }
}

//Function:0x03
//编码0x03
//打包后的结果存放在TX_Struct中。
bool EnCode_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 125) return FALSE; //待读取的连续元件数量不能超过125个。
    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x03;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = 8;
    return TRUE;
}

//接收数据解析0x03
//Mb_Addr是从站站号。
//pTr用于存放读取的HoldReg值。
bool UnCode_0x03(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x03))
        {
            //对接收到的HoldReg值进行转储。值从索引3开始。
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
    //如果从站异常
    else{
        return FALSE;
    }
}

//Function:0x04
//读AI:输入存储器
void MdRTUFunc_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址:3xxxx
    uint16_t Addr = DataAddr - 30000;

    //发送数据打包为数据帧。
    EnCode_0x04(MbAddr, Addr, Num);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);
    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x04接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x04(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x04返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确，数据已经转储在指定存储区。
            else{
                //置位数据可读取标识。
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x04返回帧CRC16校验失败！\n");
    }
}

//编码0x04
//打包后的结果存放在TX_Struct中。
bool EnCode_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 125) return FALSE; //待读取的连续元件数量不能超过125个。
    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x04;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = 8;
    return TRUE;
}

//接收数据解析0x04
//pTr用于存放读取的HoldReg值。
bool UnCode_0x04(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x04))
        {
            //对接收到的AI值进行转储。值从索引3开始。
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
    //如果从站异常
    else{
        return FALSE;
    }
}


/*------------------------------------------------------------*/

//Function:0x02
//读多个连续的输入离散量DI
void MdRTUFunc_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址:1xxxx
    uint16_t Addr = DataAddr - 10000;

    //发送数据打包为数据帧。
    EnCode_0x02(MbAddr, Addr, Num);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);
    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x02接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x02(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x02返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确，数据已经转储在指定存储区。
            else{
                //置位数据可读取标识。
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x02返回帧CRC16校验失败！\n");
    }
}

//编码0x02
bool EnCode_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 2000) return FALSE; //待读取的连续元件数量不能超过2000个。
    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x02;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = 8;
    return TRUE;
}

//接收数据解析0x02
//pTr用于存放读取的DI值。
bool UnCode_0x02(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x02))
        {
            //对接收到的DI值进行转储。值从索引3开始。
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
    //如果从站异常
    else{
        return FALSE;
    }
}

//Function:0x0F
//写多个连续的输出离散量DQ
void MdRTUFunc_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址
    uint16_t  Addr;
    //元件基址 0xxxx
    Addr = DataAddr - 0;

    //发送数据打包为数据帧。
    EnCode_0x0F(MbAddr, Addr, Num, pTr);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);

    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x0F接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x0F(&RX_Struct, MbAddr))
            {
                //test
                printf("F0x0F返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确。
            else{
                //置位完成。写操作不需要转储返回的其他数据。
                Modbus_Status_Struct.bDone = TRUE;                
            }                     
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x0F返回帧CRC16校验失败！\n");
    }
}

//编码
bool EnCode_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    uint16_t CRC16;
    uint16_t j;
    if(Num > 1976) return FALSE;  //元件数量不能大于1976.

    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x0F;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节
    //数据长度:(Num%8)?Num/8+1:Num/8;
    TX_Struct.Buffer[6] = (Num%8)? Num/8+1: Num/8;
    //数据域,长度取决于数量Num.
    for(int i = 0; i < TX_Struct.Buffer[6]; i++)
    {
        TX_Struct.Buffer[7 + i] = *(pTr + i);
    }

    //数据CRC生成
    //j是CRC存放开始单元,低字节在前。
    j = TX_Struct.Buffer[6] + 7;
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, j);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[j] = CRC16;
    TX_Struct.Buffer[j+1] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    return TRUE;
}

//接收数据解析
//pTr用于存放读取的DI值。
bool UnCode_0x0F(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //省略比较返回的其他数据：功能码、元件基址、元件数量。
        //从站号正确
        if(pRX_Struct->Buffer[0] == MbAddr)
            return TRUE;
        else
            return FALSE;
    }
    //如果从站没有返回则会超时.
    //如果从站异常。
    else{
        return FALSE;
    }
}

//Function:0x01
//读取多个连续输出离散量DQ
void MdRTUFunc_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr)
{
    //元件基址:0xxxx
    uint16_t Addr = DataAddr - 00000;

    //发送数据打包为数据帧。
    EnCode_0x01(MbAddr, Addr, Num);
    //发送
    SendFrame(&TX_Struct);
    //发送后转入接收。
    ReceiveFrame(&RX_Struct);
    //应答超时监测使能。
    RespTimeOut_Enb();
    //等待接收：转入接收不一定通讯接口忙，所以还必须或上是否可读。
    while((Modbus_Status_Struct.bBusy || !Modbus_Status_Struct.bFrame_ReadEnb) && !Modbus_Status_Struct.bResponse_TimeOut);
    //test
    if(Modbus_Status_Struct.bResponse_TimeOut)
        printf("F0x01接收超时!\n");

    //接收后解析
    //如果应答超时则不解析,处理下个事务（重发或进行下一帧发送）。
    if(Modbus_Status_Struct.bFrame_ReadEnb && !Modbus_Status_Struct.bResponse_TimeOut)
    {
        //如果无错则对帧进行解码。
        if(!Modbus_Status_Struct.bErr)
        {
            //如果返回数据不符。
            if(!UnCode_0x01(&RX_Struct, MbAddr, pTr))
            {
                //test
                printf("F0x01返回帧含出错信息或不是所需从站返回帧 \n\n\n\n");
                Usart_SendFrame(USART1, RX_Struct.Buffer,RX_Struct.u16Index);
                printf("\n");
            }
            //返回正确，数据已经转储在指定存储区。
            else{
                //置位数据可读取标识。
                Modbus_Status_Struct.bDone = TRUE;
            }
        }
        //如果出错：CRC校验失败。
        else
            printf("F0x01返回帧CRC16校验失败！\n");
    }
}

//编码
bool EnCode_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num)
{
    uint16_t CRC16;

    if(Num > 2000) return FALSE; //待读取的连续元件数量不能超过2000个。
    //发送缓冲区清零
    TX_Struct.u16Index = 0;
    //从站地址
    TX_Struct.Buffer[0] = MbAddr;
    //功能码
    TX_Struct.Buffer[1] = 0x01;
    //元件基址
    TX_Struct.Buffer[2] = DataAddr >> 8;
    TX_Struct.Buffer[3] = DataAddr;
    //元件数量
    TX_Struct.Buffer[4] = Num >> 8; //高字节
    TX_Struct.Buffer[5] = Num;  //低字节

    //数据CRC生成
    //生成的CRC16高字节在前。
    CRC16 = CRC16Gen(TX_Struct.Buffer, 6);
    //添加CRC16,低字节在前
    TX_Struct.Buffer[6] = CRC16;
    TX_Struct.Buffer[7] = CRC16 >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = 8;
    return TRUE;
}

//接收数据解析
bool UnCode_0x01(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr)
{
    //如果接收到的功能码高位不为1，则接收正常。
    if(!(pRX_Struct->Buffer[1] & 0x80))
    {
        //从站号正确且功能码正确
        if((pRX_Struct->Buffer[0] == MbAddr) && (pRX_Struct->Buffer[1] == 0x01))
        {
            //对接收到的DI值进行转储。值从索引3开始。
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
    //如果从站异常
    else{
        return FALSE;
    }
}



