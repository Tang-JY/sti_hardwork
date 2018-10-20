#include "toolkit.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "touch.h"

vu8 tk_dt=10,tk_Dt=5;//控制周期(ms)与示波器存储时间(s)
vu16 coordinates_range[4];//显示器显示范围
volatile float coordinates_data_range[4];//显示器坐标范围
_DataController_TypeDef *en0 = NULL,*en1 = NULL;//此时正在被编码器绑定的数据
vu8 press_down=0;


void (*coordinates_Pattern)(void) = NULL;//二维坐标平面显示器，背景纹样

void toolkit_Init(u8 dt)
{
	LCD_Init();//LCD初始化
	LCD_Display_Dir(1);//横屏显示
	LCD_Scan_Dir(L2R_U2D);//从左到右，从上到下扫描
	tp_dev.init();
	LCD_DisplayOn();
		
	BACK_COLOR = GRAY;
	LCD_Fill(0,0,800,480,BACK_COLOR);
	tk_dt = dt;
}



//示波器初始化
void graph_Init(u8 Dt,float min,float max)
{
	char str[8]={0};
	u8 i;
	float temp;
    
    tk_Dt = Dt;  //最大显示5s内数据

    LCD_Fill(90,10,90+400,10+300,BLACK);
    draw_frame(90,10,90+400,10+300,1);   
    draw_Grid();
		
	POINT_COLOR =BLACK;
	for(i=0;i<6;i++){//绘制纵坐标刻度
		temp = min + (max - min)*i/5;
		if(temp>=0){
			snprintf(str,8," %.2f",temp);//正数前加空格，以便与负数对齐
		}else{
			snprintf(str,8,"%.2f",temp);
		}
		LCD_ShowString(40,302-60*i,100,100,16,(u8*)str);
	}
		
}

//示波器结构体配置
void graph_Struct_CFG(_Graph_TypeDef* graph, float *p, float min, float max,u16 color)
{
	u16 i;
    graph->p=p;
    graph->i=499;
    graph->y_max = max;
    graph->y_min = min;
    graph->COLOR = color;
	for(i=0;i<500;i++){
		graph->buffer[i]=310;
	}
}

//曲线绘制
void graph_Draw(_Graph_TypeDef* graph)
{
    volatile float temp,temp1;
	vu32 temp2,temp3,temp4;
    u32 i;

    //读本次数据
    temp = *(graph->p);
	
	//限辐
    if(temp > graph->y_max){
		temp = graph->y_max;
	}else if(temp<graph->y_min){
		temp = graph->y_min;
	}
	
    //本次记录
    graph->i += 499;
	graph->i %= 500;//下标前移
	graph->buffer[graph->i] = coordinate_Transform(temp,310,10,graph->y_min,graph->y_max);//存储像素纵坐标
    //graph->buffer[graph->i] = 10 + 300 - ((temp - graph->y_min)*300.0f / (graph->y_max - graph->y_min));
	
	//绘制图像,数组可存储500*tk_dt(ms)的数据
	//故i从0增长到400时，i*(1000*tk_Dt/(500*tk_dt))*499/400可以从0增长到499*(1000*tk_Dt/(500*tk_dt))
    for(i=0;i<=400;i++){
		temp1 = 499.0f*i*tk_Dt/(200.0f*tk_dt);
		temp2 = graph->i + temp1;
		temp3 = graph->buffer[(temp2+1)%500];
		temp4 = graph->buffer[temp2%500];
		LCD_Fast_DrawPoint(490-i,temp3,BLACK);
		LCD_Fast_DrawPoint(490-i,temp4,graph->COLOR);
		//LCD_Fast_DrawPoint(490-i,graph->buffer[((u16)((499.0f*i*tk_Dt)/(200.0f*tk_dt) + graph->i+1))%500],BLACK);	//覆盖掉上次图像
		//LCD_Fast_DrawPoint(490-i,graph->buffer[((u16)((499.0f*i*tk_Dt)/(200.0f*tk_dt) + graph->i  ))%500],graph->COLOR);//绘制新图像		
	}
    draw_Grid();
}

