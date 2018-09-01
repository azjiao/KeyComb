/*
*Modbus-RTU主站通讯
*设计:azjiao
*版本:0.1
*日期：2018-07-25
*/
#ifndef __MODMASTER_H
#define __MODMASTER_H

#include <stm32f10x.h>
#include "modRTU.h"

extern ModRTU_Buffer Data_Struct; //位于modRTU.c
extern ModRTU_RX_Struct RX_Struct;
extern ModRTU_TX_Struct TX_Struct;

//Modbus主站通讯控制
//Mb_Addr:从站地址
//bMode:模式0读或1写
//Data_Addr:从站数据地址
//Num:数据byte长度,可能是读取的元件数量，也可能是写入的元件数量。
//Ptr:待发送的数据缓冲区或接收数据的缓冲区。
void Modbus_Master(uint8_t Mb_Addr, bool bMode, uint16_t Data_Addr, uint16_t Num, u8 * Ptr);

/*-----------------------------------------------------------*/
/*16位量*/

//Function:0x10
//写多个连续的保持寄存器。
void MdRTUFunc_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//接收数据解析
bool UnCode_0x10(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr);

//Function:0x03
//读多个连续的保持寄存器。
void MdRTUFunc_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//接收数据解析
//pTr用于存放读取的HoldReg值。
bool UnCode_0x03(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

//Function:0x04
//读AI:输入存储器
void MdRTUFunc_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//接收数据解析
//pTr用于存放读取的HoldReg值。
bool UnCode_0x04(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

/*--------------------------------------------------------------------*/
/*离散量*/

//Function:0x02
//读多个连续的输入离散量DI
void MdRTUFunc_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//接收数据解析
//pTr用于存放读取的DI值。
bool UnCode_0x02(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

//Function:0x0F
//写多个连续的输出离散量DQ
void MdRTUFunc_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//接收数据解析
bool UnCode_0x0F(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr); 

//Function:0x01
//读取多个连续输出离散量DQ
void MdRTUFunc_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//编码
bool EnCode_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//接收数据解析
bool UnCode_0x01(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 
#endif


