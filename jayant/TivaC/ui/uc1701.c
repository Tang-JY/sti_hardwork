#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "ui/hw_uc1701.h"
#include "ui/uc1701.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/ssi.h"

//#define   LCD_PIN_SPI_CLK   GPIO_PORTB_BASE  //PB4
#define   LCD_PIN_SPI_CS    GPIO_PORTA_BASE  //PA5
//#define   LCD_PIN_SPI_MOSI  GPIO_PORTB_BASE  //PB7
#define   LCD_PIN_CD        GPIO_PORTB_BASE  //PB3
//#define   LCD_PIN_RESET     GPIO_PORTA_BASE  //RST

const unsigned char Ascii[]={
/*   */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* ! */
0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* " */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,
/* # */
0x00,0x00,0x00,0xC0,0x30,0xC0,0x30,0x00,0xC8,0x39,0xCF,0x39,0x0F,0x09,0x01,0x00,
/* $ */
0xE0,0x10,0x10,0xF8,0x10,0x10,0x60,0x00,0x30,0x41,0x42,0xFF,0x42,0x44,0x38,0x00,
/* % */
0xC0,0x20,0x10,0xE0,0x20,0xE0,0x18,0x00,0x01,0xC2,0x31,0x0C,0x73,0x88,0x44,0x38,
/* & */
0x00,0xE0,0x10,0x10,0xE0,0x00,0x00,0x00,0x78,0x85,0x82,0x8D,0x70,0xA2,0x9E,0x42,
/* ' */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* ( */
0xC0,0x30,0x08,0x04,0x00,0x00,0x00,0x00,0x1F,0x60,0x80,0x00,0x00,0x00,0x00,0x00,
/* ) */
0x04,0x08,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x1F,0x00,0x00,0x00,0x00,
/* * */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x28,0xFE,0x28,0x44,0x00,0x00,0x00,
/* + */
0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x08,0x08,0x08,0xFF,0x08,0x08,0x08,0x00,
/* , */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x70,0x00,0x00,0x00,0x00,0x00,0x00,
/* - */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,
/* . */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
/* / */
0x00,0x00,0x80,0x70,0x00,0x00,0x00,0x00,0xE0,0x1C,0x03,0x00,0x00,0x00,0x00,0x00,
/* 0 */
0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00,0x3F,0x40,0x80,0x80,0x40,0x3F,0x00,0x00,
/* 1 */
0x20,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,
/* 2 */
0x60,0x10,0x10,0x10,0x20,0xC0,0x00,0x00,0xC0,0xA0,0x90,0x88,0x86,0xC1,0x00,0x00,
/* 3 */
0x20,0x10,0x10,0x10,0xE0,0x00,0x00,0x00,0x80,0x80,0x82,0x83,0x44,0x38,0x00,0x00,
/* 4 */
0x00,0x00,0x80,0x60,0xF0,0x00,0x00,0x00,0x18,0x16,0x11,0x10,0xFF,0x10,0x00,0x00,
/* 5 */
0x00,0xF0,0x10,0x10,0x10,0x10,0x00,0x00,0x80,0x81,0x81,0x81,0x42,0x3C,0x00,0x00,
/* 6 */
0x80,0x40,0x20,0x10,0x10,0x10,0x00,0x00,0x3F,0x42,0x81,0x81,0x42,0x3C,0x00,0x00,
/* 7 */
0x70,0x10,0x10,0x10,0x90,0x70,0x00,0x00,0x00,0x00,0xE0,0x1C,0x03,0x00,0x00,0x00,
/* 8 */
0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00,0x38,0x45,0x82,0x82,0x45,0x38,0x00,0x00,
/* 9 */
0xC0,0x20,0x10,0x10,0x20,0xC0,0x00,0x00,0x83,0x84,0x88,0x48,0x24,0x1F,0x00,0x00,
/* : */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC3,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,
/* ; */
0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x70,0x00,0x00,0x00,0x00,0x00,0x00,
/* < */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x28,0x28,0x44,0x44,0x82,0x00,0x00,
/* = */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
/* > */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x44,0x44,0x28,0x28,0x10,0x00,0x00,
/* ? */
0xE0,0x10,0x10,0x10,0x20,0xC0,0x00,0x00,0x00,0x00,0xD8,0x06,0x01,0x00,0x00,0x00,
/* @ */
0xC0,0x20,0x10,0x90,0x90,0x10,0xA0,0xC0,0x3F,0x40,0x8F,0x90,0x90,0x8F,0x50,0x5F,
/* A */
0x00,0x00,0x80,0x70,0x80,0x00,0x00,0x00,0x80,0xF0,0x8F,0x08,0x8F,0xF0,0x80,0x00,
/* B */
0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00,0x80,0xFF,0x82,0x82,0x82,0x45,0x38,0x00,
/* C */
0xC0,0x20,0x10,0x10,0x10,0x20,0x70,0x00,0x3F,0x40,0x80,0x80,0x80,0x80,0x40,0x00,
/* D */
0x10,0xF0,0x10,0x10,0x10,0x60,0x80,0x00,0x80,0xFF,0x80,0x80,0x80,0x60,0x1F,0x00,
/* E */
0x10,0xF0,0x10,0x10,0x10,0x10,0x70,0x00,0x80,0xFF,0x82,0x82,0x82,0x87,0xE0,0x00,
/* F */
0x10,0xF0,0x10,0x10,0x10,0x10,0x70,0x00,0x80,0xFF,0x82,0x02,0x02,0x07,0x00,0x00,
/* G */
0xC0,0x20,0x10,0x10,0x20,0x70,0x00,0x00,0x3F,0x40,0x80,0x80,0x84,0x7C,0x04,0x00,
/* H */
0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00,0x80,0xFF,0x82,0x02,0x82,0xFF,0x80,0x00,
/* I */
0x10,0xF0,0x10,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,
/* J */
0x00,0x00,0x10,0xF0,0x10,0x00,0x00,0x00,0x40,0x80,0x80,0x7F,0x00,0x00,0x00,0x00,
/* K */
0x10,0xF0,0x10,0x80,0x50,0x30,0x10,0x00,0x80,0xFF,0x83,0x0C,0xB0,0xC0,0x80,0x00,
/* L */
0x10,0xF0,0x10,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x80,0x80,0x80,0xC0,0x20,0x00,
/* M */
0x10,0xF0,0x80,0x00,0x80,0xF0,0x10,0x00,0x80,0xFF,0x83,0x3C,0x83,0xFF,0x80,0x00,
/* N */
0x10,0xF0,0xC0,0x00,0x10,0xF0,0x10,0x00,0x80,0xFF,0x80,0x0F,0x30,0xFF,0x00,0x00,
/* O */
0x80,0x60,0x10,0x10,0x10,0x60,0x80,0x00,0x1F,0x60,0x80,0x80,0x80,0x60,0x1F,0x00,
/* P */
0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00,0x80,0xFF,0x84,0x04,0x04,0x02,0x01,0x00,
/* Q */
0xE0,0x18,0x04,0x04,0x04,0x18,0xE0,0x00,0x07,0x18,0x20,0x20,0x60,0x98,0x87,0x00,
/* R */
0x10,0xF0,0x10,0x10,0x10,0x20,0xC0,0x00,0x80,0xFF,0x84,0x0C,0x34,0xC2,0x81,0x00,
/* S */
0xC0,0x20,0x10,0x10,0x10,0x20,0x70,0x00,0xE0,0x41,0x82,0x82,0x84,0x48,0x30,0x00,
/* T */
0x30,0x10,0x10,0xF0,0x10,0x10,0x30,0x00,0x00,0x00,0x80,0xFF,0x80,0x00,0x00,0x00,
/* U */
0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00,0x00,0x7F,0x80,0x80,0x80,0x7F,0x00,0x00,
/* V */
0x10,0xF0,0x10,0x00,0x10,0xF0,0x10,0x00,0x00,0x01,0x1E,0xE0,0x1E,0x01,0x00,0x00,
/* W */
0x10,0xF0,0x10,0xC0,0x10,0xF0,0x10,0x00,0x00,0x0F,0xF0,0x0F,0xF0,0x0F,0x00,0x00,
/* X */
0x10,0x70,0x90,0x00,0x90,0x70,0x10,0x00,0x80,0xE0,0x99,0x06,0x99,0xE0,0x80,0x00,
/* Y */
0x10,0x70,0x90,0x00,0x90,0x70,0x10,0x00,0x00,0x00,0x83,0xFC,0x83,0x00,0x00,0x00,
/* Z */
0x70,0x10,0x10,0x10,0x10,0xD0,0x30,0x00,0xC0,0xB0,0x88,0x86,0x81,0x80,0xE0,0x00,
/* [ */
0xFC,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0x80,0x00,0x00,0x00,0x00,0x00,
/* \ */
0x70,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x1C,0xE0,0x00,0x00,0x00,0x00,
/* ] */
0x04,0x04,0xFC,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0xFF,0x00,0x00,0x00,0x00,0x00,
/* ^ */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x06,0x18,0x60,0x80,0x00,
/* _ */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
/* ` */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x60,0x40,0x80,0x00,0x00,0x00,0x00,
/* a */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x91,0x89,0x49,0xFE,0x80,0x00,0x00,
/* b */
0x10,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x81,0x81,0x42,0x3C,0x00,0x00,
/* c */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x81,0x81,0x81,0x42,0x00,0x00
,
/* d */
0x00,0x00,0x00,0x10,0xF0,0x00,0x00,0x00,0x3C,0x42,0x81,0x81,0x7F,0x80,0x00,0x00,
/* e */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x4A,0x89,0x89,0x8A,0x4C,0x00,0x00,
/* f */
0x00,0x00,0xE0,0x10,0x10,0x20,0x00,0x00,0x01,0x81,0xFF,0x81,0x01,0x00,0x00,0x00,
/* g */
0x80,0x40,0x40,0x40,0x80,0x40,0x00,0x00,0x53,0xAC,0xA4,0xA4,0xA3,0x40,0x00,0x00,
/* h */
0x10,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x01,0x01,0xFE,0x80,0x00,0x00,
/* i */
0x00,0xB0,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,
/* j */
0x00,0x00,0x80,0xCC,0x00,0x00,0x00,0x00,0x40,0x80,0x80,0x7F,0x00,0x00,0x00,0x00,
/* k */
0x10,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x98,0x25,0xC3,0x81,0x00,0x00,
/* l */
0x10,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,
/* m */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0xFE,0x01,0xFE,0x01,0xFE,0x80,0x00,
/* n */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0xFE,0x01,0x01,0xFE,0x80,0x00,0x00,
/* o */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x81,0x81,0x42,0x3C,0x00,0x00,
/* p */
0x40,0x80,0x40,0x40,0x80,0x00,0x00,0x00,0x80,0xFF,0xA0,0x20,0x10,0x0F,0x00,0x00,
/* q */
0x00,0x80,0x40,0x40,0x80,0x40,0x00,0x00,0x0F,0x10,0x20,0xA0,0xFF,0x80,0x00,0x00,
/* r */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0xFF,0x82,0x01,0x01,0x00,0x00,0x00,
/* s */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE6,0x49,0x89,0x91,0x92,0x67,0x00,0x00,
/* t */
0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x7F,0x81,0x81,0x40,0x00,0x00,
/* u */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x7F,0x80,0x81,0x7F,0x80,0x00,0x00,
/* v */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x39,0xC0,0x39,0x07,0x01,0x00,
/* w */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1F,0xE1,0x1C,0xE1,0x1F,0x01,0x00,
/* x */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x81,0xE7,0x18,0x18,0xE7,0x81,0x00,0x00,
/* y */
0x40,0xC0,0x40,0x00,0x40,0xC0,0x40,0x00,0x80,0x81,0x46,0x38,0x06,0x01,0x00,0x00,
/* z */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC3,0xA1,0x91,0x89,0x85,0xC3,0x00,0x00,
/* { */
0x00,0xF0,0x08,0x00,0x00,0x00,0x00,0x00,0x02,0x7D,0x80,0x00,0x00,0x00,0x00,0x00,
/* | */
0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* } */
0x08,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x7D,0x02,0x00,0x00,0x00,0x00,0x00,
/* ~ */
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20,0x20,0x60,0xC0,0x80,0x80,0x60
};

