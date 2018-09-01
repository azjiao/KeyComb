/*
*Modbus-RTU��վͨѶ
*���:azjiao
*�汾:0.1
*���ڣ�2018-07-25
*/
#ifndef __MODMASTER_H
#define __MODMASTER_H

#include <stm32f10x.h>
#include "modRTU.h"

extern ModRTU_Buffer Data_Struct; //λ��modRTU.c
extern ModRTU_RX_Struct RX_Struct;
extern ModRTU_TX_Struct TX_Struct;

//Modbus��վͨѶ����
//Mb_Addr:��վ��ַ
//bMode:ģʽ0����1д
//Data_Addr:��վ���ݵ�ַ
//Num:����byte����,�����Ƕ�ȡ��Ԫ��������Ҳ������д���Ԫ��������
//Ptr:�����͵����ݻ�������������ݵĻ�������
void Modbus_Master(uint8_t Mb_Addr, bool bMode, uint16_t Data_Addr, uint16_t Num, u8 * Ptr);

/*-----------------------------------------------------------*/
/*16λ��*/

//Function:0x10
//д��������ı��ּĴ�����
void MdRTUFunc_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x10(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//�������ݽ���
bool UnCode_0x10(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr);

//Function:0x03
//����������ı��ּĴ�����
void MdRTUFunc_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x03(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//�������ݽ���
//pTr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool UnCode_0x03(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

//Function:0x04
//��AI:����洢��
void MdRTUFunc_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x04(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//�������ݽ���
//pTr���ڴ�Ŷ�ȡ��HoldRegֵ��
bool UnCode_0x04(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

/*--------------------------------------------------------------------*/
/*��ɢ��*/

//Function:0x02
//�����������������ɢ��DI
void MdRTUFunc_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x02(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//�������ݽ���
//pTr���ڴ�Ŷ�ȡ��DIֵ��
bool UnCode_0x02(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 

//Function:0x0F
//д��������������ɢ��DQ
void MdRTUFunc_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x0F(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//�������ݽ���
bool UnCode_0x0F(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr); 

//Function:0x01
//��ȡ������������ɢ��DQ
void MdRTUFunc_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num, uint8_t* pTr);
//����
bool EnCode_0x01(uint8_t MbAddr, uint16_t DataAddr, uint16_t Num);
//�������ݽ���
bool UnCode_0x01(ModRTU_RX_Struct* pRX_Struct, uint8_t MbAddr, uint8_t* pTr); 
#endif


