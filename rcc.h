/**
	*******************************************************
	* @file rcc.h
	* @brief reset and clock control driver for STM32F103C8T6
	*******************************************************
	**/

	

#ifndef rcc
#define rcc

// Include some necessary header file
#include "stm32f10x.h"



/**
	*******************************************************
	* @brief Clock Setup for STM32F103C8T6
	* 
	*	NOTE: In this project I want to set the clock frequency upto
	*       72MHz(maximum).
	**/
void RCC_Config (void);








#endif /*EndOfFile*/