uint8_t inverse_factor = 0; // xor factor

static void delay_ms(long ms)
{
	SysCtlDelay(SysCtlClockGet() * ms / (1000 * 3));
}

void UC1701Init(unsigned long ulSpiClock)
{
	// Enable the GPIOx port which is connected with UC1701
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);//PA5
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);//PB3
	
	// Enable the SPIx which is connected with UC1701
  SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  // Set the chip select pin as OUT_MODE
  GPIOPinTypeGPIOOutput(LCD_PIN_SPI_CS,GPIO_PIN_5);
  // Set the chip CD pin as OUT_MODE
  GPIOPinTypeGPIOOutput(LCD_PIN_CD,GPIO_PIN_3);
	
  // Config SSI moudle pin
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);
    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_4);
  // Configure MCU as a master device , 8 bits data width ,MSB first,Mode_3
		SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,SSI_MODE_MASTER, ulSpiClock, 8);
		SSIEnable(SSI2_BASE);

  // Disable UC1701 when Power up
    GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,1<<5);

  // Initial Configuration
    // issue commands
    UC1701CmdWrite(0xE2);   // system reset
    delay_ms(200);

    UC1701CmdWrite(0xFA);   // set adv program control
    UC1701CmdWrite(0x93);
      
    UC1701CmdWrite(0xA1);   // set seg direction
    
    UC1701CmdWrite(0xC8);   // set com direction
    
    UC1701CmdWrite(0x2F);   // set power control ?!

    UC1701CmdWrite(0xA2);   // set lcd bias ratio
    
    UC1701CmdWrite(0x81);   // set electronic volume
    UC1701CmdWrite(0x1F);

    UC1701CmdWrite(0xAF);   // set display enable
}

