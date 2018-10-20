#include <key/FSM_key.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "inc/hw_types.h"   //�ṩ��HWREG�ȵĶ�д�Ĵ����õĺ�
#include "inc/hw_memmap.h"  //��������������Ļ���ַ
#include "inc/hw_gpio.h"    //������GPIO�ļĴ�������������ַƫ����

#include "driverlib/sysctl.h" //�ṩ��ϵͳ���ƵĿ⺯��(ʹ������ʱ��0)
#include "driverlib/gpio.h"   //�ṩ��GPIO�Ŀ⺯��


/*����Ӳ����ص��ڲ�����*/
static int  keyGPIOInit(uint32_t Port, uint32_t PinNumber);//����GPIOΪ����
static uint8_t keyGPIORead(uint32_t Port, uint32_t PinNumber);//��ȡGPIO��ƽ

/*����ȫ�ֱ���*/
volatile key_TypeDef *g_first_key = NULL;//��¼��һ�������ṹ��ĵ�ַ

/*ʵ���ⲿ����*/

/*
 * ������ʼ��
 * key      :     �����ṹ��ָ��
 * Port     :     GPIO�˿ڻ���ַ(ת��Ϊuint32_t)
 * PinNumber:     GPIO���ź�(����)
 * Polarity :     ���ԣ�Ӳ���ϣ���������ʱGPIO��������ĵ�ƽ(HIGH��LOW)
 */
void keyInit(key_TypeDef* key,
               uint32_t Port,
               uint8_t PinNumber,
               keyPolarity_Enum Polarity,
               void(*sigleClick)(void),
               void(*doubleClick)(void))
{
    static key_TypeDef* s_previous_key = NULL;//��¼��һ�������ĵ�ַ

    //�������ñ���
    key->port                = Port;
    key->num                 = PinNumber;
    key->polarity            = Polarity;
    key->singleClickCallback = sigleClick;//�����ص�����
    key->doubleClickCallback = doubleClick;//˫���ص�����
    key->state               = NORMAL;
    key->counter             = 0;
    key->sample[0]           = 0;
    key->sample[1]           = 0;
    key->sample[2]           = 0;
    key->sample[3]           = 0;
    key->next                = NULL;

    //����GPIO
    keyGPIOInit(Port, PinNumber);

    if(s_previous_key == NULL){
    //����ǵ�һ������ʼ���İ�����������������
        g_first_key = key;//�������ĵ�ַ��ȫ�ֱ���
    }else{
    //���ǵ�һ������ʼ���İ�������ѱ��������������ӵ���һ��������
        s_previous_key->next = key;
    }

    //ȫ��������ɣ��ѱ������ĵ�ַ�����´�ʹ��
    s_previous_key = key;
}

//ɨ�赥������
void keyScan(key_TypeDef* key)
{
    uint32_t voltage;
    const uint8_t key_event_press[4]    = {1,1,0,0}; //press�¼�
    const uint8_t key_event_realease[4] = {0,0,1,1}; //release�¼�
    bool press_flag = 0 , release_flag = 0;

    //����ɲ���ֵ
    key->sample[3] = key->sample[2];
    key->sample[2] = key->sample[1];
    key->sample[1] = key->sample[0];

    voltage = keyGPIORead(key->port , key->num);

    //�²���ֵ
    if( voltage&&(key->polarity) || (!voltage)&&(!(key->polarity)) ){
        key->sample[0] = 1;
    }else{
        key->sample[0] = 0;
    }

    //�¼�
    if      ( !memcmp(key->sample, key_event_press, 4) ){
        press_flag   = 1;
    }else if(!memcmp(key->sample, key_event_realease, 4)){
        release_flag = 1;
    }

    //״̬��
    switch(key->state){
        case NORMAL:
            if(press_flag){
                key->state = PRE_ACTIVE;
                key->counter = 0;
            }
            break;


        case PRE_ACTIVE:
            if(key->counter <4){//40ms��
                if(release_flag){//40ms���ͷ�
                    key->state   = PRE_HIT;
                    key->counter = 0;
                    break;
                }
                key->counter++;
            }else{//40ms��δ�ͷ�
                key->state   = PRE_LONG_PRESS;
                key->counter = 0;
            }
            break;

        case PRE_LONG_PRESS:
            if(key->counter < 100){//1s��
                if(release_flag){//���ͷ���
                    key->state   = PRE_HIT;//�㵥��(��˫��)
                    key->counter = 0;
                    break;
                }
                key->counter++;
            }else{//������1s
                key->state   = LONG_PRESS;
                key->counter = 0;
            }
            break;

        case PRE_HIT:
            if(key->counter < 30){//300ms��
                if(press_flag){//�ְ���һ��
                    key->state   = DOUBLE_HIT;
                    key->counter = 0;
                    break;
                }
                key->counter++;
            }else{//������300ms
                key->state   = SINGLE_HIT;
                key->counter = 0;
            }
            break;

        case SINGLE_HIT:
            //ִ�е����ص�����
            if(key->singleClickCallback != NULL){
                key->singleClickCallback();
            }
            key->state = NORMAL;
            break;

        case DOUBLE_HIT:
            //ִ��˫���ص�����
            if(key->doubleClickCallback != NULL){
                key->doubleClickCallback();
            }else if(key->singleClickCallback != NULL){//��˫���ص�����δע�ᣬ��ִ�����ε����ص�����
                key->singleClickCallback();
                key->singleClickCallback();
            }
            key->state = NORMAL;
            break;

        case LONG_PRESS:
            //����״̬�£�ÿ0.5sִ��һ�ε����ص�������ѭ��ִ��
            if(release_flag){
                key->state = NORMAL;
                key->counter = 0;
                break;
            }
            if(!(key->counter%50)){//����0.5s
                //ִ�е����ص�����
                if(key->singleClickCallback != NULL){
                    key->singleClickCallback();
                }
            }
            key->counter++;
            key->counter %= 1000;
            break;

        default:
            break;
    }
}

