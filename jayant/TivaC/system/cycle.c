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


/*声明全局变量*/
volatile bool g_cycle_flag = 0;//中断标志
volatile uint32_t g_cycle_count = 0;//中断计数
volatile uint32_t g_cycle_period = 1;//最大周期
volatile cycleTask_TypeDef *g_first_foreground_task = NULL;
volatile cycleTask_TypeDef *g_first_background_task = NULL;

/*硬件相关的内部函数*/
static void cycleTimerEnable(uint32_t delta_t , void(*intHandler)(void));//与硬件有关的使能操作
static void cycleTimerIntClear(void);//与硬件有关的中断状态清除操作
static void cycleTimerPriority(void);//定时器5设置为中断优先级最低

/*硬件无关的内部函数*/
static void tick(void);//中断服务函数
static uint32_t leastCommonMultiple(uint32_t m, uint32_t n);//求最小公倍数


 /*实现外部函数*/

/*
 * 配置定时器
 * 每隔delta_t毫秒产生一次中断
 */
void cycleInit(uint32_t delta_t)
{
    cycleTimerPriority();
    cycleTimerEnable( delta_t , tick );
}

/*
 * 初始化一个前台任务
 * 配置一个前台任务函数和它的执行周期
 * 每经过period个delta_t毫秒就执行一次任务函数
 * 前台函数的执行时间必须是100us级以下的，否则将会出现不稳定
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

    //每注册一个任务，都要修改g_cycle_period值
    g_cycle_period = leastCommonMultiple( g_cycle_period , period);
}

/*
 * 初始化一个后台任务
 * 配置一个后台任务函数和它的执行周期
 * 每经过period个delta_t毫秒就执行一次任务函数
 * 两个后台任务的时间同时满足时，先初始化的后台任务将会先执行，因此后台任务的运行时间并不完全精确
 * 也因此，应当先初始化耗时较短的后台任务
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

    //每注册一个任务，都要修改g_cycle_period值
    g_cycle_period = leastCommonMultiple( g_cycle_period , period);
}

/*
 * 后台任务调度函数
 * 只需放置在main函数的while(1)中，即可自动管理后台任务
 */
void cycleMainBackgroundTask(void)
{
    cycleTask_TypeDef *task = (cycleTask_TypeDef*)g_first_background_task;
    if(g_cycle_flag){
    g_cycle_flag = 0;
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
 * 设置任务的执行周期
 * 每经过period个delta_t毫秒就执行一次任务函数
 */
void cycleTaskSetPeriod(cycleTask_TypeDef* task, uint32_t period)
{
    task->period = period;
}

/*
 * 注册该任务对应的函数
 */
void cycleTaskRegister(cycleTask_TypeDef* task, void(*taskFunction)(void))
{
    task->callback = taskFunction;
}

/*
 * 注销该任务对应的函数
 */
void cycleTaskUnregister(cycleTask_TypeDef* task)
{
    task->callback = NULL;
}


/* 实现内部函数 */

/*
 * 定时器使能(硬件相关)
 */
static void cycleTimerEnable(uint32_t delta_t , void(*intHandler)(void))
{
        uint32_t system_clock;//系统时钟频率(Hz)
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
 * 定时器中断状态清除(硬件相关)
 */
static void cycleTimerIntClear(void)
{
    TimerIntClear(TIMER5_BASE , TIMER_TIMA_TIMEOUT);
}

/*
 * 定时器中断优先级设置为最低(硬件相关)
 */
static void cycleTimerPriority(void)//与硬件有关的中断优先级操作
{
    //设置NVIC优先级分组，7位抢占优先级，1位亚优先级
    HWREG(NVIC_APINT) |= NVIC_APINT_PRIGROUP_7_1;

    //将定时器5的中断优先级设置为最低(0xff)
    //TM4C123GH6PM实际上只有3位优先级，因此用掩码0xE0屏蔽掉低5位
    HWREG(NVIC_PRI23) |= NVIC_PRI23_INTA_M & (0x3 << NVIC_PRI23_INTA_S);
}

/*
 * 定时器中断服务函数(硬件无关)
 */
static void tick(void)
{
    cycleTask_TypeDef *task = (cycleTask_TypeDef*)g_first_foreground_task;
    cycleTimerIntClear();

    g_cycle_flag = 1;
    g_cycle_count++;
    g_cycle_count %= g_cycle_period;

    //执行前台任务
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
 * 求最小公倍数
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

    //此时min值即为最大公约数
    temp = m * n / min;

    return temp;
}