//*****************************************************************************
//
//
//*****************************************************************************
void UC1701DataWrite(unsigned char ucData)
{
   uint32_t ulTemp;
	 ulTemp = ucData;//type change

   //Step 1 Select Chip Ƭѡ
   GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,0<<5);
    
   //Step 2 Set Display Enable (CD��ʾControl/Data,�˴�ѡ��Data)
    GPIOPinWrite(LCD_PIN_CD,GPIO_PIN_3,1<<3);
	
   //write data д����
    SSIDataPut(SSI2_BASE, ulTemp);while(SSIBusy(SSI2_BASE));
	  //SSIDataGet(SSI2_BASE, &ulTemp);//while(SSIDataGetNonBlocking(SSI2_BASE, &ulTemp));
	
   //Step 3 Disable chip select ȡ��Ƭѡ
    GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,1<<5);
 }

//*****************************************************************************
 //
//
 //*****************************************************************************
 void UC1701CmdWrite(unsigned char ucCmd)
 {
	 uint32_t ulTemp;
	 ulTemp = ucCmd;//type change
	 
   //Step 1 Select Chip Ƭѡ
   GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,0<<5);
	 
   //Step 2 Send a command (CD��ʾControl/Data,�˴�ѡ��Control)

   GPIOPinWrite(LCD_PIN_CD,GPIO_PIN_3,0<<3);
   //write cmd д����
   SSIDataPut(SSI2_BASE, ulTemp);while(SSIBusy(SSI2_BASE));
	 //SSIDataGet(SSI2_BASE, &ulTemp); // while(SSIDataGetNonBlocking(SSI2_BASE, &ucCmd));flush data
	 
   //Step 3 Disable chip select ȡ��Ƭѡ
   GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,1<<5);
 }

