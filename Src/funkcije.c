/*
 * funkcije.c
 *
 *  Created on: Sep 17, 2021
 *      Author: Stefan
 */
#include "main.h"
#include "funkcije.h"

/**
 * @brief  Turning LedRings OFF.
 * @param  argument: Not used
 * @retval None
 */
void LedRingOff()
{
	 HAL_GPIO_WritePin(LEDRING0_CTRL_GPIO_Port, LEDRING0_CTRL_Pin, RESET);
	 HAL_GPIO_WritePin(LEDRING1_CTRL_GPIO_Port, LEDRING1_CTRL_Pin, RESET);
}
/**
 * @brief  Turning Sensors OFF.
 * @param  argument: Not used
 * @retval None
 */
void SensorsOff()
{
	  HAL_GPIO_WritePin(SENSORS_CTRL_GPIO_Port, SENSORS_CTRL_Pin, RESET);
	  HAL_GPIO_WritePin(AUX_5V_CTRL_GPIO_Port, AUX_5V_CTRL_Pin, RESET);
}
/**
 * @brief  Turning Chargers OFF.
 * @param  argument: Not used
 * @retval None
 */
void ChargersOff()
{
	  HAL_GPIO_WritePin(WICH0_CTRL_GPIO_Port, WICH0_CTRL_Pin, RESET);
	  HAL_GPIO_WritePin(WICH1_CTRL_GPIO_Port, WICH1_CTRL_Pin, RESET);
	  HAL_GPIO_WritePin(USB0_CTRL_GPIO_Port, USB0_CTRL_Pin, RESET);
	  HAL_GPIO_WritePin(USB1_CTRL_GPIO_Port, USB1_CTRL_Pin, RESET);
}
/**
 * @brief  Turning LedRings ON.
 * @param  argument: Not used
 * @retval None
 */
void LedRingOn()
{
	 HAL_GPIO_WritePin(LEDRING0_CTRL_GPIO_Port, LEDRING0_CTRL_Pin, SET);
	 HAL_GPIO_WritePin(LEDRING1_CTRL_GPIO_Port, LEDRING1_CTRL_Pin, SET);
}

/**
 * @brief  Turning Sensors ON.
 * @param  argument: Not used
 * @retval None
 */
void SensorsOn()
{
	  HAL_GPIO_WritePin(SENSORS_CTRL_GPIO_Port, SENSORS_CTRL_Pin, SET);
	  HAL_GPIO_WritePin(AUX_5V_CTRL_GPIO_Port, AUX_5V_CTRL_Pin, SET);
}
/**
 * @brief  Turning Chargers ON.
 * @param  argument: Not used
 * @retval None
 */
void ChargersOn()
{
	  HAL_GPIO_WritePin(WICH0_CTRL_GPIO_Port, WICH0_CTRL_Pin, SET);
	  HAL_GPIO_WritePin(WICH1_CTRL_GPIO_Port, WICH1_CTRL_Pin, SET);
	  HAL_GPIO_WritePin(USB0_CTRL_GPIO_Port, USB0_CTRL_Pin, SET);
	  HAL_GPIO_WritePin(USB1_CTRL_GPIO_Port, USB1_CTRL_Pin, SET);
}
/**
 * @brief  Turning every out ON.
 * @param  argument: Not used
 * @retval None
 */
void ButOn()
{
	HAL_GPIO_WritePin(USB0_CTRL_GPIO_Port, USB0_CTRL_Pin, SET);
	HAL_GPIO_WritePin(USB1_CTRL_GPIO_Port, USB1_CTRL_Pin, SET);
	HAL_GPIO_WritePin(WICH0_CTRL_GPIO_Port,WICH0_CTRL_Pin, SET);
	HAL_GPIO_WritePin(WICH1_CTRL_GPIO_Port, WICH1_CTRL_Pin, SET);
	HAL_GPIO_WritePin(AUX_5V_CTRL_GPIO_Port, AUX_5V_CTRL_Pin, SET);
	HAL_GPIO_WritePin(AUX_12V_CTRL_GPIO_Port, AUX_12V_CTRL_Pin, SET);
	HAL_GPIO_WritePin(SENSORS_CTRL_GPIO_Port, SENSORS_CTRL_Pin, SET);
}
/**
 * @brief  Turning Main switch On, and configuration for button interupt.
 * @param  argument: Not used
 * @retval None
 */
void MainSW()
{
	 HAL_GPIO_WritePin(IN_12V_CTRL_GPIO_Port, IN_12V_CTRL_Pin, SET);
	 HAL_GPIO_WritePin(ROUTER_CTRL_GPIO_Port, ROUTER_CTRL_Pin, SET);
	 HAL_GPIO_WritePin(GPIO_0_GPIO_Port, GPIO_0_Pin, RESET);
	 HAL_GPIO_WritePin(GPIO_2_GPIO_Port, GPIO_2_Pin, SET);
	 HAL_GPIO_WritePin(Button_IN_GPIO_Port, Button_IN_Pin, RESET);

}
/**
 * @brief  Turning every out OFF.
 * @param  argument: Not used
 * @retval None
 */
void OutsOff()
{
	HAL_GPIO_WritePin(AUX_12V_CTRL_GPIO_Port, AUX_12V_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(USB0_CTRL_GPIO_Port, USB0_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(USB1_CTRL_GPIO_Port, USB1_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(WICH0_CTRL_GPIO_Port,WICH0_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(WICH1_CTRL_GPIO_Port, WICH1_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(LEDRING0_CTRL_GPIO_Port, LEDRING0_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(LEDRING1_CTRL_GPIO_Port, LEDRING1_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(SENSORS_CTRL_GPIO_Port, SENSORS_CTRL_Pin, RESET);
	HAL_GPIO_WritePin(AUX_5V_CTRL_GPIO_Port, AUX_5V_CTRL_Pin, RESET);
}
