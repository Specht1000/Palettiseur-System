/*
 * delay.h
 *
 *  Created on: 3 mars 2023
 *      Author: Acer
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "stm32f0xx.h"

/*
 * Software counting delays
 */

void BSP_DELAY_ms	(uint32_t delay);

void BSP_DELAY_us	(uint32_t delay);

/*
 * Timer delays
 */

void BSP_DELAY_TIM_init		(void);
void BSP_DELAY_TIM_ms		(uint16_t ms);

#endif /* INC_DELAY_H_ */