//*****************************************************************************
//
//
//*****************************************************************************
void UC1701DoubleCmdWrite(unsigned char ucCmd, unsigned char ucData)
{
	unsigned int ulTemp1, ulTemp2; 
	  ulTemp1 = ucCmd;  //type change
    ulTemp2 = ucData;
	
    //Step 1 Select Chip
    GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,0<<5);
     
    //Step 2 Set command
    GPIOPinWrite(LCD_PIN_CD,GPIO_PIN_3,0<<3);
	  SSIDataPut(SSI2_BASE, ulTemp1);while(SSIBusy(SSI2_BASE));
	  //SSIDataGet(SSI2_BASE, &ulTemp1);//while(SSIDataGetNonBlocking(SSI2_BASE, &ulTemp1));
	
    SSIDataPut(SSI2_BASE, ulTemp2);while(SSIBusy(SSI2_BASE));
	  //SSIDataGet(SSI2_BASE, &ulTemp1);// while(SSIDataGetNonBlocking(SSI2_BASE, &ulTemp2));
	
    //Step 3 Disable chip select
     GPIOPinWrite(LCD_PIN_SPI_CS,GPIO_PIN_5,1<<5);
 }
 
//*****************************************************************************
 //
 //
 //*****************************************************************************
 void UC1701AddressSet(unsigned char ucPA, unsigned char ucCA)
 {
   
	  ucCA += 2;
     //
    //Step 1 Set Page Address ҳ��ַ(0~7)
    //
    UC1701CmdWrite((ucPA & UC1701_SET_PA_MASK) | UC1701_SET_PA);
     
     //
     //Step 2 Set Column Address MSB �е�ַ���ֽ�(1~132)
     //

     UC1701CmdWrite(((ucCA>>4) & UC1701_SET_CA_MASK) | UC1701_SET_CA_MSB);

    //
    //Step 3 Set Column Address LSB �е�ַ���ֽ�
    //
     UC1701CmdWrite(((ucCA) & UC1701_SET_CA_MASK) | UC1701_SET_CA_LSB);
 }

 

