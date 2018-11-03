#ifndef __RING_FIFO_H_
#define __RING_FIFO_H_

// Jayant Tang
// jayant97@hust.edu.cn

//FIFO Controller


#include <stdint.h>
#include <stdbool.h>

typedef enum{
    FIFO_OK    =  0,
    FIFO_FULL  = -1,
    FIFO_EMPTY = -2,
    FIFO_LACK  = -3
}FIFO_Status_Enum;

typedef struct{
    uint8_t *buffer;
    int length;
    int put_i;
    int get_i;
    int free_size;
    int used_size;
}FIFO_TypeDef;


void fifoInit(FIFO_TypeDef* fifo, uint8_t *buffer, uint8_t length);
FIFO_Status_Enum fifoWriteByte(FIFO_TypeDef* fifo, uint8_t data);
uint8_t          fifoReadByte(FIFO_TypeDef* fifo);
uint8_t          fifoPeekByte(FIFO_TypeDef* fifo);
FIFO_Status_Enum fifoWrite(FIFO_TypeDef* fifo, uint8_t *data , int length);
FIFO_Status_Enum fifoRead(FIFO_TypeDef* fifo, uint8_t *address, int length);
int         fifoGetFreeSize(FIFO_TypeDef* fifo);
int         fifoGetUsedSize(FIFO_TypeDef* fifo);

#endif // !__RING_FIFO_H_
