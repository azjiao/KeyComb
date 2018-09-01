/*********************************************************
功能： Modbus-RTU从站通讯
描述： 实现了从站通讯的几个功能码
设计： azjiao
版本： 0.1
日期： 2018年07月25日
*********************************************************/
#include <stdio.h>
#include "dev.h"
#include "modSlave.h"
#include "modRTU.h"
#include "usart1.h"

//定义服务资源
Type_0xxxx    DQ_0xxxx;
Type_1xxxx    DI_1xxxx;
Type_3xxxx    AI_3xxxx;
Type_4xxxx    HR_4xxxx;

//从站服务函数
//Modbus初始化时485口为接收状态。
void Modbus_Slave(void)
{
    //如果接收到了数据帧(数据帧可读时端口为发送状态，防止接收到无关数据)。
    if(Modbus_Status_Struct.bFrame_ReadEnb)
    {
        //如果无错则解析
        //当帧可读时其实bErr为FALSE，可以取消判断。
        if(!Modbus_Status_Struct.bErr)
        {
            //首先判断从站地址是否相符。
            //站地址不符则放弃,重启接收。
            if(RX_Struct.Buffer[0] != MBSLAVE_ADDR){                
                ReceiveFrame(&RX_Struct);
                return;
            }

            //提取功能码做判断
            switch(RX_Struct.Buffer[1]){
                case 0x01:SlaveFunc_0x01();  //读多个DQ_0xxxx
                          break;
                case 0x0F:SlaveFunc_0x0F();  //写多个DQ_0xxxx
                          break;
                case 0x02:SlaveFunc_0x02();  //读多个DI_1xxxx
                          break;
                case 0x04:SlaveFunc_0x04();  //读多个AI_3xxxx
                          break;
                case 0x03:SlaveFunc_0x03();  //读多个HoldReg_4xxxx
                          break;
                case 0x10:SlaveFunc_0x10();  //写多个HoldReg_4xxxx
                          break;
                default:  Default_NonSupport();  //不支持的功能处理。

            }
            //结束后转入接收。
            ReceiveFrame(&RX_Struct);
        }
    }
}

//Function:0x01
//读取多个连续的DQ，0xxxx
//执行函数时从站地址已经相符。
void SlaveFunc_0x01(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x01;

    //判断元件数量是否合理。
    //不能超过2000(0x07D0)个DQ请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= (DATA_MAXLEN * 8))
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            TX_Struct.Buffer[2] = u8ByteNum;

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ字节开始索引。
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ字节位索引初始值。
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ字节初始掩码.
            uint16_t u16Tx_Index = 0;  //发送字节索引初始值。
            uint8_t u8TxBit_Index = 0; //发送字节位索引初始值.
            uint8_t u8TxMask = 0x01;  //TX字节初始掩码。

            //把需要响应的数据值赋值给TX_Struct.Buffer[3]开始的单元。
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
            //最后一个TX数据字节高位填充0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            j = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] |= 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);
}

//Function:0x0F
//强制多个连续的DQ，0xxxx
void SlaveFunc_0x0F(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。
    uint8_t u8ByteNum;  //所需字节数量
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //计算所需的字节量。虽然接收帧里面含有字节量(第6字节)，但仍然要比较是否正确。
    u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x0F;

    //判断元件数量是否合理。
    //不能超过2000个DQ请求且请求的元件量和传入的字节量匹配。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0) && (u8ByteNum == RX_Struct.Buffer[6]))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN * 8)
        {
            //打包数据帧
            //元件基址
            TX_Struct.Buffer[2] = RX_Struct.Buffer[2];
            TX_Struct.Buffer[3] = RX_Struct.Buffer[3];
            //响应的DQ元件数量.
            TX_Struct.Buffer[4] = RX_Struct.Buffer[4];
            TX_Struct.Buffer[5] = RX_Struct.Buffer[5];

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DQ_Index = u16ByteIndex;  //DQ字节开始索引。
            uint8_t u8DQBit_Index = u8BitIndex;  //DQ字节位索引初始值。
            uint8_t u8DQMask = 0x01 << u8DQBit_Index;  //DQ字节初始掩码.
            uint16_t u16Rx_Index = 7;  //接收字节索引初始值。
            uint8_t u8RxBit_Index = 0; //接收字节位索引初始值.
            uint8_t u8RxMask = 0x01;  //接收RX字节初始掩码。

            //把需要强制的数据值(从第7字节开始)赋值给DQ_0xxxx相应的单元。
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

            j = 6; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] |= 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);
}

