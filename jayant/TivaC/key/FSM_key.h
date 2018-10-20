#ifndef __FSM_KEY_H
#define __FSM_KEY_H

#include <stdint.h>
/*
 * ��������״̬��(FSM)���������Ķ���������
 *
 * Author : Jayant Tang
 *
 * Email  : jayant97@hust.edu.cn
 *
 *
 * ���:
 *      ������ѯ��ʽ��ȡGPIO��ƽ(ÿ10ms��ȡһ��)��
 *
 *      ֧�ֵ�����˫����������
 *
 *      ��Ϊ������˫�����ûص�������
 *      ������˫���󶼻���Ե����Լ��Ļص�������
 *
 *      ����1s��ÿ��0.5s�ͻ����һ�ε����ص�������ֱ����������
 *
 * ������ֲ��
 *          ���ļ�����TI��Tiva Cϵ��΢������ TM4C123GH6PMоƬ
 *      ���ǲ����˱�����ֲ��д��������
 *      (1)����ͷ�ļ��п��Կ������ⲿ���������Ǻ�Ӳ���޹صĴ�����㷨
 *      (2)������ֲ��������Ƭ���ϣ�ֻ���޸�FSM_key.c�ļ��е���Ӳ����ص������ڲ���������
 *
 */

//����״̬ö��
typedef enum{
    NORMAL         = 0,//����
    PRE_ACTIVE        ,
    PRE_HIT           ,
    PRE_LONG_PRESS    ,
    SINGLE_HIT        ,//����
    DOUBLE_HIT        ,//˫��
    LONG_PRESS        ,//����
}keyState_Enum;

//��������ö��
typedef enum{
    LOW = 0,//����ʱΪ�͵�ƽ
    HIGH
}keyPolarity_Enum;

//�����ṹ��
struct fsmKey_Struct{
    uint32_t port;                    //�˿ڻ���ַ
    uint32_t num;                     //���ű��
    keyPolarity_Enum polarity;        //����
    void (*singleClickCallback)(void);//�����ص�����
    void (*doubleClickCallback)(void);//˫���ص�����

    keyState_Enum state;              //����״̬
    uint8_t sample[4];                //�̲���
    uint16_t counter;                 //���ڴ���˫���ͳ����ļ�����
    struct fsmKey_Struct *next;       //����ṹ��ָ����һ������������
};
typedef struct fsmKey_Struct key_TypeDef;



extern void keyInit(key_TypeDef* key,
               uint32_t Port,
               uint8_t PinNumber,
               keyPolarity_Enum Polarity,
               void(*sigleClick)(void),
               void(*doubleClick)(void));

extern void keyScanAll(void);
extern void keyScan(key_TypeDef* key);


#endif /* __FSM_KEY_H */
