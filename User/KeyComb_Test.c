/*********************************************************
  ���ܣ� ��ϼ�����
  ������ ��Ӣ��������������ϲ��ԣ����Ե�����������ʹ�ã�Ҳ�������ʹ�á�
  ��ƣ� azjiao
  �汾�� 0.1
  ���ڣ� 2018��08��19��
 *********************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f10x.h"
#include "template.h"
#include "dev.h"

void assert_failed(uint8_t* file, uint32_t line)
{
    //printf("Param Error! in file name: xxx, in line %d\n",line);
    //while(1);
}

// ����������ʱ����
static TimerType Timer1, Timer2;

int main(void)
{
    //bool bSwitch = TRUE;
    bool bT1_Enb = FALSE, bT2_Enb = FALSE;  //��ʱ��ʹ�ܡ�
    bool bLED0Round = FALSE; // LED0 ��ʱ���𿪹ء�
    uint32_t KeyVal;
    //����Key��ʼ����
    key_Init();

    //����Led��ʼ����
    led_Init();

    //����beep��ʼ����
    beep_Init();

    // SysTick��ʼ������ʼ��ʱ��
    delay_Init();    
    
    Usart1_Init(19200);

    /* Infinite loop */
    while (1)
    {
        bool bIsKey0;
        bool bIsKey1;
        bool bIsWKUP;
        
        keypadScan();
        bIsKey0 = Keypad.u16ValOnce == KEY1_KEY0;  //��ϣ���KEY1��KEY0
        //bIsKey1 = Keypad.u16ValSus == KEY0_KEY1;  //��ϣ�KEY0+KEY1,��˳��Ҫ��
        //����Key0+Wkup����Wkup���¡�
        bIsWKUP = Keypad.u16ValOnce == KEY0_WKUP;
        //bIsKey0 = key0_Scan(TRUE);
        bIsKey1 = key1_Scan(TRUE);  //ʹ�õ�����ɨ�衣
        
        //���Լ��̻�����
        KeyVal = KeyBufR(TRUE, &KeyBuf);
        if(KeyVal != 0x0000)        
            printf("��⵽���̻����������ݣ�%d\r\n", KeyVal);
        
        KeyVal = KeyBufR(FALSE, &KeyBufRelease);
        if(KeyVal>>16 >=200U)
        {
            printf("��⵽�����ͷţ���ֵ%d����ʱ��Ϊ��%dms\r\n", KeyVal & 0x00FF, KeyVal>>16);
            KeyVal = KeyBufR(TRUE, &KeyBufRelease);
        }
        
        //bIsWKUP = WKUP_Scan(FALSE);
        if(bIsWKUP)
            bLED0Round = !bLED0Round;

        if (bLED0Round)
            LED1_ON;        
        else
            LED1_OFF;

        //BEEP������������
        if( bIsKey0)  
        {  
            BEEP = !BEEP;
        }           
        
        if(bIsKey1)
        {
            LED0_ON;
        }
        else 
            LED0_OFF;

        TimeON(bT1_Enb, 500U, &Timer1);
        TimeON(bT2_Enb, 1000U, &Timer2);

        if(bLED0Round){
            if(!Timer1.bQ)
            {
                bT1_Enb = TRUE; //Timer1��ʼ��ʱ��
                LED0_ON;
            }
            if(Timer1.bQ && !Timer2.bQ){
                bT2_Enb = TRUE;
                LED0_OFF;
            }
            // ��Timer2��ʱ��ʱ��λ������ʱ����
            if(Timer2.bQ){
                bT1_Enb = FALSE;
                bT2_Enb = FALSE;
            }
        }
        else {
            bT1_Enb = FALSE;
            bT2_Enb = FALSE;
            LED0_OFF;
        }
    }
}