//Function:0x02
//读取多个连续的输入离散量DI,1xxxx
void SlaveFunc_0x02(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。
    uint16_t u16ByteIndex;  //基址所在字节索引，从0开始的字节索引。
    uint8_t u8BitIndex;  //基址所在字节的开始位索引，从0开始的位索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x02;

    //判断元件数量是否合理。
    //不能超过2000(0x07D0)个DQ请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x07D0))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN * 8)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量
            u8ByteNum = (u16Num%8)? (u16Num/8+1):u16Num/8;
            TX_Struct.Buffer[2] = u8ByteNum;

            //定位u16DataAddr所在字节索引。
            u16ByteIndex = u16DataAddr/8;
            //定位所在字节的开始位索引。
            u8BitIndex = u16DataAddr%8;

            uint16_t u16DI_Index = u16ByteIndex;  //DI字节开始索引。
            uint8_t u8DIBit_Index = u8BitIndex;  //DI字节位索引初始值。
            uint8_t u8DIMask = 0x01 << u8DIBit_Index;  //DI字节初始掩码.
            uint16_t u16Tx_Index = 0;  //发送字节索引初始值。
            uint8_t u8TxBit_Index = 0; //发送字节位索引初始值.
            uint8_t u8TxMask = 0x01;  //TX字节初始掩码。

            //把需要响应的数据值赋值给TX_Struct.Buffer[3]开始的单元。
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
            //最后一个TX数据字节高位填充0
            for(; u8TxBit_Index < 8; u8TxBit_Index++)
            {
                TX_Struct.Buffer[u16Tx_Index + 3] &= (u8TxMask ^ 0xFF);
                u8TxMask <<= 1;
            }

            j = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] |= 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);
}

//Function:0x04
//读取多个连续的输入寄存器AI,3xxxx
void SlaveFunc_0x04(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前.
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x04;

    //判断元件数量是否合理。
    //不能超过125个AI请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        //提供的AI_3xxxx是16bit数组，所以一个单元是一个元件数据。
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量:元件数量*2.
            //可以使用u16Num<<1,因为不能超过125，所以此处u16Num高8位是0.
            u8ByteNum = RX_Struct.Buffer[5] << 1;
            TX_Struct.Buffer[2] = u8ByteNum;
            //把需要响应的数据值赋值给TX_Struct.Buffer[3]开始的单元。
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16位AI资源数据分拆：高字节在前。
                TX_Struct.Buffer[3 + i] = AI_3xxxx.u16Data[i/2 + u16DataAddr] >> 8; //16位数据高8位字节。
                TX_Struct.Buffer[3 + i + 1] = AI_3xxxx.u16Data[i/2 + u16DataAddr];  //16位数据低8位字节。
            }

            j = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);

}

//Function:0x03
//读取多个连续的保持寄存器HoldReg,4xxxx
void SlaveFunc_0x03(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x03;

    //判断元件数量是否合理。
    //不能超过125个HR请求。
    if((u16Num >= 0x0001) && (u16Num <= 0x007D))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        //提供的AI_3xxxx是16bit数组，所以一个单元是一个元件数据。
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            uint8_t u8ByteNum;  //所需字节数量

            //打包数据帧
            //响应的数据字节数量:元件数量*2.
            //可以使用u16Num<<1,因为不能超过125，所以此处u16Num高8位是0.
            u8ByteNum = RX_Struct.Buffer[5] << 1;
            TX_Struct.Buffer[2] = u8ByteNum;
            //把需要响应的数据值赋值给TX_Struct.Buffer[3]开始的单元。
            for(uint16_t i = 0; i < u8ByteNum; i += 2)
            {
                //16位AI资源数据分拆：高字节在前。
                TX_Struct.Buffer[3 + i] = HR_4xxxx.u16Data[i/2 + u16DataAddr] >> 8; //16位数据高8位字节。
                TX_Struct.Buffer[3 + i + 1] = HR_4xxxx.u16Data[i/2 + u16DataAddr];  //16位数据低8位字节。
            }

            j = u8ByteNum + 3; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);
}

