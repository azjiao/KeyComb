/*********************************************************
功能： Modbus-RTU从站通讯
描述： 实现了从站通讯的几个功能码
设计： azjiao
版本： 0.1
日期： 2018年07月25日
*********************************************************/
#ifndef __MODSLAVE_H
#define __MODSLAVE_H

#include "template.h"
#include "modRTU.h"

extern ModRTU_Buffer Data_Struct; //位于modRTU.c
extern ModRTU_RX_Struct RX_Struct;
extern ModRTU_TX_Struct TX_Struct;

//从站地址号
#define MBSLAVE_ADDR  3
//定义资源每类占用的最大字节。
#define DATA_MAXLEN   256
//定义资源地址和通讯元件地址的对应关系。
//如果定义了ADDR_BASE1为FALSE，则对应关系是0---->0,如果不是则对应关系是0---->1.(通讯元件地址0 无效了)
//通讯的元件地址基于1，这是西门子PLC产品的特点，同样，ABB变频器也如此.
//但是，这是主站请求处理的事情。主站如果地址40001，则主站Modbus-RTU实际发出的是40000。
//对于从站来说只需按照请求的地址来处理。
//如果有特殊对应要求则需要修改程序，比如需要偏移地址以便记忆。
#define ADDR_BASE1  FALSE

//定义供通讯的服务器资源类型
//对于一个可编程的从站来说，资源的尺寸是可以调节配置的，但对于一个具体的从站来说资源尺寸是一定的。所以，元件数量实无必要，可以不用struct来定义。
//内部位或物理coil,0xxxx
//定义为以0为基址的8bit数组。
typedef struct {
    uint8_t u8Data[DATA_MAXLEN];
    uint16_t u16Num;   //元件数量，8个占用一个字节。
}Type_0xxxx;

//输入离散量DI,1xxxx
//定义为以0为基址的8bit数组。
typedef struct  {
    uint8_t u8Data[DATA_MAXLEN];
    uint16_t u16Num;  //元件数量，8个占用一个字节。
}Type_1xxxx;

//输入存储器AI,3xxxx
//定义为以0为基址的16bit数组。
typedef struct  {
    uint16_t u16Data[DATA_MAXLEN];
    uint16_t u16Num;  //元件数量，对于16位资源来说最大为DATA_MAXLEN/2个;
}Type_3xxxx;

//保持寄存器HoldReg,4xxxx
//定义为以0为基址的16bit数组。
typedef struct  {
    uint16_t u16Data[DATA_MAXLEN];
    uint16_t u16Num;  //元件数量，对于16位资源来说最大为DATA_MAXLEN/2个;
}Type_4xxxx;

//定义服务资源
extern Type_0xxxx    DQ_0xxxx;
extern Type_1xxxx    DI_1xxxx;
extern Type_3xxxx    AI_3xxxx;
extern Type_4xxxx    HR_4xxxx;

//从站服务函数
void Modbus_Slave(void);

//Function:0x01
//读取多个连续的DQ，0xxxx
void SlaveFunc_0x01(void);

//Function:0x0F
//强制多个连续的DQ，0xxxx
void SlaveFunc_0x0F(void);

//Function:0x02
//读取多个连续的输入离散量DI,1xxxx
void SlaveFunc_0x02(void);

//Function:0x04
//读取多个连续的输入寄存器AI,3xxxx
void SlaveFunc_0x04(void);

//Function:0x03
//读取多个连续的保持寄存器HoldReg,4xxxx
void SlaveFunc_0x03(void);

//Function:0x10
//写多个连续的保持寄存器HoldReg,4xxxx
void SlaveFunc_0x10(void);

//异常01,请求的功能不支持。
void Default_NonSupport(void);

#endif /* end of include guard: __MODSLAVE_H */

