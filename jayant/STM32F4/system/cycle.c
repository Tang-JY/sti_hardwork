#include "cycle.h"




#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_tim.h"

//#define cycleTick TIM7_IRQHandler

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
static void cycleTick(void);//�жϷ�����
static uint32_t leastCommonMultiple(uint32_t m, uint32_t n);//����С������

//void (*TIM7_IRQHandler)(void) = NULL;
void TIM7_IRQHandler()
{
	cycleTick();
}

 /*ʵ���ⲿ����*/

/*
 * ���ö�ʱ��
 * ÿ��delta_t�������һ���ж�
 */
void cycleInit(uint32_t delta_t)
{
    cycleTimerEnable( delta_t , cycleTick );
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
	
        //SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
        //TimerConfigure   (TIMER5_BASE , TIMER_CFG_PERIODIC);
        //TimerLoadSet(TIMER5_BASE ,TIMER_BOTH ,(uint32_t)(0.001f*delta_t*system_clock) );
        //TimerIntRegister (TIMER5_BASE ,TIMER_BOTH, intHandler);
        //TimerIntEnable   (TIMER5_BASE, TIMER_TIMA_TIMEOUT);
        //IntEnable(INT_TIMER5A);
        //TimerEnable(TIMER5_BASE, TIMER_BOTH);
	
				TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
				NVIC_InitTypeDef NVIC_InitStructure;
				
	
	
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE); 
				TIM_TimeBaseInitStructure.TIM_Period = (1000)*delta_t - 1; 
	
				RCC_ClocksTypeDef rcc;
				RCC_GetClocksFreq(&rcc);//�������ʱ��
				if(rcc.HCLK_Frequency / rcc.PCLK1_Frequency == 1){
					TIM_TimeBaseInitStructure.TIM_Prescaler = rcc.PCLK1_Frequency/1000000 -1;
				}else{
					TIM_TimeBaseInitStructure.TIM_Prescaler = 2*rcc.PCLK1_Frequency/1000000 -1;
				}
				TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 
				//TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
				TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);
				TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE); 
	
				NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn; 
				NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; 
				NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; 
				NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
				NVIC_Init(&NVIC_InitStructure);
				TIM_Cmd(TIM7,ENABLE);
		
				//uint32_t system_clock;//ϵͳʱ��Ƶ��(Hz)
        //system_clock = SysCtlClockGet();
				
				
				
}

/*
 * ��ʱ���ж�״̬���(Ӳ�����)
 */
static void cycleTimerIntClear(void)
{
    TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
}

/*
 * ��ʱ���жϷ�����(Ӳ���޹�)
 */
static inline void cycleTick(void)
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
