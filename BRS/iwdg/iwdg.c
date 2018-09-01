/*********************************************************
���ܣ� �������Ź�
������ ������Դ�������Ź�
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��08��04��
*********************************************************/
#include "iwdg.h"

//��ʼ���������Ź�
//u8Prer:Ԥ��Ƶϵ��0��7,u8Prer��Ӧ��ʵ��Ԥ��ƵֵΪ4*2^u8Prer,���Ԥ��Ƶֵ��256.
//u16Rlr:��ת�ؼĴ���ֵ��ֻ�е�12λ��Ч�����0xFFF,��4095.
//��Ӣ��ĵ����ڲ�ʱ��LSI��40KHz,��һ�������ʱʱ��Ϊ��(4*2^u8Prer)/40 ms.
//���迴�Ź���ʱΪ��Tout(ms) = (4*2^u8Prer)/40 *u16Rlr.
void Iwdg_Init(uint8_t u8Prer, uint16_t u16Rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ȥ��PR��RLR��д����,��������
    IWDG_SetPrescaler(u8Prer); //����Ԥ��Ƶϵ����
    IWDG_SetReload(u16Rlr);  //������ת�ؼĴ���ֵ��
    IWDG_ReloadCounter();   //������ת�ؼĴ���ֵ��IWDG��������
    IWDG_Enable();   //ʹ��IWDG����ʼ������
}

void Iwdg_Feed(void)
{
    IWDG_ReloadCounter();  //��װ��RLRֵ��IWDG��������
}
