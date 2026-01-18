/*
 * bsp.h
 *
 *  Created on: 28 févr. 2023
 *      Author: Acer
 */

#ifndef INC_BSP_H_
#define INC_BSP_H_

#include "stm32f0xx.h"

/*
 * LED driver functions
 */

void	BSP_LED_Init	(void);
void	BSP_LED_On	(void);
void	BSP_LED_Off	(void);
void	BSP_LED_Toggle	(void);

/*
 * Push-Button driver functions
 */

void       BSP_PB_Init		(void);
uint8_t    BSP_PB_GetState	(void);

/*
 * Debug Console init
 */

void	BSP_Console_Init	(void);

/*
 * ADC functions
 */

void BSP_ADC_Init		(void);

/*
 * Timer functions
 */

void BSP_TIMER_Timebase_Init (void);
void BSP_TIMER_IC_Init		(void);

void BSP_TIMER_PWM_Init();

void BSP_DAC_Init();


/*
 * Initialize I2C1 peripheral @100kHz
 */
void BSP_I2C1_Init();

void BSP_DBG_Pins_Init();

uint8_t	BSP_I2C1_Read( uint8_t device_address,
		uint8_t register_address,
		uint8_t *buffer,
		uint8_t nbytes );

uint8_t	BSP_I2C1_Write( uint8_t device_address,
		uint8_t register_address,
		uint8_t *buffer, uint8_t nbytes );

/*
 * BSP_SPI1_Init()
 * Initialize SPI1 peripheral @375kHz
 */
void BSP_SPI1_Init();

uint8_t BSP_SPI_SendReceive(uint8_t tx_byte);

void BSP_LPS25H_Read(uint8_t register_address, uint8_t *buffer, uint8_t nbytes);

void BSP_LPS25H_Write(uint8_t register_address, uint8_t data);

void BSP_LPS25H_ReadFast(uint8_t register_address, uint8_t *buffer, uint8_t nbytes);

void BSP_NVIC_Init(void);

void BSP_DBG_Pin_Init();


#endif /* INC_BSP_H_ */
