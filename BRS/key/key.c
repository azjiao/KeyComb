/*
 * �����������Դ������key.
 * KEY0:�ӵ�PE4�˿ڣ��������룻
 * KEY1:�ӵ�PE3�˿ڣ��������룻
 * RESET���ӵ�NRST��TODO...
 * WK_UP:�ӵ�PA0�˿ڣ��������룻
 */
#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
#include "stdio.h"

//�������̽ṹ������
KeypadType Keypad;
//�������̻������ṹ������
//�������Ϊstatic������ڲ����ӣ������ļ������ܷ���KeyBuf.
KeyBufType KeyBuf;

//����Key��ʼ����
void key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; //������ʼ�����ݽṹ��

    //GPIOA��GPIOEʱ��ʹ�ܡ�
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    //KEY0��KEY1������ͬ��
    GPIO_InitStruct.GPIO_Pin = (GPIO_Pin_4 | GPIO_Pin_3);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; //��������ģʽ��
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    //WK_UP���á�
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD; //��������ģʽ��
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    //���̻�������ʼ��
    KeyBuf.u16Index = 0;
    for(int i = 0; i < KEYBUFSIZE; i++)
    {
        KeyBuf.u32KeyBuffer[i] = 0x0;
    }
}

//key0 ɨ�����
//bSusKey==TRUE���������¿���ɨ�赽�������ֵ��
//bSusKey==FALSE����������ֻɨ�赽һ������ֵ��
bool key0_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //�����ϴ�״̬��
    //��������������⵽����ѹ�¡�
    bool bKey;
    //��������ⶨʱ����
    static TimerType timer;
    //����ֵ��
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // ��������
    bKey = TimeON(KEY0_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //�ؼ�⡣�����ؾ�ΪTRUE������ʱΪFALSE.
    //�������������ģʽ��
    if(bSusKey)
        bsLastKey = FALSE;  //��������ģʽʱ�����ϴΰ���״̬��ΪFALSE��
    //����ǵ�����ģʽ��
    else
        bsLastKey = bKey;


    return bRet;
}

//key1 ɨ�����
//bSusKey==TRUE���������¿���ɨ�赽�������ֵ��
//bSusKey==FALSE����������ֻɨ�赽һ������ֵ��
bool key1_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //�����ϴ�״̬��
    //��������������⵽����ѹ�¡�
    bool bKey;
    //��������ⶨʱ����
    static TimerType timer;
    //����ֵ��
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // ��������
    bKey = TimeON(KEY1_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //�ؼ�⡣�����ؾ�ΪTRUE������ʱΪFALSE.
    //�������������ģʽ��
    if(bSusKey)
        bsLastKey = FALSE;  //��������ģʽʱ�����ϴΰ���״̬��ΪFALSE��
    //����ǵ�����ģʽ��
    else
        bsLastKey = bKey;

    return bRet;
}


//WKUP ɨ�����
//bSusKey==TRUE���������¿���ɨ�赽�������ֵ��
//bSusKey==FALSE����������ֻɨ�赽һ������ֵ��
bool WKUP_Scan( bool bSusKey )
{
    static bool bsLastKey = FALSE;  //�����ϴ�״̬��
    //��������������⵽����ѹ�¡�
    bool bKey;
    //��������ⶨʱ����
    static TimerType timer;
    //����ֵ��
    bool bRet = FALSE;

    assert_param(Is_BOOL(bSusKey));

    // ��������
    bKey = TimeON(WKUP_CODE, 150U, &timer);
    bRet = bKey & (bKey ^ bsLastKey);  //�ؼ�⡣�����ؾ�ΪTRUE������ʱΪFALSE.
    //�������������ģʽ��
    if(bSusKey)
        bsLastKey = FALSE;  //��������ģʽʱ�����ϴΰ���״̬��ΪFALSE��
    //����ǵ�����ģʽ��
    else
        bsLastKey = bKey;

    return bRet;
}

