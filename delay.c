/**
	*******************************************************
	* @file delay.c
	* @brief delay driver for STM32F103C8T6
	*******************************************************
  **/

#include "delay.h"


/**
	*******************************************************
	* @brief Timer4 Initialization
	*	@param [in] 
	**/
void Timer4_Init (void)
{
		// Enable the clock for Timer4
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
		
		// Configure the prescaler
		TIM4->PSC = 71;
		TIM4->ARR = 0xFFFF;
		
		TIM4->CR1 |= TIM_CR1_CEN;
		while(!(TIM4->SR & TIM_SR_UIF));
}

/**
	*******************************************************
	* @brief Microseconds delay function
	*	@param [in] 
	**/
void Delay_us (uint32_t us)
{
		TIM4->CNT = 0;
		while(TIM4->CNT < us);
}

/**
	*******************************************************
	* @brief Miliseconds delay function
	*	@param [in] 
	**/
void Delay_ms (uint32_t ms)
{
		for (uint32_t i = 0; i < ms; i++)
		{
				Delay_us(1000);
		}
}









/*EndOfFile*/
