#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ui/str_convert.h>

/*�����ڲ�����*/
static int Float2Str_NoScientificNotation(char* str, float data, int precision, int length);//��֧�ֿ�ѧ�������ĸ�����ת�ַ���


/*ʵ���ⲿ����*/

/*
 * ������ת�ַ���, ����ֵΪת�����ַ������ȡ�
 * ������10^4��С��10^-4�����Զ����ÿ�ѧ������
 * ���������Ȳ�������ʾ�������ַ����������ʾһ����X��;
 *
 * ����:
 *      precision:���ȣ�С�����nλ
 *      length   :�ַ�������󳤶ȣ���ֹ���
 *
 * ����ֵ:
 *      ת������ַ����ĳ���
 */
int Float2Str(char* str, float data, int precision, int length)
{
    uint16_t N = 0, exp_length = 0;
    int i = 0;
    int exp = 0 , exp_temp = 0;//��ѧ������ָ��
    uint8_t scientific_notation = 0;//�Ƿ�ʹ�ÿ�ѧ������
    float temp = data ;
    char *p = str;

    //����10000��С��0.0001��ʹ�ÿ�ѧ�����������ָ��
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

    //��ʱ��N���洢������ָ������������ַ���
    if(scientific_notation){
        N++;//��"E"�ַ�
        if( exp < 0){
            N++;//��'-'��
        }
        //ָ������
        exp_temp = exp;
        for( ; exp_temp ; N++){
            exp_temp /= 10;
        }
        length -= N;//���ó��ȼ�С(Ԥ����ָ���Ĳ���)
        exp_length = N - 1;//��ȥ'E'
        if(length <= 0){//��ָ�����ֶ�д���£���ʾһ��'X'
            *p = 'X';
            return 1;
        }
    }

    //��Ϊ�Ѿ�Ԥ����ָ�����֣����Լ�ʹlength��С��0����Ȼ�пռ��ʾָ��
    i = Float2Str_NoScientificNotation( p , temp , precision , length );
    N += i;
    length -= i;
    p += i;

    if(scientific_notation){
    //��ʾָ������
        *(p++) = 'e';
        Int2Str( p , exp , exp_length);
    }
    return N;
}

/*
 * ����ת�ַ�����
 *
 * ����ֵ:
 *      ת������ַ�������
 */
int Int2Str(char* str, int data, int length)
{
    volatile char *p = str;
    volatile int n = 0;//λ��
    volatile int temp = data;//��ʱ����
    volatile int i = 0;
    volatile uint8_t sign = 0;//����λ

    if(length <= 0){
        return 0;
    }

    if(data < 0){
        data *= -1;//ȡ��
        *p = '-';
        p++;
        length--;
        sign++;
        if(!length){
            return 1;
        }
    }

    //��ø�ʮ����������λ��
    do{
        n++;
        temp /= 10;
    }while(temp);//

    temp = data;

    //���������Ȳ���д����ֻ��ʾһ��'X'���˺�������
    if(n > length){
        *(--p) == 'X';
        return 1;
    }

    //ָ���ƶ�����󣬴Ӻ���ǰд
    p += n;
    for(i = 0; i < n ; i++){
        p--;
        *p = '0' + (temp%10);
        temp /= 10;
    }
    n += sign;
    return n;
}



/*ʵ���ڲ�����*/

//��֧�ֿ�ѧ�������ĸ�����ת�ַ���
static int Float2Str_NoScientificNotation(char* str, float data, int precision, int length)
{
    uint32_t fraction;//β��
    uint8_t exponent;//����
    int shift;//�ƶ�С����
    uint32_t fixed_integer;//������������
    uint32_t fixed_fraction;//����С������
//    volatile uint8_t scientific_notation = 0;
    char *p = str;
    uint32_t *p_float = (uint32_t*)&data;//��ָ��ǿ�ƶ�float�������ݵĶ�����
    uint32_t data_bin=0;//���ݶ�����
    uint16_t i = 0;
    uint16_t N = 0;//�Ѿ�д���ַ���
    int temp = 0;//��ʱ����
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
    if(*p_float){//����λ
        *p = '-';
        p++;
        length--;
        N++;
        if(length <= 0){
            return 1;
        }
    }

    shift = exponent - 127;

    fraction |= 0x00800000;//����С�����������ּ���1

/*
    //����2^24��С��2^-32ʱ����ʹ�ÿ�ѧ������
     if(shift > 23 || shift < -9 ){
        //��̫���̫С����ʹ�ÿ�ѧ������;
        scientific_notation = 1;
    }
*/

    //����������ֺͶ���С������
    fixed_integer  = fraction >> (23 - shift);
    fixed_fraction = fraction << (9 + shift); //��ʱС���������λ���

    //С�����֣�ֻȡ��Ҫ�ľ���(4bit����ʾ15���պñ�10��)
   if(precision > 4){
       precision = 4;//û�취֧��̫�ߵľ���(���߾��Ȼ��ÿ�ѧ������)
   }
   for(i = 0 ; i < (32 - 4*precision) ; i++  ){
       fixed_fraction >>= 1;
   }//���ȿ�����ɣ�����С����λ�ڵ�4*precision-1λ�����

   //С������ת��Ϊ�������ౣ��һλ
   for(i = 0 ; i<= precision ; i++){
       fixed_fraction *= 10;
   }
   fixed_fraction >>= 4*precision;

    //С��������������
    if(fixed_fraction % 10 >= 5){
        fixed_fraction += 10;
    }
    fixed_fraction /= 10;

    //�ж�С������������Ƿ��λ����������
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

    //��ת����������Ϊ�ַ���
    temp = Int2Str((char*)p , fixed_integer , length );
    N += temp;
    p += temp;
    length -= temp;
    if(!length){
        return N;
    }

    //дС����
    *p = '.';
    N++;
    p++;
    length--;
    if(!length){
        return N;
    }

    //��ת��С������Ϊ�ַ���
    if(precision <= length){//ʣ�೤�Ȼ���дС��
        length = precision;
    }else{//ʣ�೤�Ȳ���дС����ֱ����ʾ'X'
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
        //δ��ʾ�꣬��0
        while(length--){
            *p = '0';
            p++;
            N++;
        }
    }
    return N;
}