void UC1701Dispaly(unsigned char ucLine, unsigned char ucRow,
                    unsigned char ucAsciiWord)
 { 
     int i=0,k=0;
    unsigned char uctemp;
     k=(ucAsciiWord-32)*16;

     //��8��(һ��page)��8��
    for(i=0;i<8;i++)
     {
         UC1701AddressSet((ucLine*2),(129-(ucRow*8)-i));//������ʼ��ַ
        uctemp=Ascii[k+i];
        UC1701DataWrite(uctemp);
     }  

    //��8��(���ڵ�page)��8��
   for(i=0;i<8;i++)
   {
        UC1701AddressSet((ucLine*2)+1,(129-(ucRow*8)-i));
         uctemp=Ascii[k+i+8];
         UC1701DataWrite(uctemp);
     }  
 }
 

 void UC1701InverseDispaly(unsigned char ucLine, unsigned char ucRow,
                           unsigned char ucAsciiWord)
 { 
	 int i=0,k=0;
     unsigned char uctemp;
    k=(ucAsciiWord-32)*16;
	 
    for(i=0;i<8;i++)
     {
        UC1701AddressSet((ucLine*2),(129-(ucRow*8)-i));
        uctemp=~Ascii[k+i];
         UC1701DataWrite(uctemp);
    }  
    for(i=0;i<8;i++)
    {
        UC1701AddressSet((ucLine*2)+1,(129-(ucRow*8)-i));
        uctemp=~Ascii[k+i+8];
         UC1701DataWrite(uctemp);
    } 
 }
 

