#ifndef __CYCLE_H
#define __CYCLE_H


/*
 * ����ǰ��̨���������
 *
 * Author : Jayant Tang
 *
 * Email  : jayant97@hust.edu.cn
 *
 *
 *
 * ��飺
 *      ʹ�ö�ʱ���ļ���ǰ��̨���������
 *
 * �÷���
 *      (1)��ʼ��һ����ʱ����ÿdelta_t�������һ���ж�
 *          void cycleInit(uint32_t delta_t);
 *
 *      (2)��������
 *          cycleTask_TypeDef task1 , task2;
 *
 *      (3)Ϊ����ע����������������ǰ̨����Ҳ�����Ǻ�̨����
 *          void cycleForegroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));
 *          void cycleBackgroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));
 *          �����񽫻�ÿ��period*delta_t����ִ��һ��
 *
 *      (4)��̨���������������Ҫ�ֶ�����main������while(1)��
 *          void cycleMainBackgroundTask(void);
 *
 * ʾ����
 *      extern void keyScan(void);
 *      extern void displayerRefresh(void);
 *
 *      void main(void)
 *      {
 *          cycleTask_TypeDef task1 , task2;
 *
 *          void cycleInit(1);//ÿ1ms����һ���ж�
 *
 *          cycleForegroundTaskInit(&task1 , 10  , keyScan         );//ǰ̨����ÿ10msɨ��һ�ΰ���
 *          cycleBackgroundTaskInit(&task2 , 200 , displayerRefresh);//��̨����ÿ200msˢ��һ����Ļ
 *
 *          while(1){
 *              cycleMainBackgroundTask();//��̨���������
 *          }
 *      }
 *
 * ע�⣺
 *      (1)ǰ̨�����������ж��У���̨����������main�����У�ǰ̨����ɴ�Ϻ�̨����
 *      (2)����ʱ�伫�̵ĺ���(100ns����)������ǰ̨����
 *      (3)���ǰ̨����û�����ȼ���ͬʱ���㷢������ʱ������ʼ�����Ⱥ�˳�����Ⱥ�ִ��
 *      (4)���ж����̨����ʱ�����ĳ����̨���������ʱ��Զ����delta_t����������̨����
 *         �����ܲ��ᰴ�������С�����������̨����task1��task2��task1ÿ10msִ��һ�Σ�
 *         task2ÿ200msִ��һ�Σ���task2ÿ��ִ����Ҫ150ms����ô��150ms�ڣ�task1���޷�
 *         ���task2�ģ��޷���10msһ��ִ�У�
 *      (5)һ��Ҫ��cycleMainBackgroundTask()������ӵ�main������while(1)ѭ����
 *
 * ������ֲ��
 *          ���ļ�����TI��Tiva Cϵ��΢������ TM4C123GH6PMоƬ��ʹ����Timer5��
 *      ���ǲ����˱�����ֲ��д��������
 *      (1)����ͷ�ļ��п��Կ������ⲿ���������Ǻ�Ӳ���޹صĴ�����㷨
 *      (2)������ֲ��������Ƭ���ϣ�ֻ���޸�cycle.c�ļ��е���Ӳ����ص������ڲ���������
 */



#include <stdint.h>
#include <stdbool.h>


struct cycleTask{
    uint32_t period;
    void (*callback)(void);
    struct cycleTask *next;
};
typedef struct cycleTask cycleTask_TypeDef;


extern void cycleInit(uint32_t delta_t);
extern void cycleMainBackgroundTask(void);

extern void cycleForegroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));
extern void cycleBackgroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));

extern void cycleTaskSetPeriod(cycleTask_TypeDef* task, uint32_t period);
extern void cycleTaskRegister(cycleTask_TypeDef* task,  void(*taskFunction)(void));
extern void cycleTaskUnregister(cycleTask_TypeDef* task);


#endif /* __CYCLE_H */
