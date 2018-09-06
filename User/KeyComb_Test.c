/*********************************************************
  功能： 组合键测试
  描述： 精英版三个按键的组合测试，可以单个按键独立使用，也可以组合使用。
  设计： azjiao
  版本： 0.1
  日期： 2018年08月19日
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

// 定义两个定时器。
static TimerType Timer1, Timer2;

int main(void)
{
    //bool bSwitch = TRUE;
    bool bT1_Enb = FALSE, bT2_Enb = FALSE;  //定时器使能。
    bool bLED0Round = FALSE; // LED0 定时亮灭开关。
    uint32_t KeyVal;
    uint32_t KeyStatusTemp = 0x0; //键值暂存，初始化为键盘键值为0。
    //板载Key初始化。
    key_Init();

    //板载Led初始化。
    led_Init();

    //板载beep初始化。
    beep_Init();

    // SysTick初始化，开始定时。
    delay_Init();    
    
    Usart1_Init(19200);

    /* Infinite loop */
    while (1)
    {
        bool bIsKey0;
        bool bIsKey1;
        bool bIsWKUP;  
        
        keypadScan();
               
        //测试键盘缓冲区
        KeyVal = KeyBufR(TRUE, &KeyBuf);
        if(Keypad.u32KeyStatus ^ KeyStatusTemp)
            printf("ss~ss键值状态变化：0x%x--0x%x\r\n", Keypad.u32KeyStatus, KeyStatusTemp);
        if(KeyVal != 0x0000)        
            printf("|键盘缓冲区有数据：0x%x\r\n", KeyVal);
        if((KeyVal & (0xF<<16)) == (uint32_t)(LONGPR_STATUS<<16))
            printf("---长按事件：0x%x\r\n", KeyVal);
        if((KeyVal & (0xF<<16)) == (uint32_t)(DOUBLECLICK_EVENT<<16))
            printf("^^双击事件：0x%x\r\n", KeyVal);
        
        KeyStatusTemp = Keypad.u32KeyStatus;
       
        bIsKey0 = (Keypad.u16ValOnce == KEY1_KEY0) || (KeyVal == KEY1_DOUBLE);  //组合：先KEY1后KEY0
        //bIsKey1 = Keypad.u16ValSus == KEY0_KEY1;  //组合：KEY0+KEY1,无顺序要求
        //按下Key0+Wkup，且Wkup后按下。
        bIsWKUP = (Keypad.u16ValOnce == KEY0_WKUP) || (KeyVal == KEY0_DOUBLE);
        //bIsKey0 = key0_Scan(TRUE);
        bIsKey1 = key1_Scan(TRUE);  //使用单个键扫描。
        
        //bIsWKUP = WKUP_Scan(FALSE);
        if(bIsWKUP)
            bLED0Round = !bLED0Round;

        if (bLED0Round)
            LED1_ON;        
        else
            LED1_OFF;

        //BEEP发声、禁声。
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
                bT1_Enb = TRUE; //Timer1开始定时。
                LED0_ON;
            }
            if(Timer1.bQ && !Timer2.bQ){
                bT2_Enb = TRUE;
                LED0_OFF;
            }
            // 当Timer2定时到时复位两个定时器。
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