//按键结构体配置,type:开关类型，0：轻触开关，1：自锁开关
void key_Struct_CFG(_Key_TypeDef* key,u16 x,u16 y,_KeyMode_Enum mode,const char* str)
{
	u16 length=0;
	
	key->mode = mode;
	key->flag=0;
	key->x = x;
	key->y = y;
	key->sta[0]=0;
	key->sta[1]=0;
	length = strlen(str);//根据字符串长度判断位置
	memcpy(key->str,str,length+1);
	draw_frame(x,y,x+60,y+60,key->flag);
	LCD_Fill(x,y,x+60,y+60,GRAY);
	POINT_COLOR=BLACK;
	
	if(length<=6){//长度小于等于6，使用16像素高字体（宽8）
		LCD_ShowString(x+30-length*4,y+22,length*8,length*8,16,(u8*)str);
	}else{//否则使用12像素高字体（宽6）
		LCD_ShowString(x+30-length*3,y+24,length*6,length*6,12,(u8*)str);
	}
	
	if(mode){
		LCD_Fill(x+3,y+3,x+6,y+6,BLACK);//自锁开关标记
	}
}

//屏幕按键触摸扫描
void key_Scan_All(void)
{
	press_down = tp_dev.scan(0);
}

//按键检查
u8 key_Check(_Key_TypeDef* key)
{
	vu8 temp;
	
	//存储历史
	key->sta[3] = key->sta[2];
	key->sta[2] = key->sta[1];
	key->sta[1] = key->sta[0];

	//开始检查触摸状态
	if(!press_down){//若屏幕无触摸
		key->sta[0] = 0;//直接判断状态，无需判断坐标
		
	}else{		
		if((tp_dev.x[0] > key->x) && (tp_dev.y[0]>key->y) && (tp_dev.x[0]<key->x+60) && (tp_dev.y[0]<key->y+60)){
			key->sta[0] = 1;
		}else{
			key->sta[0] = 0;
		}
	}
	
	//开始处理触摸结果
	if(!(key->sta[3]) && !(key->sta[2]) && key->sta[1] && key->sta[0])//按键按下事件
	{	
		if(!(key->mode)){//轻触开关	
			key->flag = 1;
			draw_frame(key->x,key->y,key->x+60,key->y+60,1);
			return 1;
		}else{ //自锁开关
			temp = !key->flag;
			key->flag = temp;//取反
			draw_frame(key->x,key->y,key->x+60,key->y+60,temp);
			return temp;
		}
		
	}else if(key->sta[3] && key->sta[2] && !key->sta[1] && !key->sta[0]){//按键松开事件，仅对轻触开关有用
	
		if(!key->mode){
			key->flag = 0;
			draw_frame(key->x,key->y,key->x+60,key->y+60,0);
			return 0;
		}
		
	}else{ //正在按住 或 未触摸 或 噪声
	
		if(!key->mode){//轻触开关
			return 0;
		}else{
			temp = key->flag;
			return temp;//自锁开关
		}
	}
	temp = key->flag;
	return temp;
}

void key_setState(_Key_TypeDef* key,u8 sta)
{
	key->flag = sta;
	draw_frame(key->x,key->y,key->x+60,key->y+60,key->flag);
}

//二维平面坐标显示器初始化
void coordinates_Init(u16 x,u16 y,u16 w,u16 h,float min_x,float max_x,float min_y,float max_y,void (*fp)(void))
{
	draw_frame(x-5,y-5,x+w+5,y+h+5,1);
	LCD_Fill(x-5,y-5,x+w+5,y+h+5,BLACK);
	coordinates_range[0] = x;
	coordinates_range[1] = x+w;
	coordinates_range[2] = y;
	coordinates_range[3] = y+h;
	coordinates_data_range[0] = min_x;
	coordinates_data_range[1] = max_x;
	coordinates_data_range[2] = min_y;
	coordinates_data_range[3] = max_y;
	coordinates_Pattern = fp;
	if(coordinates_Pattern != NULL){
		coordinates_Pattern();
	}
}


//二维平面坐标显示器配置
void coordinates_Struct_CFG(_Coordinates_TypeDef *data,float *tx,float *ty,float *cx,float *cy,u16 color)
{
	data->tx = tx;
	data->ty = ty;
	data->cx = cx;
	data->cy = cy;
	data->color = color;
	memset(data->history,0,4);
}