//���ɼ������״ֵ̬����key0����ʱu16Keypad��0λΪ1����key1����ʱu16keypad��1λΪ1��ͬ���2λ��WKUP�ġ�
uint16_t genKeyCombVal(void)
{
    uint16_t u16KeyCombVal = 0;
    if(KEY0_CODE)
        u16KeyCombVal |= KP_KEY0;
    if(KEY1_CODE)
        u16KeyCombVal |= KP_KEY1;
    if(WKUP_CODE)
        u16KeyCombVal |= KP_WKUP;

    return u16KeyCombVal;
}

//���̰���״̬ɨ�衣
//�������������⺯�������Լ���������򵥸�������
//���а����������ܡ�
//֧����������ͬһ�鰴�������������ͬһ������״ֵ̬�������Keypad.u16ValSus�С�
//֧����������ͬһ�鰴��ֻ����һ������״ֵ̬�������Keypad.u16ValOnce�С�
//��������ֵΪKeypad.u16DownTrg���ǰ���ͻ���,���򵥶�������ʹ�á�
uint16_t keypadScan(void)
{
    static uint16_t u16KeypadRTTmp = 0x0; //��һ��ʵʱ����״̬�ݴ�ֵ��
    uint16_t u16KeypadRT;  //����ʵʱ����״ֵ̬��
    uint16_t u16KeypadRTTrg;   //��ʵʱ�����ر仯��
    static uint16_t u16KeypadStabValTemp = 0x0; //��һ�μ���ļ���״ֵ̬��
    //��������ⶨʱ����
    static TimerType timer;

    //������ʵʱ״ֵ̬.
    u16KeypadRT = genKeyCombVal();

    //����������ǰ�����μ�⵽�ļ���״ֵ̬��ͬʱ�Ż�ʹ��ʱ������������
    //������150ms������
    //���������μ�ⲻһ��ʱ���ֵΪ��һ�εļ��ֵ.
    //�������������º��ͷ�ʱ������150ms����ʱ��
    u16KeypadRTTrg = u16KeypadRT ^ u16KeypadRTTmp;
    TimeON(!u16KeypadRTTrg, KEYFILTIME, &timer);
    u16KeypadRTTmp = u16KeypadRT;
    if(timer.bQ)
    {
        Keypad.u16ValSus = u16KeypadRT;
    }
    //�������Ҫ�ͷŷ�������else�����ȡ����
    else
        Keypad.u16ValSus = u16KeypadStabValTemp;

    //���Ͼ���������������ȶ�����ֵ��
    //�ؼ�⣺������ͻ�䡣
    Keypad.u16KeypadStabTrg = (Keypad.u16ValSus ^ u16KeypadStabValTemp);
    //����ͻ�����⡣
    Keypad.u16DownTrg = Keypad.u16ValSus & Keypad.u16KeypadStabTrg;
    //�ͷ�ͻ�����⡣
    Keypad.u16UpTrg = ~Keypad.u16ValSus & Keypad.u16KeypadStabTrg;

    //ֻ����һ�μ��,������ϴβ�һ����״ֵ̬������ʵ����������ͬһ���ʱֻ�ܼ��һ��״ֵ̬����
    if(Keypad.u16KeypadStabTrg)
        Keypad.u16ValOnce = Keypad.u16ValSus;
    else
        Keypad.u16ValOnce = 0;

    u16KeypadStabValTemp = Keypad.u16ValSus;

    //��ֵ״̬���ɡ�
    genKeyStatus();
    //״̬�����˫���¼�.
    KeyStatusMachine();
    return Keypad.u16DownTrg;
}

