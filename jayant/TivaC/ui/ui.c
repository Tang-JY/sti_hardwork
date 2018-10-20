#include "ui/ui.h"
#include "ui/hw_uc1701.h"
#include "ui/uc1701.h"
#include "ui/font.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ui/str_convert.h>

#include "inc/hw_types.h"   //�ṩ��HWREG�ȵĶ�д�Ĵ����õĺ�
#include "inc/hw_memmap.h"  //��������������Ļ���ַ
#include "inc/hw_gpio.h"    //������GPIO�ļĴ�������������ַƫ����

#include "driverlib/sysctl.h" //�ṩ��ϵͳ���ƵĿ⺯��(ʹ������ʱ��0)
#include "driverlib/gpio.h"   //�ṩ��GPIO�Ŀ⺯��



/*�����ڲ�����*/
static int  uiEdgeCheck(int x, int y);//�����Ƿ�����Ļ��



/*�Դ�*/
volatile uint8_t GRAM[8][128] = {0};


/*����ȫ�ֱ���*/
volatile uiDataDisplayer_TypeDef *g_first_data_displayer = NULL;//��¼��һ��������ʾ��ĵ�ַ



/*ʵ���ⲿ����*/


//������ʾ���ʼ��
void uiDataDisplayerInit(uiDataDisplayer_TypeDef* displayer,
                         const char *label,
                         float *data,
                         uint8_t x,
                         uint8_t y,
                         uint8_t length)
{
    static uiDataDisplayer_TypeDef *s_previous_displayer = NULL;
    uint8_t label_length;

    //������ʾ�����ñ���

    label_length = strlen(label);
    displayer->data      = data;
    displayer->x         = x;
    displayer->y         = y;
    displayer->length    = length;
    displayer->next      = NULL;

    memset (displayer->label,'\0',16);
    strncpy(displayer->label , label , label_length);

    if(s_previous_displayer == NULL){
        //����ǵ�һ������ʼ������ʾ�򣬽�����������
            g_first_data_displayer = displayer;//�������ĵ�ַ��ȫ�ֱ���
    }else{
        //���ǵ�һ������ʼ������ʾ����ѱ���ʾ����������ӵ���һ����ʾ����
            s_previous_displayer->next = displayer;
    }

        //ȫ��������ɣ��ѱ������ĵ�ַ�����´�ʹ��
        s_previous_displayer = displayer;
}


//ˢ��һ��������ʾ���ڴ���
void uiDataDisplayerRefresh(uiDataDisplayer_TypeDef *displayer)
{
    uint8_t label_length, x1,y1;
    uint8_t x2, y2;
    float data_value;
    char data_str[32];

    label_length  = strlen(displayer->label) + 1;

    memset(data_str,'\0',32);
    data_value = *(displayer->data);
    Float2Str(data_str, data_value, 2, 32);

    data_str[31] = '\0';

    x1 = displayer->x - 6*label_length;
    y1 = displayer->y;
    uiDisplayShowString( x1, y1 , FONT1206 , displayer->label);

    x1 = displayer->x;
    uiDisplayShowString( x1, y1 , FONT1206 , data_str);

    x1 = displayer->x - 2;
    y1 = displayer->y - 1;
    x2 = displayer->x + (displayer->length)*6 + 1;
    y2 = displayer->y + 12;
    uiDisplayDrawFrame ( x1, y1 , x2, y2);
}

//ˢ��ȫ��������ʾ���ڴ���
void uiDataDisplayersRefresh(void)
{
    uiDataDisplayer_TypeDef* pointer = (uiDataDisplayer_TypeDef*)g_first_data_displayer;

     while(1)
     {
         if(pointer != NULL)
         {
             uiDataDisplayerRefresh(pointer);//ɨ�赥��
             pointer = pointer->next;//�������ƶ�����һ��
         }else{
             break;
         }
     }
     return;
}


//��ʾ����ʼ��
void uiDisplayInit(void)
{
    UC1701Init(100000);//60000ΪSSI��Ƶ��
    UC1701Clear();
}

//��ʾ�����
void uiDisplayClear(void)
{
    UC1701Clear();
}

//�Դ����
void uiGRAMClear(void)
{
     uint8_t i,j,*p;
     p=(uint8_t*)GRAM;
     for(i=0; i<8;i++){
         p = (uint8_t*)GRAM[i];
         for(j=0; j<128;j++){
             *p = 0x00;
             p++;
         }
     }
}