//绘制当前点与目标点
void coordinates_Draw(_Coordinates_TypeDef *data)
{
	u16 x,y;
	
	//cx限辐
	if(*(data->cx) > coordinates_data_range[1]){
		*(data->cx) = coordinates_data_range[1];
	}else if(*(data->cx) < coordinates_data_range[0]){
		*(data->cx) = coordinates_data_range[0];
	}
	
	//cy限辐
	if(*(data->cy) > coordinates_data_range[3]){
		*(data->cy) = coordinates_data_range[3];
	}else if(*(data->cy) < coordinates_data_range[2]){
		*(data->cy) = coordinates_data_range[2];
	}
	
	//tx限辐
	if(*(data->tx) > coordinates_data_range[1]){
		*(data->tx) = coordinates_data_range[1];
	}else if(*(data->tx) < coordinates_data_range[0]){
		*(data->tx) = coordinates_data_range[0];
	}
	
	//ty限辐
	if(*(data->ty) > coordinates_data_range[3]){
		*(data->ty) = coordinates_data_range[3];
	}else if(*(data->ty) < coordinates_data_range[2]){
		*(data->ty) = coordinates_data_range[2];
	}
	
	
	//涂抹掉上次的点
	POINT_COLOR = BLACK;
	x = coordinate_Transform(data->history[0],coordinates_range[0],coordinates_range[1],coordinates_data_range[0],coordinates_data_range[1]);
	y = coordinate_Transform(data->history[1],coordinates_range[3],coordinates_range[2],coordinates_data_range[2],coordinates_data_range[3]);
	//LCD_DrawRectangle(x-2,y-2,x+2,y+2);
	draw_Cross(x,y,BLACK);		
	
	x = coordinate_Transform(data->history[2],coordinates_range[0],coordinates_range[1],coordinates_data_range[0],coordinates_data_range[1]);
	y = coordinate_Transform(data->history[3],coordinates_range[3],coordinates_range[2],coordinates_data_range[2],coordinates_data_range[3]);
	draw_Current_Point(x,y,BLACK);
	
	
	//绘制目标点
	POINT_COLOR = data->color;
	data->history[0] = *(data->tx);
	data->history[1] = *(data->ty);
	x = coordinate_Transform(data->history[0],coordinates_range[0],coordinates_range[1],coordinates_data_range[0],coordinates_data_range[1]);
	y = coordinate_Transform(data->history[1],coordinates_range[3],coordinates_range[2],coordinates_data_range[2],coordinates_data_range[3]);
	draw_Cross(x,y,data->color);

	
	//绘制当前坐标点
	data->history[2] = *(data->cx);
	data->history[3] = *(data->cy);
	x = coordinate_Transform(data->history[2],coordinates_range[0],coordinates_range[1],coordinates_data_range[0],coordinates_data_range[1]);
	y = coordinate_Transform(data->history[3],coordinates_range[3],coordinates_range[2],coordinates_data_range[2],coordinates_data_range[3]);
	draw_Current_Point(x,y,data->color);
	

	//绘制区域背景纹样
	if(coordinates_Pattern != NULL){
		coordinates_Pattern();
	}
}

//字符串显示器数据结构体配置
void strDisplay_Struct_CFG(_StrDisplay_TypeDef *data,u16 x,u16 y,char *str,u16 length,const char* name)
{
	u8 tmp = strlen(name);
	draw_frame(x,y,x+length*8+4,y+20,1);
	LCD_Fill(x,y,x+length*8+4,y+20,BLACK);
	POINT_COLOR = BLACK;
	LCD_ShowString(x-8*tmp-10,y+2,tmp*8,16,16,(u8*)name);
	data->len = length;
	data->str = str;
	data->x = x;
	data->y = y;
}

//字符串显示  
void strDisplay_Draw(_StrDisplay_TypeDef *data)
{
	u8 tmp = strcmp(data->str,data->str_his);
	if(tmp){		
		LCD_Fill(data->x,data->y,data->x+data->len*8+4,data->y+20,BLACK);
		POINT_COLOR = GREEN;
		
		LCD_ShowString(data->x+2,data->y+2,8*data->len,16,16,(u8*)(data->str));
		memset(data->str_his,'\0',25);
		strcpy(data->str_his,data->str);//记录本次数据
	}
}