//��ֵ״̬���ɡ�
//��ֵ״̬��32λ�޷�����������16λΪ��ֵԭ�ͣ�bit16-19�Ǽ�ֵ�ϸ�״̬�ļ�¼����20λ�ϲ���һ����ֵ��
//bit20-23λ�Ǽ�ֵ��ǰ���ڣ����£��Ͳ�����(�ͷ�)��״̬��PRESS_STATUSΪ����RELEASE_STATUS���ͷš�
//bit24-31������
//��ֵ״̬�У��������¡����³������м�ʱ�䡢���³�������ʱ�䡢�����ͷš��ͷų������м����ʱ�䣨���̳����޼�ֵ����
void genKeyStatus(void)
{
    #if ISREPEATKEY
    //����ͬһ���������������¼�뻺�����Ķ�ʱ����
    static TimerType timer2;
    //����ͬһ����������ΰ���¼�뻺�����ı�ʶ��
    static bool bMultiFlag = FALSE;
    #endif

    //���¼�ֵ�ݴ�
    /*static uint16_t u16ValSusTemp = 0x0;*/
    static bool bKeyAct = FALSE;  //��ֵ�ȶ��Ҳ�Ϊ0.
    //�����ȶ���ֵ��ʱ���ʱ����
    static TimerType timer3;
    static bool bReset = FALSE;  //��ֵ�ȶ���ʱ����λ��

    //���м������ʱ����
    static TimerType timer4;
    static bool bSeqSpaceEnb = FALSE;  //���м������ʱʹ�ܡ�


    //���̰��»�����д������
    //ʹ��KeypadScan()ɨ����̣����Ի����ճ�������Ϊһ����ֵ����������������ȡ���̻�������ֵʱ���Դ���������Ρ�
    //һ�����ڵ���������������ֵ,������A�ͷ��ٰ������������ų�ʹ��ճ����ϵ����Ρ�

    //���ISREPEATKEYΪTRUE,һ���������µ�ʱ�䳬����KEYSPACETIMEʱ�䣬����Ϊ����Ҫ�ظ�¼��ļ�ֵ��
    #if ISREPEATKEY
    //���������¼���ظ�һ�μ�ֵ����������ʹ��ʱ����λ�����¿��ż�⡣
    if(!timer2.bQ && bMultiFlag)
        bMultiFlag = FALSE;

    //����ͬһ������ͬһ������ֵ��ʱ����Ƿ�¼���μ�ֵ��
    TimeON(!Keypad.u16DownTrg && !bMultiFlag && Keypad.u16ValSus, KEYSPACETIME, &timer2);
    //���̻�����д:������ʱ�򳤰��ѵ�ǰ����ֵ¼�뻺������
    if(Keypad.u16DownTrg || timer2.bQ)
        KeyBufW(Keypad.u16ValSus, TRUE, &KeyBuf);
    if(Keypad.u16DownTrg)
    {
            //���ü�ֵ״̬�������16-23λ��
        Keypad.u32KeyStatus = Keypad.u16ValSus | (PRESS_STATUS << 16) | (PRESS_STATUS << 20);
    }
    //���������¼���ظ�һ�μ�ֵ����������رն�ʱ����
    if(timer2.bQ)
        bMultiFlag = TRUE;

    //���ISREPEATEKEYΪFLASE��һ�������������۶೤ʱ�䶼ֻ¼��һ�μ�ֵ��
    #else
        if(Keypad.u16DownTrg)
        {
            KeyBufW(Keypad.u16ValSus, TRUE, &KeyBuf);
            //���ü�ֵ״̬�������16-23λ��
            Keypad.u32KeyStatus = Keypad.u16ValSus | (PRESS_STATUS << 16) | (PRESS_STATUS << 20);
        }
    #endif

    //��ֵ�ȶ�����ֵ��
    bKeyAct = !Keypad.u16KeypadStabTrg && Keypad.u16ValSus;
    //���м�ֵͻ��ʱ����ʱ��������
    if(Keypad.u16KeypadStabTrg)
        bReset = FALSE;

    //��ֵ�ȶ�����ʱ����
    //���ⰴ���¼������κ��ȶ��ļ���ֵ�����������ǰ��²��������ͷŲ���������ֵΪ0���⣬Ҳ�����ͷź����û�м�ֵ���⡣
    //ʱ���ۼ�
    TimeACC(bKeyAct && !bReset, &timer3);

    //���¼������ʱ��ʹ��ͬһ����ʱ�������м���ʱ�ͳ�����ʱ����ʱ�϶̵����м���ʱ����λ�ڳ�����ʱǰ�汻ִ�У��������ܱ�֤��ʱ�ϳ��ĳ�������ʱ�Ḳ�����м���ʱ��
    //������������¼�����KEYSEQPR_TIME������Ϊ�������а�����
    if(timer3.uEt >= KEYSEQPR_TIME)
    {
        //���ü�ֵ״̬,�����16-23λ��
        Keypad.u32KeyStatus = Keypad.u16ValSus | (NOSEQ_STATUS << 16) | (PRESS_STATUS << 20);
    }

    //�����¼���
    if(timer3.uEt >= LONGPR_TIME)
    {
        uint32_t KeyVal;
        KeyVal =  Keypad.u16ValSus| ((uint32_t)(LONGPR_STATUS << 16));
        KeyBufW(KeyVal, TRUE, &KeyBuf);
        //���ü�ֵ״̬,�����16-23λ��
        Keypad.u32KeyStatus = KeyVal | (PRESS_STATUS << 20);
        bReset = TRUE;
    }

    //�����ֵΪ0�Ҽ�ֵ״̬���ڷ�0״̬�����ü�ֵ״̬,��ֵά��֮ǰ��ֵ��
    if(!Keypad.u16ValSus && Keypad.u32KeyStatus)
    {
        Keypad.u32KeyStatus &= 0xFFFFF; //������20λ���ݡ�
        Keypad.u32KeyStatus |= RELEASE_STATUS << 20;
        bSeqSpaceEnb = TRUE;        
    }
    else
        bSeqSpaceEnb = FALSE;
    TimeON(bSeqSpaceEnb, KEYSEQSPACE_TIME, &timer4);
    //������̼�ֵΪ0ʱ�䳬��KEYSEQSPACE_TIME��������м����ʱ�䣬���ü�ֵ״̬���㸴λ��
    if(timer4.bQ)
    {
        Keypad.u32KeyStatus = 0x0;
        bSeqSpaceEnb = FALSE;           
    }     

}

