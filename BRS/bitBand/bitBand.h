/*********************************************************
���ܣ� λ��ӳ��
������ GPIO�˿ڵ�λ��ӳ��궨��
��ƣ� azjiao
�汾�� 1.0
���ڣ� 2018��07��04��
*********************************************************/
#ifndef __BITBAND_H
#define __BITBAND_H
#include <stm32f10x.h>
// ȡ��λ��ӳ���ֵַ
#define BITBAND(addr,bitnum)  ((addr & 0xF0000000) + 0x2000000 + ((addr &0xFFFFF) << 5)+(bitnum << 2))
// �ѵ�ֵַת��Ϊָ��unsigned long ��ָ��,������Ϊָ����ָ��ַ�����ݡ�
#define MEM_ADDR(addr)  *((volatile unsigned long *)(addr))
// ����λ������
#define BIT_ADDR(addr,bitnum)  MEM_ADDR(BITBAND(addr,bitnum))

// IO�˿ڵ�ַ�궨��:ΪGPIO��ODR��IDR�Ĵ����ĵ�ַӳ�䡣
#define GPIOA_ODR_ADDR  (GPIOA_BASE + 0xc)
#define GPIOB_ODR_ADDR  (GPIOB_BASE + 0xc)
#define GPIOC_ODR_ADDR  (GPIOC_BASE + 0xc)
#define GPIOD_ODR_ADDR  (GPIOD_BASE + 0xc)
#define GPIOE_ODR_ADDR  (GPIOE_BASE + 0xc)
#define GPIOF_ODR_ADDR  (GPIOF_BASE + 0xc)
#define GPIOG_ODR_ADDR  (GPIOG_BASE + 0xc)

#define GPIOA_IDR_ADDR  (GPIOA_BASE + 0x8)
#define GPIOB_IDR_ADDR  (GPIOB_BASE + 0x8)
#define GPIOC_IDR_ADDR  (GPIOC_BASE + 0x8)
#define GPIOD_IDR_ADDR  (GPIOD_BASE + 0x8)
#define GPIOE_IDR_ADDR  (GPIOE_BASE + 0x8)
#define GPIOF_IDR_ADDR  (GPIOF_BASE + 0x8)
#define GPIOG_IDR_ADDR  (GPIOG_BASE + 0x8)


// IO�˿ڶ���һλ��д�����궨��
// nֵ��ΧΪ��n<=15��n>=0
#define PAout(n)  BIT_ADDR(GPIOA_ODR_ADDR,n)
#define PAin(n)   BIT_ADDR(GPIOA_IDR_ADDR,n)

#define PBout(n)  BIT_ADDR(GPIOB_ODR_ADDR,n)
#define PBin(n)   BIT_ADDR(GPIOB_IDR_ADDR,n)

#define PCout(n)  BIT_ADDR(GPIOC_ODR_ADDR,n)
#define PCin(n)   BIT_ADDR(GPIOC_IDR_ADDR,n)

#define PDout(n)  BIT_ADDR(GPIOD_ODR_ADDR,n)
#define PDin(n)   BIT_ADDR(GPIOD_IDR_ADDR,n)

#define PEout(n)  BIT_ADDR(GPIOE_ODR_ADDR,n)
#define PEin(n)   BIT_ADDR(GPIOE_IDR_ADDR,n)

#define PFout(n)  BIT_ADDR(GPIOF_ODR_ADDR,n)
#define PFin(n)   BIT_ADDR(GPIOF_IDR_ADDR,n)

#define PGout(n)  BIT_ADDR(GPIOG_ODR_ADDR,n)
#define PGin(n)   BIT_ADDR(GPIOG_IDR_ADDR,n)

#endif /* end of include guard: __BITBAND_H */
