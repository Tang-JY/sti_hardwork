#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ui/str_convert.h>

/*声明内部函数*/
static int Float2Str_NoScientificNotation(char* str, float data, int precision, int length);//不支持科学记数法的浮点数转字符串


/*实现外部函数*/

/*
 * 浮点数转字符串, 返回值为转换的字符串长度。
 * 若大于10^4或小于10^-4，则自动采用科学记数法
 * 若所给长度不足以显示完整的字符串，则会显示一个‘X’;
 *
 * 参数:
 *      precision:精度，小数点后n位
 *      length   :字符串的最大长度，防止溢出
 *
 * 返回值:
 *      转换后的字符串的长度
 */
int Float2Str(char* str, float data, int precision, int length)
{
    uint16_t N = 0, exp_length = 0;
    int i = 0;
    int exp = 0 , exp_temp = 0;//科学记数法指数
    uint8_t scientific_notation = 0;//是否使用科学记数法
    float temp = data ;
    char *p = str;

    //大于10000或小于0.0001则使用科学记数法。求出指数
    if(fabsf(temp) <= 0.0001f && temp != 0){
        scientific_notation = 1;
        for(exp = 0; fabsf(temp) < 1.0f ;exp--){
            temp *= 10.0f;
        }
    }else if(fabsf(temp) >= 10000.0f){
        scientific_notation = 1;
        for(exp = 0; fabsf(temp) >= 10.0f ; exp++){
            temp /= 10.0f;
        }
    }

    //暂时用N来存储记数法指数部分所需的字符数
    if(scientific_notation){
        N++;//存"E"字符
        if( exp < 0){
            N++;//存'-'号
        }
        //指数部分
        exp_temp = exp;
        for( ; exp_temp ; N++){
            exp_temp /= 10;
        }
        length -= N;//可用长度减小(预留了指数的部分)
        exp_length = N - 1;//减去'E'
        if(length <= 0){//仅指数部分都写不下，显示一个'X'
            *p = 'X';
            return 1;
        }
    }

    //因为已经预留了指数部分，所以即使length减小到0，仍然有空间表示指数
    i = Float2Str_NoScientificNotation( p , temp , precision , length );
    N += i;
    length -= i;
    p += i;

    if(scientific_notation){
    //显示指数部分
        *(p++) = 'e';
        Int2Str( p , exp , exp_length);
    }
    return N;
}

/*
 * 整数转字符串。
 *
 * 返回值:
 *      转换后的字符串长度
 */
int Int2Str(char* str, int data, int length)
{
    volatile char *p = str;
    volatile int n = 0;//位数
    volatile int temp = data;//临时数据
    volatile int i = 0;
    volatile uint8_t sign = 0;//符号位

    if(length <= 0){
        return 0;
    }

    if(data < 0){
        data *= -1;//取反
        *p = '-';
        p++;
        length--;
        sign++;
        if(!length){
            return 1;
        }
    }

    //获得该十进制整数的位数
    do{
        n++;
        temp /= 10;
    }while(temp);//

    temp = data;

    //若所给长度不够写，则只显示一个'X'，此函数结束
    if(n > length){
        *(--p) == 'X';
        return 1;
    }

    //指针移动到最后，从后往前写
    p += n;
    for(i = 0; i < n ; i++){
        p--;
        *p = '0' + (temp%10);
        temp /= 10;
    }
    n += sign;
    return n;
}



/*实现内部函数*/

//不支持科学记数法的浮点数转字符串
static int Float2Str_NoScientificNotation(char* str, float data, int precision, int length)
{
    uint32_t fraction;//尾数
    uint8_t exponent;//阶码
    int shift;//移动小数点
    uint32_t fixed_integer;//定点整数部分
    uint32_t fixed_fraction;//定点小数部分
//    volatile uint8_t scientific_notation = 0;
    char *p = str;
    uint32_t *p_float = (uint32_t*)&data;//用指针强制读float类型数据的二进制
    uint32_t data_bin=0;//数据二进制
    uint16_t i = 0;
    uint16_t N = 0;//已经写的字符数
    int temp = 0;//临时变量
    int temp2;

    data_bin = *p_float;
    p_float = &data_bin;


    if(length<=0){
        return 0;
    }

    fraction  = *p_float & 0x007fffff;
    *p_float    >>= 23;
    exponent  = *p_float & 0x000000ff;

    *p_float >>= 8;
    if(*p_float){//符号位
        *p = '-';
        p++;
        length--;
        N++;
        if(length <= 0){
            return 1;
        }
    }

    shift = exponent - 127;

    fraction |= 0x00800000;//定点小数的整数部分加上1

/*
    //大于2^24或小于2^-32时，需使用科学记数法
     if(shift > 23 || shift < -9 ){
        //数太大或太小，需使用科学记数法;
        scientific_notation = 1;
    }
*/

    //获得整数部分和定点小数部分
    fixed_integer  = fraction >> (23 - shift);
    fixed_fraction = fraction << (9 + shift); //此时小数点在最高位左侧

    //小数部分，只取需要的精度(4bit最大表示15，刚好比10大)
   if(precision > 4){
       precision = 4;//没办法支持太高的精度(更高精度会用科学记数法)
   }
   for(i = 0 ; i < (32 - 4*precision) ; i++  ){
       fixed_fraction >>= 1;
   }//精度控制完成，现在小数点位于第4*precision-1位的左侧

   //小数部分转换为整数，多保留一位
   for(i = 0 ; i<= precision ; i++){
       fixed_fraction *= 10;
   }
   fixed_fraction >>= 4*precision;

    //小数部分四舍五入
    if(fixed_fraction % 10 >= 5){
        fixed_fraction += 10;
    }
    fixed_fraction /= 10;

    //判断小数四舍五入后，是否进位到整数部分
    temp  = fixed_fraction;
    temp2 = 1;
    for(i = 0 ; i< precision ; i++){
        temp  /= 10;
        temp2 *= 10;
    }
    if(temp){
        fixed_integer++;
        fixed_fraction %= temp2;
    }

    //先转换整数部分为字符串
    temp = Int2Str((char*)p , fixed_integer , length );
    N += temp;
    p += temp;
    length -= temp;
    if(!length){
        return N;
    }

    //写小数点
    *p = '.';
    N++;
    p++;
    length--;
    if(!length){
        return N;
    }

    //再转换小数部分为字符串
    if(precision <= length){//剩余长度还够写小数
        length = precision;
    }else{//剩余长度不够写小数，直接显示'X'
        N++;
        *p = 'X';
        return N;
    }
    temp = Int2Str((char*)p , fixed_fraction , length);
    N += temp;
    p += temp;
    length -= temp;
    if(!length){
        return N;
    }else{
        //未显示完，补0
        while(length--){
            *p = '0';
            p++;
            N++;
        }
    }
    return N;
}