//Function:0x10
//写多个连续的保持寄存器HoldReg,4xxxx
void SlaveFunc_0x10(void)
{
    uint16_t u16Num;  //元件数量
    uint16_t u16DataAddr;  //元件基址
    uint16_t u16CRC;
    uint16_t j; //CRC装载单元索引。
    uint8_t u8ByteNum;  //所需字节数量

    //获取元件基址及数量。
    //元件基址在第2、3字节，高字节在前。
    u16DataAddr = (((uint16_t)RX_Struct.Buffer[2]) << 8) | RX_Struct.Buffer[3];
    //数量在第4、5字节,高字节在前
    u16Num = (((uint16_t)RX_Struct.Buffer[4]) << 8) | RX_Struct.Buffer[5];

    //计算所需的字节量。虽然接收帧里面含有字节量(第6字节)，但仍然要比较是否正确。
    u8ByteNum = u16Num << 1;

    //取得实际元件基址
    //如果元件地址基于1但传入的元件基址为0，则减一成为最大值0xFFFF.
    if(ADDR_BASE1)
        u16DataAddr--;

    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = 0x10;

    //判断元件数量是否合理。
    //不能超过123个HR请求且请求的元件量和传入的字节量匹配。
    if((u16Num >= 0x0001) && (u16Num <= 0x007B) && (u8ByteNum == RX_Struct.Buffer[6]))
    {
        //判断元件基址和数量是否合适。
        //从站单元地址从0开始。(可以修改为1开始，可视情况定。)
        if((u16DataAddr + u16Num) <= DATA_MAXLEN)
        {
            //打包数据帧
            //元件基址
            TX_Struct.Buffer[2] = RX_Struct.Buffer[2];
            TX_Struct.Buffer[3] = RX_Struct.Buffer[3];
            //响应的HR元件数量.
            TX_Struct.Buffer[4] = RX_Struct.Buffer[4];
            TX_Struct.Buffer[5] = RX_Struct.Buffer[5];

            //把赋值的数据(从第7字节开始)赋值给HR_4xxxx相应的单元。
            for(uint16_t i = 0; i < u8ByteNum; i+=2)
            {
                HR_4xxxx.u16Data[i/2 + u16DataAddr] = RX_Struct.Buffer[7 + i]; //高字节
                HR_4xxxx.u16Data[i/2 + u16DataAddr] <<= 8;
                HR_4xxxx.u16Data[i/2 + u16DataAddr] |= RX_Struct.Buffer[7 + i + 1]; //低字节
            }

            j = 6; //j为CRC所在单元。

        }
        else{
            //产生异常02：地址非法
            TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
            TX_Struct.Buffer[2] = 0x02;
            j = 3;
        }
    }
    else{
        //产生异常03：数据非法
        TX_Struct.Buffer[1] = TX_Struct.Buffer[1] + 0x80;
        TX_Struct.Buffer[2] = 0x03;
        j = 3;
    }

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, j);
    TX_Struct.Buffer[j] = u16CRC;
    TX_Struct.Buffer[j+1] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = j + 2;
    //发送打包后的帧
    SendFrame(&TX_Struct);
}

//异常01,请求的功能不支持。
void Default_NonSupport(void)
{
    uint16_t u16CRC;
    //装配数据:
    //从站地址
    TX_Struct.Buffer[0] = MBSLAVE_ADDR;
    //功能码
    TX_Struct.Buffer[1] = RX_Struct.Buffer[1] + 0x80;
    //异常码01
    TX_Struct.Buffer[2] = 0x01;

    //生成CRC16。
    u16CRC = CRC16Gen(TX_Struct.Buffer, 3);
    TX_Struct.Buffer[3] = u16CRC;
    TX_Struct.Buffer[4] = u16CRC >> 8;
    //帧长度字节数。
    TX_Struct.u16Index = 5;
    //发送打包后的帧

    SendFrame(&TX_Struct);
    printf("This functiosn nonSupport!---%d\n",TX_Struct.Buffer[1]);
}
