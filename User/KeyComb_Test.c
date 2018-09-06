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
    uint32_t KeyStatusTemp = 0x0; //��ֵ�ݴ棬��ʼ��Ϊ���̼�ֵΪ0��
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
               
        //���Լ��̻�����
        KeyVal = KeyBufR(TRUE, &KeyBuf);
        if(Keypad.u32KeyStatus ^ KeyStatusTemp)
            printf("ss~ss��ֵ״̬�仯��0x%x--0x%x\r\n", Keypad.u32KeyStatus, KeyStatusTemp);
        if(KeyVal != 0x0000)        
            printf("|���̻����������ݣ�0x%x\r\n", KeyVal);
        if((KeyVal & (0xF<<16)) == (uint32_t)(LONGPR_STATUS<<16))
            printf("---�����¼���0x%x\r\n", KeyVal);
        if((KeyVal & (0xF<<16)) == (uint32_t)(DOUBLECLICK_EVENT<<16))
            printf("^^˫���¼���0x%x\r\n", KeyVal);
        
        KeyStatusTemp = Keypad.u32KeyStatus;
       
        bIsKey0 = (Keypad.u16ValOnce == KEY1_KEY0) || (KeyVal == KEY1_DOUBLE);  //��ϣ���KEY1��KEY0
        //bIsKey1 = Keypad.u16ValSus == KEY0_KEY1;  //��ϣ�KEY0+KEY1,��˳��Ҫ��
        //����Key0+Wkup����Wkup���¡�
        bIsWKUP = (Keypad.u16ValOnce == KEY0_WKUP) || (KeyVal == KEY0_DOUBLE);
        //bIsKey0 = key0_Scan(TRUE);
        bIsKey1 = key1_Scan(TRUE);  //ʹ�õ�����ɨ�衣
        
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





