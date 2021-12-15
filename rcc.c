/**
	*******************************************************
	* @file rcc.c
	* @brief reset and clock control driver for STM32F103C8T6
	*******************************************************
	**/


#include "rcc.h"

/**
	*******************************************************
	* @brief Clock Setup for STM32F103C8T6
	* 
	*	NOTE: In this project I want to set the clock frequency upto
	*       72MHz(maximum).
	**/
void RCC_Config (void)
{
		// Turn on HSE crystal and wait until HSE crystal become stable
		RCC->CR |= RCC_CR_HSEON;
		while(!(RCC->CR & RCC_CR_HSERDY));
	
		// Enable the prefetch buffer
		FLASH->ACR |= FLASH_ACR_PRFTBE;
	
		// Set the flash latency
		FLASH->ACR |= FLASH_ACR_LATENCY_1;
		
		// Configure PLL
		RCC->CFGR &= ~(RCC_CFGR_PLLXTPRE); // no pre - HSE divider for PLL entry
	
		RCC->CFGR |= RCC_CFGR_PLLSRC; //PLL source
	
		RCC->CFGR |= RCC_CFGR_PLLMULL9; // PLL input clock*9
	
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1; // AHB Prescaler = 1
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 Prescaler = 2
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 Prescaler = 1
		
		// Turn on PLL and wait until it's ready
		RCC->CR |= RCC_CR_PLLON;
		while(!(RCC->CR & RCC_CR_PLLRDY));
		
		// Select PLL as system clock
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		while( (RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
		
		SystemCoreClockUpdate();

		
	
}


/*EndOfFile*/
