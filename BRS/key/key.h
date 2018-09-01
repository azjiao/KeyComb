/*********************************************************
���ܣ� ��������
������ ����¼���Ϊ������������ϰ�������ϰ����ķ�ʽ�����֣����ճ���������а�����
       ճ�����ǰ���A�����ٰ���B�γɵ���ϼ������м��ǰ���A�ͷ��ٰ���B�ͷ��γɵ����а�����
       ���м���Ҫ��һ���Ķ���ʱ���ڴ����İ������вű���Ϊ�����м���
��ƣ� azjiao
�汾�� 0.1
���ڣ� 2018��08��30��
*********************************************************/

#ifndef __KEY_H
#define __KEY_H
#include "template.h"
#include "bitBand.h"

#define  __BITBAND_KEY
// ʹ��λ������KEY��
#ifdef  __BITBAND_KEY
#define KEY0    PEin(4)
#define KEY1    PEin(3)
#define WKUP    PAin(0)
#else
//����KEY0��KEY1��Wk_UP������Ϊ��Ӧkey��������ֵ��
//��Ϊ��ֵ��KEY0��KEY1����ʱֵΪ0��WKUP����ʱֵΪ1.
#define KEY0   GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)
#define KEY1   GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define WKUP  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#endif

//���̻������ߴ�
#define KEYBUFSIZE    16
//������ʹ�ü���ɨ�蹦��keypadScan(�ǵ�����������)�����ֵ���塣
//�����genKeypadVal()�������ļ�ֵ����һ�¡�����ʹ��ֵ���ֶ�λΪ1�����������������16��������
//���һ����ֵ�ж�λΪ1���������ô������ճ����ͬʱ����ʱ���޷������Ƕ������һ������
//�����Ҫ����16������������չ��ֵ��ʾ��16�����������ͬʱ���¶�������Ҽ��˳��
//��Ϊö�����Ͷ����ֵ��
typedef enum {
    KP_KEY0  =  0x0001,
    KP_KEY1  =  0x0002,
    KP_WKUP  =  0x0004
}KeyValType;

//#define KP_KEY0    0x0001
//#define KP_KEY1    0x0002
//#define KP_WKUP    0x0004

//ͨ��ֱ�Ӷ�ȡ�����˿���ֵ����ȡ��������ֵ��
//��������������ʱ�Ķ�������ֵ.�������ڰ����ͷż��Ҳ�������ڼ�����ֵ��⡣
//ע�⣺������ֵ��KEY0/KEY1/WKUP������������.
#define KEY0_CODE   !KEY0?  KP_KEY0: 0x0000
#define KEY1_CODE   !KEY1?  KP_KEY1: 0x0000
#define WKUP_CODE   WKUP?   KP_WKUP: 0x0000

//�����������˲�������ms
#define KEYFILTIME   150U
//ͬһ�������������ļ��ʱ�䣬������ʱ��Ų�����һ����ֵ��������Ϊ��ͬһ�ΰ�������ʱ��Ӧ���ڷ���ʱ�䡣
#define KEYSPACETIME    500U
//���м������м����ʱ�䣬ֻ���ڴ�ʱ���ڵ����м�����Ϊ����ϼ���������Ϊ����ϼ���
#define KEYSEQTIME    2000U

//���̽ṹ
typedef struct
{
    uint16_t u16ValSus; //��ǰ����״ֵ̬(����״ֵ̬).
    uint16_t u16ValOnce; //��������ͬһ���ֻ����һ��ֵ.
    uint16_t u16DownTrg;  //���̰���ͻ���,��ʵ��u16ValOnce��ͬһ�߼���
    uint16_t u16UpTrg;  //�����ͷ�ͻ���
}KeypadType;
extern KeypadType Keypad;

//���̻������ṹ:FIFO.
//��������ŵ��ǵ��������ļ�ֵ���С�
//�������������ɣ�����ȶ��ļ�ֵ������ȶ��ļ�ֵ������Ԥ����ļ��ʱ����Ȼ��Ч�������ڶ�����ֵ��
//�Դ����ƣ�ÿ��������ͬһ����ֵ�ļ��ʱ��ΪKeypad.u16SpaceTime.
typedef struct
{
    uint32_t u32KeyBuffer[KEYBUFSIZE];
    uint16_t u16Index; //������βָ�롣
} KeyBufType;
//���̰��»�����.
extern KeyBufType KeyBuf;
//�����ͷŻ���������16λ�ǰ�������ʱ��msֵ(���65535ms��Լ65s)����16λ�Ǽ���ֵ��
extern KeyBufType KeyBufRelease;

//����Key��ʼ����
void key_Init(void);

//Key0ɨ�����
//bSusKey==TRUE:�������´�����Ρ�
//bSusKey==FALSE:�������µ���һ�ΰ��´�����
bool key0_Scan( bool bSusKey );
//Key1ɨ�����
bool key1_Scan( bool bSusKey );
//WK_UPɨ�����
bool WKUP_Scan( bool bSusKey );

//���ɼ������״ֵ̬����key0����ʱKEYPAD��0λΪ1����key1����ʱKEYPAD��1λΪ1��ͬ����3λ��WKUP�ġ�
uint16_t genKeyCombVal(void);

//���̰���״̬��⡣
//�������������⺯�������Լ���������򵥸�������
//���а����������ܡ�
uint16_t keypadScan(void);

//���̻�����FIFO д������
void KeyBufW(uint32_t KeyVal, bool bNext, KeyBufType* KeyBuf);
//���̻�����FIFO ��������
uint32_t KeyBufR(bool bNext, KeyBufType* KeyBuf);


//������Ŀ��Ҫ�������谴����ϣ�����ǵ��������Ͳ������ⶨ���ˡ�
//��ϼ������˳��Ҫ��������ʱ����ʹ��Keypad.u16ValOnce������Keypad.u16ValSus���ж�.
//KEY0-WKUP��ϣ��Һ���WKUP��
#define KEY0_WKUP    (KP_KEY0 + KP_WKUP) && (Keypad.u16DownTrg == KP_WKUP)
//KEY1-KEY0��ϣ��Һ���KEY0.
#define KEY1_KEY0    (KP_KEY1 + KP_KEY0) && (Keypad.u16DownTrg == KP_KEY0)
//KEY0-KEY1��ϣ���˳��Ҫ��
#define KEY0_KEY1    (KP_KEY0 + KP_KEY1)

#endif /* ifndef __KEY_H */