//ˢ���Դ浽��Ļ��
void uiDisplayerRefresh(void)
{
    uint8_t i, j, data;

    for(i=0 ; i<8 ; i++){
        //��ҳ����ˢ�£��������õ�ַ
        UC1701CmdWrite(0xb0+i);
        UC1701CmdWrite(0x10);
        UC1701CmdWrite(0x00);

        for(j=0 ; j<128 ; j++){
            data = GRAM[i][127-j];
            UC1701DataWrite(data);
        }
    }
}

//����
void uiDisplayDrawPoint(int x, int y)
{
    uint8_t data;
    if(uiEdgeCheck(x,y)){//Ҫ���ĵ�����Ļ��
        return;
    }
    data = 0x01<<(y%8);
    GRAM[y/8][x] |= data;
}

//������
void uiDisplayErasePoint(int x, int y)
{
    uint8_t data = ~( 0x01<<(y%8));
    GRAM[y/8][x] &= data;
}

//����(Bresenham�㷨)
void uiDisplayDrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    short int delta_x, delta_y, error, polarity;
    uint8_t x_start, y_start, i=0, j=0, axis;

    delta_x = x2 - x1;
    delta_y = y2 - y1;

    //Ԥ����,ȡ���������С���Ǹ���Ϊ���
    if(fabsf(delta_x) < fabsf(delta_y)){//y��Ϊ����
        axis = 1;//y��
        if(delta_y < 0){
            y_start = y2;
            x_start = x2;
            polarity = x1>x2 ? 1 : -1;//����
        }else{
            y_start = y1;
            x_start = x1;
            polarity = x2>x1 ? 1 : -1;//����
        }
    }else{  //��x��Ϊ����
        axis = 0;//x��
        if(delta_x < 0){
            x_start = x2;
            y_start = y2;
            polarity = y1>y2 ? 1 : -1;//����
        }else{
            x_start = x1;
            y_start = y1;
            polarity = y2>y1 ? 1 : -1;//����
        }
    }

    delta_y = fabsf(delta_y);
    delta_x = fabsf(delta_x);


    if(axis){//��y��Ϊ����
        error = -1*delta_y;
        for(i=0; i<=delta_y; i++){
            uiDisplayDrawPoint( x_start + polarity*j , y_start + i);//����
            if(error >= 0)
            {
                j++;
                error -= 2*delta_y;
            }
            error += 2*delta_x;
        }

    }else{//��x��Ϊ����
        error = -1*delta_x;
        for(i=0; i<=delta_x; i++){
            uiDisplayDrawPoint( x_start + i , y_start + polarity*j);//����
            if(error >= 0){
               j++;
               error -= 2*delta_x;
           }
           error += 2*delta_y;
        }
    }
}

//����
void uiDisplayDrawFrame(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    uiDisplayDrawLine(x1,y1,x1,y2);
    uiDisplayDrawLine(x1,y1,x2,y1);
    uiDisplayDrawLine(x1,y2,x2,y2);
    uiDisplayDrawLine(x2,y1,x2,y2);
}

//�������
void uiDisplayDrawBlock(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    //�����pageֱ��д�Դ棬���±�Ե�ú��߲���
    uint8_t y_min, y_max, page_address, column_address, width,i,j;
    if(y1>y2){
        y_max = y1;
        y_min = y2;
    }else{
        y_min = y1;
        y_max = y2;
    }

    if(x1<x2){
        column_address = x1;
        width = x2-x1;
    }else{
        column_address = x2;
        width = x1-x2;
    }

    if( (y_min + 7)/8 < y_max/8 ){//�����������ڴ�
        for(page_address = (y_min + 7)/8 ; page_address < y_max/8 ; page_address++){
            for(i = 0 ; i <= width ; i++){
                GRAM[page_address][column_address + i] = 0xff;
            }
        }
    }

    for(j = 0; j <= width; j++ ){
        for(i = y_min ; i<((y_min + 7)/8)*8 ; i++){
            uiDisplayDrawPoint(column_address + j , i);
        }

        for(i = (y_max/8)*8 ; i <= y_max ; i++){
            uiDisplayDrawPoint(column_address + j , i);
        }
    }
}

//�������
void uiDisplayEraseBlock(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    //�����pageֱ��д�Դ棬���±�Ե�ú��߲���
    uint8_t y_min, y_max, page_address, column_address, width,i,j;
    if(y1>y2){
        y_max = y1;
        y_min = y2;
    }else{
        y_min = y1;
        y_max = y2;
    }

    if(x1<x2){
        column_address = x1;
        width = x2-x1;
    }else{
        column_address = x2;
        width = x1-x2;
    }

    if( (y_min + 7)/8 < y_max/8 ){//�����������ڴ�
        for(page_address = (y_min + 7)/8 ; page_address < y_max/8 ; page_address++){
            for(i = 0 ; i <= width ; i++){
                GRAM[page_address][column_address + i] = 0x00;
            }
        }
    }

    for(j = 0; j <= width; j++ ){
        for(i = y_min ; i<((y_min + 7)/8)*8 ; i++){
            uiDisplayErasePoint(column_address + j , i);
        }

        for(i = (y_max/8)*8 ; i <= y_max ; i++){
            uiDisplayErasePoint(column_address + j , i);
        }
    }
}

