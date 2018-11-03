#include "command.h"

#include <string.h>
#include <stdio.h>


//创建一个命令，包括命令名，回调函数
//如果要接收命令后的数据，需要目标存储地址和数据的长度
/*
void commandInit(CMD_TypeDef *cmd,
                const char *cmd_name_str,
                CMD_Type_Enum callback_type,
                ...)
{
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name) );
    cmd->cmd_type = callback_type;
}
*/

//定长数据接收命令配置函数
void commandSet_DataFixedLength(CMD_TypeDef *cmd, 
                                const char *cmd_name_str,
                                uint8_t *target_address,
                                int data_length)
{
    //配置
    cmd->cmd_type = CMD_DATA_FIXED_LENGTH;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->target_address = target_address;
    cmd->data_length = data_length;

    //没用到的配置清空
    memset(&(cmd->func_union) , 0 , sizeof(cmd->func_union));
    cmd->terminating_character = '\0';
}

//不定长数据接收命令配置函数
void commandSet_DataWithTerminating(CMD_TypeDef *cmd,
                                    const char *cmd_name_str,
                                    uint8_t *target_address,
                                    char terminating_character)
{
    //配置
    cmd->cmd_type = CMD_DATA_WITH_TERMINATING;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->target_address = target_address;
    cmd->terminating_character = terminating_character;

    //没用到的配置清空
    memset(&(cmd->func_union) , 0 , sizeof(cmd->func_union));
    cmd->data_length = 0;

}

//带空参回调函数的命令
void commandSet_CallbackVoid(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(void)
                            )
{
    //配置
    cmd->cmd_type = CMD_CALLBACK_VOID;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->func_union.callbackVoid = callback;

    //没用到的配置清空
    cmd->data_length = 0;
    cmd->terminating_character = '\0';

}

//带int型参数回调函数的命令
void commandSet_CallbackInt(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(int)
                            )
{
    //配置
    cmd->cmd_type = CMD_CALLBACK_INT;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->func_union.callbackInt = callback;

    //没用到的配置清空
    cmd->data_length = 0;
    cmd->terminating_character = '\0';
}

//带float型参数回调函数的命令
void commandSet_CallbackFloat(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(float)
                            )
{
    //配置
    cmd->cmd_type = CMD_CALLBACK_FLOAT;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->func_union.callbackFloat = callback;

    //没用到的配置清空
    cmd->data_length = 0;
    cmd->terminating_character = '\0';

}

//带字符串参数回调函数的命令
void commandSet_CallbackStr(CMD_TypeDef *cmd,
                            const char *cmd_name_str,
                            void (*callback)(char*)
                            )
{
    //配置
    cmd->cmd_type = CMD_CALLBACK_STR;
    strncpy( cmd->command_name , cmd_name_str , sizeof(cmd->command_name));
    cmd->command_name_length = strlen(cmd_name_str);
    cmd->func_union.callbackStr = callback;

    //没用到的配置清空
    cmd->data_length = 0;
    cmd->terminating_character = '\0';
}

//创建一个控制台
//分配该控制台可用的命令表（CMD_TypeDef类型数组及其长度）
//指定输入FIFO与输出FIFO
void commandTerminalInit(CMD_Terminal_TypeDef *terminal,
                        CMD_TypeDef cmd_group[],
                        int cmd_number,
                        FIFO_TypeDef *rx_fifo,
                        FIFO_TypeDef *tx_fifo
                        )
{
    terminal->cmd_group  = cmd_group;
    terminal->cmd_number = cmd_number;
    terminal->rx_fifo    = rx_fifo;
    terminal->tx_fifo    = tx_fifo;
}

//命令接收器轮询
//每隔一定的时间就需要去访问rx_fifo，从中读取命令并且解析
void commandTerminalPolling(CMD_Terminal_TypeDef *terminal)
{
    int receive_state = 0;//指示数据接收的状态
    char cmd_name[16] = {0};

    while( !fifoGetUsedSize(terminal->rx_fifo) ){

        while( fifoReadByte(terminal->rx_fifo) != '#' ){
            if( !fifoGetUsedSize(terminal->rx_fifo) ){
            return;
            }
        }
    }  
}

//命令发送器
//向tx_fifo写命令和数据
//不带数据的命令，可以在第二个参数写NULL
void commandTerminalSendCmd(CMD_Terminal_TypeDef *terminal ,const char *cmd, const uint8_t *data, int length)
{
    //先发送'#'字符
    while( fifoWriteByte(terminal->tx_fifo, '#') != FIFO_OK )
    {
    }

    //把命令名发送到FIFO，不包括'\0'
    while( fifoWrite(terminal->tx_fifo , cmd , strlen(cmd)) != FIFO_OK )
    {
    }
    
    //发送'$'字符
    while( fifoWriteByte(terminal->tx_fifo, '$') != FIFO_OK )
    {
    }

    //把命令后带的参数发送到FIFO
    if(data != NULL){    
        while( fifoWrite(terminal->tx_fifo , data , length) != FIFO_OK )
        {       
        }   
    }
}