#ifndef __CPU_TIME_H
#define __CPU_TIME_H

//循环代码开始时，将PB9拉低，循环代码结束时，将PB9置高。这样测量，PB9上的PWM波或者看LED即可知道CPU占用时间。

void cpuWorkTimeInit(void);

void cpuWorkStart(void);

void cpuWorkEnd(void);



#endif

