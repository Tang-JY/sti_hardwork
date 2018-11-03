#include "RingFIFO.h"

#include <string.h>

void fifoInit(FIFO_TypeDef* fifo, uint8_t *buffer, uint8_t length)
{
    fifo->buffer    = buffer;
    fifo->length    = length;
    fifo->get_i     = 0;
    fifo->put_i     = 0;
    fifo->used_size = 0;
    fifo->free_size = length;

    memset(buffer, 0 , length);

    return 0;
}



FIFO_Status_Enum fifoWriteByte(FIFO_TypeDef* fifo , uint8_t data)
{
    if( ! fifo->free_size ){
        return FIFO_FULL;
    }

    fifo->buffer[fifo->put_i] = data;
    fifo->put_i++;
    fifo->put_i %= fifo->length;
    fifo->used_size++;
    fifo->free_size--;

    return FIFO_OK;
}

uint8_t fifoReadByte(FIFO_TypeDef* fifo)
{
    uint8_t temp;

    if( ! fifo->used_size ){
        return FIFO_EMPTY;
    }

    temp = fifo->buffer[fifo->get_i];
    fifo->get_i++;
    fifo->get_i %= fifo->length;
    fifo->used_size--;
    fifo->free_size++;

    return temp;
}

uint8_t fifoPeekByte(FIFO_TypeDef* fifo)
{
    uint8_t temp;

    if( ! fifo->used_size ){
        return FIFO_EMPTY;
    }

    temp = fifo->buffer[fifo->get_i];
    return temp;
}

FIFO_Status_Enum fifoWrite(FIFO_TypeDef* fifo, uint8_t *data , int length)
{
    int i;

    if( fifo->free_size < length ){
        return FIFO_LACK;
    }

    for( i = 0 ; i < length ; i++){
        fifo->buffer[fifo->put_i] = data[i];
        fifo->put_i ++;
        fifo->put_i %= fifo->length;
        fifo->used_size++;
        fifo->free_size--; 
    }

    return FIFO_OK;
}


FIFO_Status_Enum fifoRead(FIFO_TypeDef* fifo, uint8_t *address, int length)
{
    int i;

    if( fifo->used_size < length){
        return FIFO_LACK;
    }

    for( i = 0 ; i< length ; i++ ){
        address[i] = fifo->buffer[fifo->get_i];
        fifo->get_i++;
        fifo->get_i %= fifo->length;
        fifo->used_size--;
        fifo->free_size++;
    }

    return FIFO_OK;
}

int fifoGetFreeSize(FIFO_TypeDef* fifo)
{
    return fifo->free_size;
}

int fifoGetUsedSize(FIFO_TypeDef* fifo)
{
    return fifo->used_size;
}