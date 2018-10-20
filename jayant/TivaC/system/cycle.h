#ifndef __CYCLE_H
#define __CYCLE_H


/*
 * 简易前后台任务管理器
 *
 * Author : Jayant Tang
 *
 * Email  : jayant97@hust.edu.cn
 *
 *
 *
 * 简介：
 *      使用定时器的简易前后台任务管理器
 *
 * 用法：
 *      (1)初始化一个定时器，每delta_t毫秒产生一次中断
 *          void cycleInit(uint32_t delta_t);
 *
 *      (2)创建任务
 *          cycleTask_TypeDef task1 , task2;
 *
 *      (3)为任务注册任务函数，可以是前台任务也可以是后台任务
 *          void cycleForegroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));
 *          void cycleBackgroundTaskInit(cycleTask_TypeDef* task , uint32_t period , void(*taskFunction)(void));
 *          该任务将会每隔period*delta_t毫秒执行一次
 *
 *      (4)后台任务管理器函数需要手动放入main函数的while(1)中
 *          void cycleMainBackgroundTask(void);
 *
 * 示例：
 *      extern void keyScan(void);
 *      extern void displayerRefresh(void);
 *
 *      void main(void)
 *      {
 *          cycleTask_TypeDef task1 , task2;
 *
 *          void cycleInit(1);//每1ms产生一次中断
 *
 *          cycleForegroundTaskInit(&task1 , 10  , keyScan         );//前台任务每10ms扫描一次按键
 *          cycleBackgroundTaskInit(&task2 , 200 , displayerRefresh);//后台任务每200ms刷新一次屏幕
 *
 *          while(1){
 *              cycleMainBackgroundTask();//后台任务管理器
 *          }
 *      }
 *
 * 注意：
 *      (1)前台任务运行在中断中，后台任务运行在main函数中，前台任务可打断后台任务。
 *      (2)运行时间极短的函数(100ns以下)才能作前台任务；
 *      (3)多个前台任务，没有优先级。同时满足发生条件时，按初始化的先后顺序来先后执行
 *      (4)当有多个后台任务时，如果某个后台任务的运行时间远大于delta_t，则其他后台任务
 *         将可能不会按设置运行。例如两个后台任务，task1和task2。task1每10ms执行一次，
 *         task2每200ms执行一次，但task2每次执行需要150ms，那么这150ms内，task1是无法
 *         打断task2的，无法按10ms一次执行；
 *      (5)一定要把cycleMainBackgroundTask()函数添加到main函数的while(1)循环中
 *
 * 关于移植：
 *          本文件用于TI的Tiva C系列微控制器 TM4C123GH6PM芯片，使用了Timer5。
 *      但是采用了便于移植的写法，即：
 *      (1)所有头文件中可以看到的外部函数，都是和硬件无关的纯软件算法
 *      (2)若想移植到其他单片机上，只需修改cycle.c文件中的与硬件相关的两个内部函数即可
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
