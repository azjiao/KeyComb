/*********************************************************
���ܣ� Modbus-RTU��վͨѶ
������ ʵ���˴�վͨѶ�ļ���������
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��07��25��
*********************************************************/
#ifndef __MODSLAVE_H
#define __MODSLAVE_H

#include "template.h"
#include "modRTU.h"

extern ModRTU_Buffer Data_Struct; //λ��modRTU.c
extern ModRTU_RX_Struct RX_Struct;
extern ModRTU_TX_Struct TX_Struct;

//��վ��ַ��
#define MBSLAVE_ADDR  3
//������Դÿ��ռ�õ�����ֽڡ�
#define DATA_MAXLEN   256
//������Դ��ַ��ͨѶԪ����ַ�Ķ�Ӧ��ϵ��
//���������ADDR_BASE1ΪFALSE�����Ӧ��ϵ��0---->0,����������Ӧ��ϵ��0---->1.(ͨѶԪ����ַ0 ��Ч��)
//ͨѶ��Ԫ����ַ����1������������PLC��Ʒ���ص㣬ͬ����ABB��Ƶ��Ҳ���.
//���ǣ�������վ����������顣��վ�����ַ40001������վModbus-RTUʵ�ʷ�������40000��
//���ڴ�վ��˵ֻ�谴������ĵ�ַ������
//����������ӦҪ������Ҫ�޸ĳ��򣬱�����Ҫƫ�Ƶ�ַ�Ա���䡣
#define ADDR_BASE1  FALSE

//���幩ͨѶ�ķ�������Դ����
//����һ���ɱ�̵Ĵ�վ��˵����Դ�ĳߴ��ǿ��Ե������õģ�������һ������Ĵ�վ��˵��Դ�ߴ���һ���ġ����ԣ�Ԫ������ʵ�ޱ�Ҫ�����Բ���struct�����塣
//�ڲ�λ������coil,0xxxx
//����Ϊ��0Ϊ��ַ��8bit���顣
typedef struct {
    uint8_t u8Data[DATA_MAXLEN];
    uint16_t u16Num;   //Ԫ��������8��ռ��һ���ֽڡ�
}Type_0xxxx;

//������ɢ��DI,1xxxx
//����Ϊ��0Ϊ��ַ��8bit���顣
typedef struct  {
    uint8_t u8Data[DATA_MAXLEN];
    uint16_t u16Num;  //Ԫ��������8��ռ��һ���ֽڡ�
}Type_1xxxx;

//����洢��AI,3xxxx
//����Ϊ��0Ϊ��ַ��16bit���顣
typedef struct  {
    uint16_t u16Data[DATA_MAXLEN];
    uint16_t u16Num;  //Ԫ������������16λ��Դ��˵���ΪDATA_MAXLEN/2��;
}Type_3xxxx;

//���ּĴ���HoldReg,4xxxx
//����Ϊ��0Ϊ��ַ��16bit���顣
typedef struct  {
    uint16_t u16Data[DATA_MAXLEN];
    uint16_t u16Num;  //Ԫ������������16λ��Դ��˵���ΪDATA_MAXLEN/2��;
}Type_4xxxx;

//���������Դ
extern Type_0xxxx    DQ_0xxxx;
extern Type_1xxxx    DI_1xxxx;
extern Type_3xxxx    AI_3xxxx;
extern Type_4xxxx    HR_4xxxx;

//��վ������
void Modbus_Slave(void);

//Function:0x01
//��ȡ���������DQ��0xxxx
void SlaveFunc_0x01(void);

//Function:0x0F
//ǿ�ƶ��������DQ��0xxxx
void SlaveFunc_0x0F(void);

//Function:0x02
//��ȡ���������������ɢ��DI,1xxxx
void SlaveFunc_0x02(void);

//Function:0x04
//��ȡ�������������Ĵ���AI,3xxxx
void SlaveFunc_0x04(void);

//Function:0x03
//��ȡ��������ı��ּĴ���HoldReg,4xxxx
void SlaveFunc_0x03(void);

//Function:0x10
//д��������ı��ּĴ���HoldReg,4xxxx
void SlaveFunc_0x10(void);

//�쳣01,����Ĺ��ܲ�֧�֡�
void Default_NonSupport(void);

#endif /* end of include guard: __MODSLAVE_H */

