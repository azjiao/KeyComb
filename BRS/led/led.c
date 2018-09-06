/*
 * �����������Դ��led.
 * LED0: ����PB5�˿ڣ���©�����
 * LED1: ����PE5�˿ڣ���©�����
 *
 *
 */
#include "stm32f10x.h"
#include "dev.h"


//����led��ʼ����
void led_Init(void)
{
    //GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��

    //GPIOB��GPIOEʱ��ʹ�ܡ�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	   
    //ʹ�ö��η�װ������ʼ��GPIO.
    My_GPIO_Init(GPIOB, GPIO_Pin_5, GPIO_Mode_Out_OD, GPIO_Speed_10MHz);//����LED0,��©���.
    My_GPIO_Init(GPIOE, GPIO_Pin_5, GPIO_Mode_Out_OD, GPIO_Speed_10MHz);//����LED1,��©���.
    
	//��ʼ��ʹled��Ϩ��
	//�����������led��Ӧ�˿ڳ�ʼ�����Ϊ0���Ʊ�������
	LED0_OFF;
	LED1_OFF;
}