void UC1701CharDispaly(unsigned char ucLine, unsigned char ucRow,
                       char *pcChar)
{
//    xASSERT((ucLine <= 3) && (ucRow <= 15));
     do
     {
        UC1701Dispaly(ucLine,ucRow,*pcChar++);
        ucRow++;
        if(ucRow>15)
        {
             ucLine++;
            ucRow=0;
            if(ucLine>3)
                break;
        }
     }
     while(*pcChar!='\0');  
 }

 void UC1701ChineseDispaly(unsigned char ucLine, unsigned char ucRow,
                          unsigned char ucLength, char *pcChar)
 { 
     int i,j=0;
//     xASSERT((ucLine <= 3) && (ucRow <= 15));
	 
     for(j=0;j<ucLength;j++)
     {
         for(i=0;i<16;i++)
         {
             UC1701AddressSet((ucLine*2),(129-(ucRow*8)-i));
             UC1701DataWrite(pcChar[j*32+i]);
         }  
         for(i=0;i<16;i++)
         {
             UC1701AddressSet((ucLine*2)+1,(129-(ucRow*8)-i));
             UC1701DataWrite(pcChar[j*32+16+i]);
         }  
         ucRow = ucRow +2;
         if (ucRow > 14)
        {
            ucLine++;
             if(ucLine > 3)
            {
                 break;
             }
        }
 }    
}

 void UC1701DisplayN(unsigned char ucLine, unsigned char ucRow,
                      unsigned long n)
 {
	 
     char pcBuf[16], *p;
	 
     if (n == 0)
     {
         UC1701Dispaly(ucLine, ucRow, '0');
     }
     else
     {
         p = pcBuf;
         while (n != 0)
        {
            *p++ = (n % 10) + '0';
             n /= 10;
         }
 
         while (p > pcBuf)
         {
             UC1701Dispaly(ucLine, ucRow, *--p);
            ucRow++;
            if (ucRow > 14)
            {
                ucLine++;
                if(ucLine > 3)
                 {
                     break;
                 }
             }
         }
     }
}

 //*****************************************************************************
void UC1701Clear(void)
{
    unsigned char i,j;
    
    //unsigned char ComTable[]={3,2,1,0,7,6,5,4,};

    for(i=0;i<8;i++)
    {
        UC1701CmdWrite(0xb0+i);
        //UC1701CmdWrite(0xb0|ComTable[i]);
        UC1701CmdWrite(0x10);
        UC1701CmdWrite(0x00);
        for(j=0;j<132;j++)
        {
            UC1701DataWrite(0x00);
        }
    }
}

void UC1701InverseEnable(void)
{
#if 0
    uint32_t x,y;
    for(y=0;y<8;y++)
    {    
        UC1701CmdWrite(0xb0+y);
        UC1701CmdWrite(0x10);
        UC1701CmdWrite(0x00);
        for(x=0;x<128;x++) 
        if(x%2==0) UC1701CmdWrite(0); 
        else UC1701CmdWrite(0xFF);
    }

    UC1701CmdWrite(UC1701_SET_DC0_EN);
#else
    inverse_factor = 0xFF;
#endif
}

//*****************************************************************************
//
 //
//*****************************************************************************
void UC1701InverseDisable(void)
{
#if 0
    UC1701CmdWrite(UC1701_SET_DC0);
#else
    inverse_factor = 0x0;
#endif
}

//*****************************************************************************
//
 //
 //*****************************************************************************
void UC1701AllPixelOnEnable(void)
{
    UC1701CmdWrite(UC1701_SET_DC1_EN);
 }

 void UC1701AllPixelOnDisable(void)
 {
     UC1701CmdWrite(UC1701_SET_DC1);
}


 //*****************************************************************************
void UC1701DisplayOn(void)
{
     UC1701CmdWrite(UC1701_SET_DC2_EN);
}

void UC1701DisplayOff(void)
{
       UC1701CmdWrite(UC1701_SET_DC2);
}

//*****************************************************************************
void UC1701ScrollLineSet(unsigned char ucLine)
{
//        xASSERT((ucLine <= 63) && (ucLine >= 0));
        UC1701CmdWrite(UC1701_SET_SL | (ucLine & UC1701_SET_SL_MASK));
}
 
//*****************************************************************************
void UC1701PMSet(unsigned char ucPM)
{
        UC1701DoubleCmdWrite(UC1701_SET_PM, ucPM);
}

//*****************************************************************************
void UC1701BRSet(unsigned char ucBR)
{
        UC1701CmdWrite(UC1701_SET_BR_MASK & ucBR);
}

//*****************************************************************************
void UC1701SegDirSet(unsigned char ucSegDir)
{
        UC1701CmdWrite(UC1701_SEG_DIR_MASK & ucSegDir);
}

//*****************************************************************************
void UC1701ComDirSet(unsigned char ucComDir)
{
        UC1701CmdWrite(UC1701_COM_DIR_MASK & ucComDir);
}