//ɨ��ȫ�尴��
void keyScanAll(void)
{
    key_TypeDef* key_pointer = (key_TypeDef*)g_first_key;

    while(1)
    {
        if(key_pointer != NULL)
        {
            keyScan(key_pointer);//ɨ�赥������
            key_pointer = key_pointer->next;//�������ƶ�����һ������
        }else{
            break;
        }
    }
    return;
}



/*ʵ���ڲ�����*/

//GPIO��ȡ��ѹ
static uint8_t keyGPIORead(uint32_t Port, uint32_t PinNumber)//��ȡGPIO��ƽ
{
    uint8_t voltage = 0;
    voltage = GPIOPinRead(Port , 0x00000001<<PinNumber);

    return voltage;
}

//GPIO���ú���
static int keyGPIOInit(uint32_t Port, uint32_t PinNumber)
{
    uint32_t sysctl_periph_base;//ϵͳ�����������ַ
    uint8_t unlock_flag = 0;//�Ƿ����

    //����ֻ������GPIO���������ַ����Ҫ�Լ��Ƶ���ʱ��ʹ����Ҫ��ֵ
    switch(Port){
        case GPIO_PORTA_BASE     :
        case GPIO_PORTA_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOA; break;
        case GPIO_PORTB_BASE:
        case GPIO_PORTB_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOB; break;
        case GPIO_PORTC_BASE:
        case GPIO_PORTC_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOC; break;
        case GPIO_PORTD_BASE:
        case GPIO_PORTD_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOD; break;
        case GPIO_PORTE_BASE:
        case GPIO_PORTE_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOE; break;
        case GPIO_PORTF_BASE:
        case GPIO_PORTF_AHB_BASE : sysctl_periph_base = SYSCTL_PERIPH_GPIOF; break;
        default: return -1;//��������
    }

    //����ʹ��
    SysCtlPeripheralEnable(sysctl_periph_base);

    //�ж��Ƿ����
    switch(Port){
        default:
            unlock_flag = 0;
            break;
        case  GPIO_PORTC_BASE   :
            if(PinNumber < 4){//PC[3:0]������
                unlock_flag = 1;
            }
            break;
        case  GPIO_PORTC_AHB_BASE   :
            if(PinNumber < 4){//PC[3:0]������
                unlock_flag = 1;
            }
            break;
        case  GPIO_PORTD_BASE   :
            if(PinNumber == 7){//PD7������
                unlock_flag = 1;
            }
            break;
        case  GPIO_PORTD_AHB_BASE   :
            if(PinNumber == 7){//PD7������
                unlock_flag = 1;
            }
            break;
        case  GPIO_PORTF_BASE   :
            if(PinNumber == 0){//PF0������
                unlock_flag = 1;
            }
            break;
        case  GPIO_PORTF_AHB_BASE   :
            if(PinNumber == 0){//PF0������
                unlock_flag = 1;
            }
            break;
    }

    //����
    if(unlock_flag){
        HWREG(Port + GPIO_O_LOCK) = GPIO_LOCK_KEY;
        HWREG(Port + GPIO_O_CR) |= 0x01<<PinNumber;
        HWREG(Port + GPIO_O_LOCK) = 0;
    }

    //GPIO����Ϊ����ģʽ
    GPIOPinTypeGPIOInput(Port, 0x00000001<<PinNumber);//GPIO����Ϊ����

    return 0;
}