//״̬��.
//˫�����ݽṹ��֧�ֵ���˫������ȻҲ֧�ֶ��˫�������������������������
struct {
        uint8_t u8State; //״̬��״̬��
        uint32_t uiKeyValOld; //�ϴμ�ֵ��
        uint8_t u8Count;  //����״̬������
    }DoubleClick;

void KeyStatusMachine(void)
{
    uint16_t u16curKeyStatus; //��ǰ��ֵ״̬��
    uint32_t uicurKeyVal;  //��ǰ��ֵ��
    static uint32_t u32KeyStatusOld = 0x0;  //�ϴμ�ֵ״̬�֡�

    //���������ֵ״̬�ı��ִ��״̬����
    if(u32KeyStatusOld ^ Keypad.u32KeyStatus)
    {
        uicurKeyVal = (Keypad.u32KeyStatus & 0xFFFFF);  //ȡ�õ�ǰ��ֵ:20λ���ϼ�ֵ��
        u16curKeyStatus = (Keypad.u32KeyStatus & 0xFFF00000)>>20; //ȡ�õ�ǰ��ֵ״̬.

        //�жϵ�һ��״̬��ʼ��
        if((DoubleClick.uiKeyValOld == 0x0) && (u16curKeyStatus == PRESS_STATUS))
        {
            DoubleClick.uiKeyValOld = uicurKeyVal;
            DoubleClick.u8Count = 0x0;
            DoubleClick.u8State = 0x1;
        }
        //�жϽ���״̬��
        //�������˫�����⵽��˫���Զ���λ״̬�⣬�����̼�⵽�޼�ֵ״̬���޼�ֵ�ҳ����������м����ʱ�䣩�Զ���λ��
        if(Keypad.u32KeyStatus == 0x0)
        {
            DoubleClick.uiKeyValOld = 0x0;
            DoubleClick.u8Count = 0x0;
            DoubleClick.u8State = 0x0;
        }

        switch(DoubleClick.u8State){
            case 0x01:
                //�жϵ�һ��״̬
                if(!(uicurKeyVal ^ DoubleClick.uiKeyValOld) && (u16curKeyStatus == PRESS_STATUS))
                {
                    DoubleClick.uiKeyValOld = uicurKeyVal;
                    DoubleClick.u8State++;
                }
                //������Ǽ�ֵ�ı䣬˫����Ч��
                else {
                    //�¼�����������λ״̬�֡�
                    DoubleClick.uiKeyValOld = 0x0;
                    DoubleClick.u8Count = 0x0;
                    DoubleClick.u8State = 0x0;
                }
                break;

            case 0x02:
                //˫����⵽��һ��״̬��ʼ�ڶ�״̬���.
                //����Ƿ����ͷ�״̬:��ֵһ���Ҽ�ֵ״̬��ǰ���ͷš�
                if(!(uicurKeyVal ^ DoubleClick.uiKeyValOld) && u16curKeyStatus == RELEASE_STATUS)
                {
                    DoubleClick.uiKeyValOld = uicurKeyVal;
                    DoubleClick.u8Count++;
                    DoubleClick.u8State = 0x1;
                    if(DoubleClick.u8Count == 0x02)
                    {
                        //��˫���¼�¼�뻺������
                        uicurKeyVal &= 0xFFFF;  //ȡ�ü�ֵԭ�롣
                        KeyBufW(uicurKeyVal | DOUBLECLICK_EVENT << 16, TRUE, &KeyBuf);
                        //�¼�����������λ״̬�֡�
                        //Ϊ�˲������û����飬�����˫���ּ���������������������״̬�����,����˫�����á�
                        DoubleClick.uiKeyValOld = 0x0;
                        DoubleClick.u8Count = 0x0;
                        DoubleClick.u8State = 0x0;
                    }
                }
                //�����ǰ�����ʱ��
                else {
                    //�¼�����������λ״̬�֡�
                    DoubleClick.uiKeyValOld = 0x0;
                    DoubleClick.u8Count = 0x0;
                    DoubleClick.u8State = 0x0;
                }
                break;
        }
    }
    u32KeyStatusOld = Keypad.u32KeyStatus;    
}

