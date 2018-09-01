/*
 * �����������Դ��led.
 * LED0: ����PB5�˿ڣ���©�����
 * LED1: ����PE5�˿ڣ���©�����
 *
 *
 */
#include "stm32f10x.h"
#include "led.h"


//����led��ʼ����
void led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��

    //GPIOB��GPIOEʱ��ʹ�ܡ�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    //����LED0��
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode =GPIO_Mode_Out_OD;   //��©���
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; //����ٶ�10MHz.
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //����LED1.���˶˿��鲻һ���⣬����һ����
    GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	//��ʼ��ʹled��Ϩ��
	//�����������led��Ӧ�˿ڳ�ʼ�����Ϊ0���Ʊ�������
	LED0_OFF;
	LED1_OFF;
}