//浮点数显示器数据结构体配置
void dataDisplay_Struct_CFG(_DataDisplay_TypeDef *data,u16 x,u16 y,float *p,u16 length,const char* name)
{
	u8 tmp = strlen(name);
	draw_frame(x,y,x+length*8+4,y+20,1);
	LCD_Fill(x,y,x+length*8+4,y+20,BLACK);
	POINT_COLOR = BLACK;
	LCD_ShowString(x-8*tmp-10,y+2,tmp*8,16,16,(u8*)name);
	if(length<=32){
		data->len = length;
	}else{
		data->len =32;
	}
	data->p = p;
	data->x = x;
	data->y = y;
}

//浮点数显示
void dataDisplay_Draw(_DataDisplay_TypeDef *data)
{
	char temp[32]={0};
	u8 offset=0;
	if(*(data->p)!=data->data_his){		
		data->data_his = *(data->p);//记录历史数据
		LCD_Fill(data->x,data->y,data->x+data->len*8+4,data->y+20,BLACK);
		POINT_COLOR = GREEN;
		snprintf(temp,data->len,"%.2f",*(data->p));
		if(*(data->p)>0.0f){
			offset = 8;
		}
		LCD_ShowString(data->x+2+offset,data->y+2,8*data->len,16,16,(u8*)temp);
	}
}

//数据控制器初始化
void dataController_Init(void)
{
	softEncoder_Init();
}

//数据控制器结构体配置
void dataController_Struct_CFG(_DataController_TypeDef *data,u16 x,u16 y,float *p,u16 length,const char* name,_dataController_Enum encoder,float min,float max)
{
	static u16 num0=0,num1=0;//记录每个控制器组内部受控数据的个数
	static _DataController_TypeDef *tmp0 = NULL, *tmp1 = NULL, *first0 = NULL, *first1=NULL;//用于记录上一个结构体的地址和第一个结构体的地址
	
	u8 name_len = strlen(name);
	char str[32]={0};
	//绘制边框和变量名
	draw_frame(x,y,x+length*8+4,y+20,1);
	LCD_Fill(x,y,x+length*8+4,y+20,BLACK);
	POINT_COLOR = BLACK;
	LCD_ShowString(x-8*name_len-10,y+2,name_len*8,16,16,(u8*)name);

	snprintf(str,data->len,"%.4f",*(data->data_struct.data));	
	POINT_COLOR = YELLOW;
	LCD_ShowString(data->x+2,data->y+2,8*data->len,16,16,(u8*)str);
	
	
	//存储数据
	if(length<=32){
		data->len = length;
	}else{
		data->len = 32;
	}
	data->x = x;
	data->y = y;
	softEncoder_CFG(&(data->data_struct),p,min,max);
	
	if(!encoder){//绑定到编码器0	
		
		data->encoder = 0;
		if(!num0){//是该组的第一个				
			LCD_Fill(data->x - 10, data->y + 8, data->x - 6, data->y + 12,BLACK);
			softEncoder_Binding(0,&(data->data_struct));
			en0 = data;
			first0 = data;
			tmp0 = data;			
		}else{			
			if(tmp0!=0){
				tmp0->next = data;//上一个节点的next指针指向本节点
			}			
		}
		data->next = first0;//本节点的next指针指向首节点（即使本节点也是首节点）
		tmp0 = data;//保存本节点的指针供下一节点使用
		num0++;
		
	}else{//绑定到编码器1
	
		data->encoder = 1;
		if(!num1){//是该组的第一个
			LCD_Fill(data->x - 10, data->y + 8, data->x - 6, data->y + 12,BLACK);
			softEncoder_Binding(1,&(data->data_struct));
			en1 = data;
			first1 = data;
			tmp1 = data;
		}else{
			if(tmp1!=0){
				tmp1->next = data;//上一个节点的next指针指向本节点
			}		
		}
		data->next = first1;//本节点的next指针指向首节点
		tmp1 = data;//保存本节点的指针供下一节点使用
		num1++;
	}
}

