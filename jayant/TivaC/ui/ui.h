#ifndef __UI_H
#define __UI_H

#include <stdint.h>
#include <stdbool.h>


//显示器使用引脚PA5 PB3 PB4 PB7


//显示器字体枚举
typedef enum{
    FONT1206 = 12,//一个字符的像素占12字节
    FONT1608 = 16,
    FONT2412 = 36
}uiFont_Enum;

/*显示器*/
void uiDisplayInit(void);
void uiDisplayClear(void);
void uiGRAMClear(void);
void uiDisplayerRefresh(void);

void uiDisplayDrawPoint(int x, int y);
void uiDisplayErasePoint(int x, int y);
void uiDisplayDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void uiDisplayDrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void uiDisplayDrawBlock(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void uiDisplayEraseBlock(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void uiDisplayDrawCircle(uint8_t x0 , uint8_t y0 , uint8_t radius );
void uiDisplayShowChar(uint8_t x , uint8_t y , uiFont_Enum font , char ascii);
void uiDisplayShowString(uint8_t x , uint8_t y , uiFont_Enum font , char* str);
void uiDisplayShowFloat(uint8_t x , uint8_t y , uiFont_Enum font , float data, uint8_t precision);
void uiDisplayShowNumber(uint8_t x , uint8_t y , uiFont_Enum font , int data);


/*数据显示框*/

//数据显示框结构体
struct DisplayType{
    char label[16];
    float *data;
    uint8_t x;
    uint8_t y;
    uint16_t length;//显示框的长度
    struct DisplayType *next;
};
typedef struct DisplayType uiDataDisplayer_TypeDef;

void uiDataDisplayerInit(uiDataDisplayer_TypeDef* displayer,
                         const char *label,
                         float *data,
                         uint8_t x, uint8_t y,
                         uint8_t length);
void uiDataDisplayersRefresh(void);//刷新所有数据显示框到显存中
void uiDataDisplayerRefresh(uiDataDisplayer_TypeDef *displayer);//刷新单个数据显示框到显存中

#endif
