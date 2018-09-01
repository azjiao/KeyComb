/*********************************************************
功能： 位带映射
描述： GPIO端口的位带映射宏定义
设计： azjiao
版本： 1.0
日期： 2018年07月04日
*********************************************************/
#ifndef __BITBAND_H
#define __BITBAND_H
#include <stm32f10x.h>
// 取得位带映射地址值
#define BITBAND(addr,bitnum)  ((addr & 0xF0000000) + 0x2000000 + ((addr &0xFFFFF) << 5)+(bitnum << 2))
// 把地址值转换为指向unsigned long 的指针,并定义为指针所指地址的内容。
#define MEM_ADDR(addr)  *((volatile unsigned long *)(addr))
// 定义位操作宏
#define BIT_ADDR(addr,bitnum)  MEM_ADDR(BITBAND(addr,bitnum))

// IO端口地址宏定义:为GPIO的ODR和IDR寄存器的地址映射。
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


// IO端口独立一位读写操作宏定义
// n值范围为：n<=15且n>=0
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
