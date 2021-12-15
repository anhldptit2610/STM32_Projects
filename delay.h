/**
	*******************************************************
	* @file delay.h
	* @brief delay driver for STM32F103C8T6
	*******************************************************
  **/


#ifndef delay
#define delay

// Include some necessary header files
#include "stm32f10x.h"

/**
	*******************************************************
	* @brief Timer4 Initialization
	*	@param [in] 
	**/
void Timer4_Init (void);


/**
	*******************************************************
	* @brief Microseconds delay function
	*	@param [in] 
	**/
void Delay_us (uint32_t us);

/**
	*******************************************************
	* @brief Miliseconds delay function
	*	@param [in] 
	**/
void Delay_ms (uint32_t ms);









#endif /*EndOfFile*/