//���̻�����FIFO д������
//�򻺳���β��д��һ����Ԫֵ��
//�������������û�б���������������򻺳����ڵ��׵�Ԫ���Ƴ����������ᵼ�°�����©����Ҫ�û����򾡿��ȡ¼��ļ�ֵ,������©������
//FIXME:����bNext������ʵ�ֶ���һ��ֵ�ĸ��Ǻ�������ӡ�
void KeyBufW(uint32_t KeyVal, bool bNext, KeyBufType* pKeyBuf)
{
    //�����д��һ�����ݡ�
    if(bNext)
    {
        //������̻���������Խ�磬�����Ȱѻ�������ͷ���ƶ�һ����Ԫ��
        if(pKeyBuf->u16Index >= KEYBUFSIZE)
        {
            for(uint16_t i = 0; i < (KEYBUFSIZE-1); i++)
                pKeyBuf->u32KeyBuffer[i] = pKeyBuf->u32KeyBuffer[i+1];
            pKeyBuf->u16Index--;
        }

        pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index++] = KeyVal;
    }
    //�������һ�ε����ݸ��ǡ�
    else{
        if(pKeyBuf->u16Index > 0)
           pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index - 1] = KeyVal;
        //����ǵ�0��Ԫ�������⴦��
        else
            pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index++] = KeyVal;
    }

}

//���̻�����FIFO ��������
//�ӻ�����ͷ������һ����Ԫֵ���ѻ�����ֵ��ͷ���ƶ�һ����Ԫ��
//FIXME������bNext������ʵ�ֶ��׵�Ԫ���ظ���ȡ���������ԡ�
uint32_t KeyBufR(bool bNext, KeyBufType* pKeyBuf)
{
    uint32_t KeyVal;

    if(pKeyBuf->u16Index > 0)
    {
        KeyVal = pKeyBuf->u32KeyBuffer[0];

        //������������ԣ���������ͷ���ƶ�һ����Ԫ��
        //������ظ���ȡͷ����Ԫ���򲻽����ƶ���
        if(bNext)
        {
            for(uint16_t i = 0; i < (pKeyBuf->u16Index-1); i++)
                pKeyBuf->u32KeyBuffer[i] = pKeyBuf->u32KeyBuffer[i+1];
            pKeyBuf->u32KeyBuffer[pKeyBuf->u16Index-1] = 0x0000;
            pKeyBuf->u16Index--;
        }
    }
    else
        KeyVal = 0x0000;

    return  KeyVal;
}
