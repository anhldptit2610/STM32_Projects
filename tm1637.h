#ifndef TM1637
#define TM1637

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"
#include <stdbool.h>

#define TM1637_DIO_Pin  GPIO_Pin_7
#define TM1637_CLK_Pin  GPIO_Pin_6

#define C0H     0xC0
#define C1H     0xC1
#define C2H     0xC2
#define C3H     0xC3
#define C4H     0xC4
#define C5H     0xC5

#define TM1637_DATA_SET                     0x40
#define TM1637_DATA_FIXED                   0x44
#define TM1637_DISPLAY_ON                   0x88
#define TM1637_DISPLAY_OFF                  0x80
#define TM1637_PACKET_SIZE                  0x08
#define TM1637_NUMBER_OF_SEGMENTS           0x04

enum GPIO_Direction
{
    IN,
    OUT,
};

void TM1637_Init(void);

void TM1637_Start(void);

void TM1637_End(void);

void TM1637_SendCommand(uint8_t command);

void TM1637_CLK_High(void);

void TM1637_CLK_Low(void);

void TM1637_DIO_High(void);

void TM1637_DIO_Low(void);

void TM1637_DisplayBaseOnPos(uint8_t brightness, uint8_t value, uint8_t position);

void TM1637_Display(uint8_t brightness, uint16_t value);

#endif