#ifndef __TOOLKIT_H
#define __TOOLKIT_H

#include "stm32f4xx.h"
#include "soft_encoder.h"
#include "lcd.h"

#define COLOR_CHANGE(color) ((((((color)&0xff0000)>>16)*32/0xff)<<11)|(((((color)&0x00ff00)>>8 )*64/0xff)<<5)|(((color)&0x0000ff)*32/0xff))
//将RGB888转换为RGB565


/******************************************************/
/*      本工具箱目前包含以下功能：                    */
/*          （1）多通道示波器                         */
/*          （2）轻触按键与自锁按键                   */
/*          （3）二维平面坐标显示器                   */
/*          （4）字符串显示器                         */
/*          （5）浮点数显示器                         */
/*          （6）利用编码器输入的浮点数控制器         */
/******************************************************/


void toolkit_Init(u8 dt);
//工具箱初始化,参数为主循环周期(ms)

/**********************************************************/
/*                           示波器                       */
/**********************************************************/

//示波器数据存储
typedef struct{
	float y_min;
	float y_max;
	u16   buffer[500];//可存储500个点
	vu16  i;//每次从该下标开始，向后绘制任意个点
	float *p;//指向原始数据的指针
	u16   COLOR;//曲线的颜色
}_Graph_TypeDef;

void graph_Init(u8 Dt, float min, float max);
	//绘制示波器边框，输入参数(存储时间s，刻度上界，刻度下界)
	//由于存储大小限制，存储时间Dt(单位s)与控制周期(单位ms)的数值之比最大值为0.5，即1000*Dt/dt应当小于500
	//由于绘制时间限制，每绘制一条曲线，控制周期至少为12ms，建议20ms，这样也有50帧.否则每两个周期才能绘制一次，导致绘图时间错误

void graph_Struct_CFG(_Graph_TypeDef* graph, float *p, float min, float max,u16 color);
	//graph结构体初始化,参数为(结构体，被检测数据地址，最大值，最小值,曲线颜色)

void graph_Draw(_Graph_TypeDef* graph);
	//在示波器上显示曲线，每周期调用一次

/************************************************************/



/*******************************************************/
/*                          按键                       */
/*******************************************************/

//按键类型枚举
typedef enum{
	unlock = 0,//轻触按键
	lock   = 1,//自锁按键
}_KeyMode_Enum;

//按键数据存储
typedef struct{
	vu8  sta[4];//触摸屏状态
	vu8  flag;//按键状态，用于自锁开关
	u8   mode;//按键种类,0:轻触开关；1：自锁开关；
	u16  x;
	u16  y;//按键坐标，所有按键均为长50宽50的正方形
	char str[9];//显示在按键上的字符串(长度最多8)
}_Key_TypeDef;

void key_Struct_CFG(_Key_TypeDef* key, u16 x, u16 y, _KeyMode_Enum mode, const char* str);
	//按键结构体初始化,参数(结构体，x,y,按键类型，按键字符串)
	//mode:开关类型，0：轻触开关，1：自锁开关

void key_Scan_All(void);
	//扫描所有按键（每周期调用一次）

u8   key_Check(_Key_TypeDef* key);
	//根据扫描结果，检查按键状态,并进行更改

void key_setState(_Key_TypeDef* key, u8 sta);
	//强行用软件更改按键状态，尤其是自锁开关

/*********************************************************/




/********************************************/
/*              二维坐标显示器              */
/********************************************/

//二维平面坐标显示器数据存储
typedef struct{
	float *tx;
	float *ty;//目标坐标原始数据指针
	float *cx;
	float *cy;//当前坐标原始数据指针
	float history[4];//上一周期的四个坐标值(tx,ty,cx,cy)
	u16 color;//坐标的颜色
}_Coordinates_TypeDef;

void coordinates_Init(u16 x, u16 y, u16 w, u16 h, float min_x, float max_x, float min_y, float max_y, void (*fp)(void));
	//坐标显示器初始化（最后一个参数为背景绘制函数，每次绘制完点后，覆盖绘制背景）

void coordinates_Struct_CFG(_Coordinates_TypeDef *data, float *tx, float *ty, float *cx, float *cy, u16 color);
	//数据结构体配置

void coordinates_Draw(_Coordinates_TypeDef *data);
	//在显示器上显示当前点与目标点，每周期调用一次

/**********************************************/




/********************************************/
/*               字符串显示器               */
/********************************************/

//字符串显示器数据存储
typedef struct{
	u16 x;
	u16 y;//显示器坐标
	u16 len;//字符串长度
	char *str;
	char str_his[25];//记录上一次的字符串
}_StrDisplay_TypeDef;

void strDisplay_Struct_CFG(_StrDisplay_TypeDef *data, u16 x, u16 y, char *str, u16 length, const char* name);
	//结构体配置

void strDisplay_Draw(_StrDisplay_TypeDef *data);
	//更新字符串

/**********************************************/





/********************************************/
/*              浮点数显示器                */
/********************************************/
typedef struct{
	u16 x;
	u16 y;
	u16 len;
	float *p;
	float data_his;//历史数据
}_DataDisplay_TypeDef;

void dataDisplay_Struct_CFG(_DataDisplay_TypeDef *data,u16 x,u16 y,float *p,u16 length,const char* name);
	//结构体配置
	
void dataDisplay_Draw(_DataDisplay_TypeDef *data);
	//更新浮点数显示

/********************************************/





/**************************************************************************/
/*                              数据控制器组                              */
/*           （共2个编码器，每个编码器上可绑定任意数量的浮点数）          */
/**************************************************************************/
//编码器选择
typedef enum{
	encoder0 = 0,
	encoder1 = 1
}_dataController_Enum;

//结构体（单向链表结构）
struct dataController{
	u16 x;
	u16 y;
	u16 len;
	SoftEncoder_TypeDef data_struct;
	float data_his;//记录上一次的数据
	u8 encoder;//记录所属编码器
	struct dataController *next;//指向下一个节点的指针
};
typedef struct dataController _DataController_TypeDef;

void dataController_Init(void);
	//初始化

void dataController_Struct_CFG(_DataController_TypeDef *data,u16 x,u16 y,float *p,u16 length,const char* name,_dataController_Enum encoder,float min,float max);
	//结构体配置

void dataController_KeyScan(void);
	//扫描编码器按键，更新数据

void dataController_Draw(_DataController_TypeDef *data);
	//显示数据到屏幕

/**************************************************************************/





/************************************************辅助函数*****************************************************/
static void draw_frame(u16,u16,u16,u16,u8);//按键UI边缘光影，参数(x_s,y_s,x_e,y_e,flag)，flag为0表示上凸，1表示下凸
static void draw_Grid(void);//示波器绘制网格
static void draw_Current_Point(u16 x,u16 y,u16 color);//绘制棱形
static void draw_Cross(u16 x,u16 y,u16 color);//绘制十字
static u16  coordinate_Transform(float data,s32 min,s32 max,float data_min,float data_max);

#endif