//扫描编码器按键，以修改被控的数据
void dataController_KeyScan(void)
{
	if(softEncoder_GetKeyState(0)){
		LCD_Fill(en0->x - 10, en0->y + 8, en0->x - 6, en0->y + 12,BACK_COLOR);
		softEncoder_Binding(0,&(en0->next->data_struct));
		en0 = en0->next;
		LCD_Fill(en0->x - 10, en0->y + 8, en0->x - 6, en0->y + 12,BLACK);
	}
	
	if(softEncoder_GetKeyState(1)){
		LCD_Fill(en1->x - 10, en1->y + 8, en1->x - 6, en1->y + 12,BACK_COLOR);
		softEncoder_Binding(1,&(en1->next->data_struct));
		en1 = en1->next;
		LCD_Fill(en1->x - 10, en1->y + 8, en1->x - 6, en1->y + 12,BLACK);
	}
	softEncoder_UpdateEncoderState();
}

//数据控制器绘制
void dataController_Draw(_DataController_TypeDef *data)
{
	char str[32];
	if(*(data->data_struct.data) != data->data_his){//若本次数据与上次数据不同，则刷新，否则跳过以节约资源
		LCD_Fill(data->x,data->y,data->x+data->len*8+4,data->y+20,BLACK);		
		snprintf(str,data->len,"%.4f",*(data->data_struct.data));
		
		POINT_COLOR = YELLOW;
		LCD_ShowString(data->x+2,data->y+2,8*data->len,16,16,(u8*)str);
		data->data_his = *(data->data_struct.data);//记录本次数据	
	}
}






//辅助函数
static void draw_frame(u16 x_s,u16 y_s,u16 x_e,u16 y_e,u8 flag)
{
    u16 color1,color2;
	if(!flag){
		color1 =  COLOR_CHANGE(0xbbbbbb);//亮色
		color2 = COLOR_CHANGE(0x696969);//暗色
	}else{
		color2 =  COLOR_CHANGE(0xbbbbbb);//亮色
		color1 = COLOR_CHANGE(0x696969);//暗色
	}   

	POINT_COLOR = color1;
	LCD_Fill(x_s-3,y_s-3,x_e+3,y_s-1,POINT_COLOR);
	LCD_Fill(x_s-3,y_s  ,x_s-1,y_e+3,POINT_COLOR);
	POINT_COLOR = color2;
	LCD_DrawLine(x_s-3,y_e+3,x_e+3,y_e+3);
	LCD_DrawLine(x_s-2,y_e+2,x_e+2,y_e+2);
	LCD_DrawLine(x_s-1,y_e+1,x_e+1,y_e+1);
	LCD_DrawLine(x_e+3,y_s-3,x_e+3,y_e+3);
	LCD_DrawLine(x_e+2,y_s-2,x_e+2,y_e+2);
	LCD_DrawLine(x_e+1,y_s-1,x_e+1,y_e+1); 
}

static void draw_Grid()
{
	u8 i;
	POINT_COLOR = COLOR_CHANGE(0x009900);
	LCD_DrawLine(90,10 ,490,10 );
	LCD_DrawLine(90,70 ,490,70 );
	LCD_DrawLine(90,130,490,130);
	LCD_DrawLine(90,190,490,190);
	LCD_DrawLine(90,250,490,250);
	LCD_DrawLine(90,310,490,310);
    for(i=0;i<=tk_Dt;i++){
		LCD_DrawLine(90+(490-90)*i/tk_Dt,10,90+(490-90)*i/tk_Dt,310);		
	}
}

static void draw_Current_Point(u16 x,u16 y,u16 color)
{
	POINT_COLOR = color;
	LCD_DrawLine(x-3,y,x,y-3);
	LCD_DrawLine(x,y-3,x+3,y);
	LCD_DrawLine(x+3,y,x,y+3);
	LCD_DrawLine(x,y+3,x-3,y);
}

static void draw_Cross(u16 x,u16 y,u16 color)
{
	POINT_COLOR = color;
	LCD_DrawLine(x-3,y,x+3,y);
	LCD_DrawLine(x,y-3,x,y+3);
}

static u16 coordinate_Transform(float data,s32 min,s32 max,float data_min,float data_max)
{
	float temp,width;
	u16 result;
	width = max-min;
	temp = (data-data_min)/(data_max-data_min);
	result = temp*width + min;
	return result;
}
