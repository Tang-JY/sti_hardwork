#ifndef __COMMAND_H_
#define __COMMAND_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "RingFIFO.h"

//  Jayant Tang
//  jayant97@hust.edu.cn

//基于环形FIFO的命令终端
//
//  命令：
//  (1)支持2种类型的命令：收到命令后触发回调函数的、或者收到命令后把紧跟在命令后面的数据传输到指定地址的
//  (2)命令名是不超过16字节的字符串，且不能包含'#'字符和'$'字符
//  (3)向终端写命令时，每个命令名前要加'#'字符，命令名后要加'$'字符，然后再加数据(如果有数据的话)
//      例如： #LED_ON$ , #SET_ANGLE&30
// 
//  终端：
//  (1)可配置输入FIFO和输出FIFO
//  (2)可解析输入FIFO中读到的命令，并执行回调函数，或者把命令后的数据传输到指定位置
//  (3)可以向输出FIFO中写命令。写命令时，调用函数commandTerminalSendCmd，这时不用在命令前后加'#'和'$'，函数内部自动加入
//




//命令类型枚举
// (1)用于数据传输的命令：需要指定数据存放的位置，不需注册回调函数
//      (a)待传输的数据是定长的，需要指定数据长度
//      (b)待传输的数据是不定长的，需要指定终止符
// (2)用于触发回调函数的命令：需要指定回调函数的参数，不需要指定数据存放的位置
//      (a)参数类型是void
//      (b)参数类型是int
//      (c)参数类型是float
//      (d)参数是一个字符串，以‘\0’结尾
typedef enum{
    CMD_DATA_FIXED_LENGTH = 0,
    CMD_DATA_WITH_TERMINATING,
    CMD_CALLBACK_VOID ,
    CMD_CALLBACK_INT  ,
    CMD_CALLBACK_FLOAT,
    CMD_CALLBACK_STR  
}CMD_Type_Enum;

//回调函数联合体
typedef union{
    void (*callbackVoid)(void);
    void (*callbackInt)(int);
    void (*callbackFloat)(float);
    void (*callbackStr)(char*);
}CMD_Callback_Union;

//命令数据结构
//包含命令名（字符串），命令类型，回调函数，
typedef struct{
    char command_name[16];
    int command_name_length;
    CMD_Type_Enum cmd_type;
    uint8_t *target_address;
    int data_length;
    uint8_t terminating_character;
    CMD_Callback_Union func_union;
}CMD_TypeDef;

//命令终端数据结构
//包含一个命令表(首地址和长度)，一个接受fifo和一个发送fifo
typedef struct{
    CMD_TypeDef *cmd_group;
    int cmd_number;
    FIFO_TypeDef *rx_fifo;
    FIFO_TypeDef *tx_fifo;
}CMD_Terminal_TypeDef;


//命令配置系列函数
void commandSet_DataFixedLength(CMD_TypeDef *cmd, const char *cmd_name_str,
                                uint8_t *target_address,
                                int data_length);
void commandSet_DataWithTerminating(CMD_TypeDef *cmd,
                                    const char *cmd_name_str,
                                    uint8_t *target_address,
                                    char terminating_character);
void commandSet_CallbackVoid(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(void)
                            );
void commandSet_CallbackInt(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(int)
                            );
void commandSet_CallbackFloat(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(float)
                            );
void commandSet_CallbackStr(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(char*)
                            );

//命令终端相关函数
void commandTerminalInit(CMD_Terminal_TypeDef *terminal,
                        CMD_TypeDef cmd_group[],
                        int cmd_number,
                        FIFO_TypeDef *rx_fifo,
                        FIFO_TypeDef *tx_fifo
                        );
void commandTerminalPolling(CMD_Terminal_TypeDef *terminal);
void commandTerminalSendCmd(CMD_Terminal_TypeDef *terminal , const char *cmd, const uint8_t *data, int length);


#endif / ! __COMMAND_H_