#ifndef __BEEP_H
#define __BEEP_H
#include "bitBand.h"
/*
 * ���ط�������
 * beep�ӵ�PB8, ���������
 *
 */

//����BEEP�����ͽ����ꡣ
//���˿�Ϊ1ʱʱbeep���ŷŴ�ܵ�ͨ��GND��beep������
#define __BITBAND_BEEP
// ʹ��λ��������������
#ifdef __BITBAND_BEEP
#define BEEP    PBout(8)
#define BEEP_ON  PBout(8) = 1
#define BEEP_OFF  PBout(8) = 0
// ʹ����ͨ�⺯��������������
#else
#define BEEP_ON  GPIO_SetBits( GPIOB, GPIO_Pin_8 )
#define BEEP_OFF  GPIO_ResetBits( GPIOB, GPIO_Pin_8 )
#endif

//BEEP��ʼ��
void beep_Init(void);



#endif /* ifndef __BEEP_H */

