#ifndef __LED_H
#define __LED_H
#include "bitBand.h"

//����LED0��LED1�ĵ�����Ϩ��ꡣ
//����LED0��LED1�ǿ�©��������0���ܵ����������1Ϩ��
#define __BITBANDLED
// ʹ��λ������LED��
#ifdef  __BITBANDLED
#define LED0 PBout(5)
#define LED1 PEout(5)

#define LED0_ON  LED0 = 0
#define LED0_OFF LED0 = 1
#define LED1_ON  LED1 = 0
#define LED1_OFF LED1 = 1


//ʹ����ͨ�⺯��������LED��
#else
#define LED0_ON GPIO_ResetBits( GPIOB, GPIO_Pin_5 )
#define LED0_OFF GPIO_SetBits( GPIOB, GPIO_Pin_5 )
#define LED1_ON GPIO_ResetBits( GPIOE, GPIO_Pin_5 )
#define LED1_OFF GPIO_SetBits( GPIOE, GPIO_Pin_5 )
#endif

//����led��ʼ����
void led_Init(void);


#endif /* ifndef __LED_H */
