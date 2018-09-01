/*
 * ���ط�������
 * beep�ӵ�PB8, ���������
 * ���˿�Ϊ1ʱʱbeep���ŷŴ�ܵ�ͨ��GND��beep������
 *
 */

#include "stm32f10x.h"
#include "beep.h"

//BEEP��ʼ��
void beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��

    //GPIOB ʱ��ʹ�ܡ�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

    //BEEP �˿�����.
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; //���������
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; //����ٶ�10MHz.
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //��ʼ��BEEP ������
    BEEP_OFF;

}