//��Բ(Bresenham�㷨)
void uiDisplayDrawCircle(uint8_t x0 , uint8_t y0 , uint8_t radius )
{
    uint8_t width , i = 0 , j = radius;

    width = (uint8_t)(0.7072f * radius);

    for(i = 0; i<= width; i++){
        uiDisplayDrawPoint(x0 + i, y0 + j);
        uiDisplayDrawPoint(x0 + j, y0 + i);
        uiDisplayDrawPoint(x0 + j, y0 - i);
        uiDisplayDrawPoint(x0 + i, y0 - j);
        uiDisplayDrawPoint(x0 - i, y0 - j);
        uiDisplayDrawPoint(x0 - j, y0 - i);
        uiDisplayDrawPoint(x0 - j, y0 + i);
        uiDisplayDrawPoint(x0 - i, y0 + j);

        //�ж�(i+1,j)��(i+1,j-1)���������е���һ���㵽���ĵľ����radius��ӽ�
        if(radius*radius < (i+1)*i + (j-1)*j + 1){
            j--;
        }
    }
}

//��ʾһ���ַ�
void uiDisplayShowChar(uint8_t x , uint8_t y , uiFont_Enum font , char ascii)
{
    uint8_t label, size, temp, i, j , height, y0, *font_ptr;

    label = ascii - ' ';//�ֿ�� ' ' ���ſ�ʼȡģ���˴�����±�

    if(label > 94){//���ǿ���ʾ���ַ�
        label = 31;//��ʾ'?'��
    }

    y0 = y;//�����ֵ

    switch(font){
        case FONT1206:
            font_ptr = (uint8_t*)&asc2_1206[0][0];
            height = 12;
            size   = 12;
            break;
        case FONT1608:
            font_ptr = (uint8_t*)&asc2_1608[0][0];
            height = 16;
            size = 16;
            break;
        case FONT2412:
            font_ptr = (uint8_t*)&asc2_2412[0][0];
            height = 24;
            size = 36;
            break;
        default:return;//�ֿⲻ����
    }

    for(i = 0 ; i < size ; i++)//д�ֽ�
    {
        temp = *(font_ptr + label*size + i);
        for(j = 0 ; j < 8 ; j++){//дλ
            if(temp&0x80){
                uiDisplayDrawPoint(x,y);
            }
            temp <<= 1;
            y++;
            if(y - y0 == height){
                y = y0;
                x++;
                break;//������д�꣬���һ���ֽ����û�����꣬˵����һ�ֽڱ����ֻ�и�4λ����
            }
        }
    }
}

//��ʾ�ַ���
void uiDisplayShowString(uint8_t x , uint8_t y , uiFont_Enum font , char* str)
{
    int n = strlen(str), i, width, x0;
    x0 = x;

    switch(font){
        case FONT1206: width = 6 ;break;
        case FONT1608: width = 8 ;break;
        case FONT2412: width = 12;break;
        default:return;
    }

    for(i = 0; i < n; i++){
        if(*str == '\n'){//�س�
            x = x0;
            y += 2*width;
            str++;
            continue;
        }else if(*str == '\r'){
            x = x0;
            str++;
            continue;
        }else if(*str == '\t'){
            x += 4*width;
            str++;
            continue;
        }
        uiDisplayShowChar(x, y, font, *(str++));
        x += width;
    }
}


//��ʾ���������ɱ���С������λ��
void uiDisplayShowFloat(uint8_t x , uint8_t y , uiFont_Enum font , float data, uint8_t precision)
{
    char str[32];
    memset(str,'\0',32);
    Float2Str(str , data , precision , 32);
    uiDisplayShowString( x , y , font , str);
}


//��ʾ����
void uiDisplayShowNumber(uint8_t x , uint8_t y , uiFont_Enum font , int data)
{
    char str[32];
    memset(str,'\0',32);
    Int2Str(str , data , 32);
    uiDisplayShowString( x , y , font , str);
}




/*ʵ���ڲ�����*/

//��������Ƿ񳬳���Ļ��Ե
static int uiEdgeCheck(int x, int y)
{
    if(y>63){
        return -1;
    }
    if(x>127){
        return -1;
    }
    if(x<0){
        return -1;
    }
    if(y<0){
        return -1;
    }

    return 0;
}
