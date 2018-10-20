#include "cycle.h"

#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*����ȫ�ֱ���*/
volatile bool g_cycle_flag = 0;//�жϱ�־
volatile uint32_t g_cycle_count = 0;//�жϼ���
volatile uint32_t g_cycle_period = 1;//�������
volatile cycleTask_TypeDef *g_first_foreground_task = NULL;
volatile cycleTask_TypeDef *g_first_background_task = NULL;

/*Ӳ����ص��ڲ�����*/
static void cycleTimerEnable(uint32_t delta_t , void(*intHandler)(void));//��Ӳ���йص�ʹ�ܲ���
static void cycleTimerIntClear(void);//��Ӳ���йص��ж�״̬�������

/*Ӳ���޹ص��ڲ�����*/
static void tick(void);//�жϷ�����
static uint32_t leastCommonMultiple(uint32_t m, uint32_t n);//����С������


 /*ʵ���ⲿ����*/

/*
 * ���ö�ʱ��
 * ÿ��delta_t�������һ���ж�
 */
void cycleInit(uint32_t delta_t)
{
    cycleTimerEnable( delta_t , tick );
}

/*
 * ��ʼ��һ��ǰ̨����
 * ����һ��ǰ̨������������ִ������
 * ÿ����period��delta_t�����ִ��һ��������
 * ǰ̨������ִ��ʱ�������100us�����µģ����򽫻���ֲ��ȶ�
 */
void cycleForegroundTaskInit(
        cycleTask_TypeDef* task,
        uint32_t period,
        void(*taskFunction)(void))
{
    static cycleTask_TypeDef *s_previous_task = NULL;

    cycleTaskSetPeriod( task , period);
    cycleTaskRegister ( task , taskFunction);
    task->next = NULL;

    if(s_previous_task == NULL){
        g_first_foreground_task = task;
    }else{
        s_previous_task->next   = task;
    }
    s_previous_task = task;

    //ÿע��һ�����񣬶�Ҫ�޸�g_cycle_periodֵ
    g_cycle_period = leastCommonMultiple( g_cycle_period , period);
}

/*
 * ��ʼ��һ����̨����
 * ����һ����̨������������ִ������
 * ÿ����period��delta_t�����ִ��һ��������
 * ������̨�����ʱ��ͬʱ����ʱ���ȳ�ʼ���ĺ�̨���񽫻���ִ�У���˺�̨���������ʱ�䲢����ȫ��ȷ
 * Ҳ��ˣ�Ӧ���ȳ�ʼ����ʱ�϶̵ĺ�̨����
 *
 */
void cycleBackgroundTaskInit(
        cycleTask_TypeDef* task ,
        uint32_t period ,
        void(*taskFunction)(void))
{
    static cycleTask_TypeDef *s_previous_task = NULL;

    cycleTaskSetPeriod( task , period);
    cycleTaskRegister ( task , taskFunction);
    task->next = NULL;

    if(s_previous_task == NULL){
        g_first_background_task = task;
    }else{
        s_previous_task->next = task;
    }
    s_previous_task = task;

    //ÿע��һ�����񣬶�Ҫ�޸�g_cycle_periodֵ
    g_cycle_period = leastCommonMultiple( g_cycle_period , period);
}

/*
 * ��̨������Ⱥ���
 * ֻ�������main������while(1)�У������Զ������̨����
 */
void cycleMainBackgroundTask(void)
{
    cycleTask_TypeDef *task = (cycleTask_TypeDef*)g_first_background_task;
    if(g_cycle_flag){

        while(task != NULL){
            if( !(g_cycle_count % task->period) ){
                if( task->callback != NULL ){
                    task->callback();
                }
            }
            task = task->next;
        }
    }
}


/*
 * ���������ִ������
 * ÿ����period��delta_t�����ִ��һ��������
 */
void cycleTaskSetPeriod(cycleTask_TypeDef* task, uint32_t period)
{
    task->period = period;
}

/*
 * ע��������Ӧ�ĺ���
 */
void cycleTaskRegister(cycleTask_TypeDef* task, void(*taskFunction)(void))
{
    task->callback = taskFunction;
}

/*
 * ע���������Ӧ�ĺ���
 */
void cycleTaskUnregister(cycleTask_TypeDef* task)
{
    task->callback = NULL;
}


/* ʵ���ڲ����� */

/*
 * ��ʱ��ʹ��(Ӳ�����)
 */
static void cycleTimerEnable(uint32_t delta_t , void(*intHandler)(void))
{
        uint32_t system_clock;//ϵͳʱ��Ƶ��(Hz)
        system_clock = SysCtlClockGet();

        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
        TimerConfigure   (TIMER5_BASE , TIMER_CFG_PERIODIC);
        TimerLoadSet(TIMER5_BASE ,TIMER_BOTH ,(uint32_t)(0.001f*delta_t*system_clock) );
        TimerIntRegister (TIMER5_BASE ,TIMER_BOTH, intHandler);
        TimerIntEnable   (TIMER5_BASE, TIMER_TIMA_TIMEOUT);
        IntEnable(INT_TIMER5A);
        TimerEnable(TIMER5_BASE, TIMER_BOTH);
}

/*
 * ��ʱ���ж�״̬���(Ӳ�����)
 */
static void cycleTimerIntClear(void)
{
    TimerIntClear(TIMER5_BASE , TIMER_TIMA_TIMEOUT);
}

/*
 * ��ʱ���жϷ�����(Ӳ���޹�)
 */
static void tick(void)
{
    cycleTask_TypeDef *task = (cycleTask_TypeDef*)g_first_foreground_task;
    cycleTimerIntClear();

    g_cycle_flag = 1;
    g_cycle_count++;
    g_cycle_count %= g_cycle_period;

    //ִ��ǰ̨����
    while( task != NULL){

        if( !(g_cycle_count % task->period) ){
            if(task->callback != NULL){
                task->callback();
            }
        }
        task = task->next;
    }
}

/*
 * ����С������
 */
static uint32_t leastCommonMultiple(uint32_t m, uint32_t n)
{
    uint32_t temp = 0 , r ,max ,min;

    if( m > n){
        max = m;
        min = n;
    }else{
        max = n;
        min = m;
    }

    r = max % min;
    while(r){
        max = min;
        min = r;
        r   = max % min;
    }

    //��ʱminֵ��Ϊ���Լ��
    temp = m * n / min;

    return temp;
}
