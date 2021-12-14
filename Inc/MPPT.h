/*
 * MPPT.h
 *
 *  Created on: May 7, 2021
 *  MpptMessage structure for saving data from mppt, funciton to recive that data and to
 *  set mode for MPPT. Aveable mods in MPPT.c
 *      Author: Aleksandar
 */
#ifndef INC_MPPT_H_
#define INC_MPPT_H_
#include "main.h"


struct MpptMessage
	{
	  char Pid[10];				//Product ID
	  char FW[10];				//Firmware version
	  char SER[15];				//Serial number
	  char V[10];				//Battery voltage mV
	  char I[10];				//Battery current mA
	  char Vpv[10];				//Panel Voltage
	  char Ppv[10];				//Panel Power
	  char Cs[10];				//State of MPPT
	  char Mppt[10];			//Tracker operation mode
	  char ERR[10];				//Error code
	  char Load[10];			//State of Load exit (ON/OFF)
	  char Il[10];				//Load current
	  char H19[10];				//Yield total
	  char H20[10];				//Yield today
	  char H21[10];				//Max power today
	  char H22[10];				//Yield yestrday
	  char H23[10];				//Max power yestrday
	  char Hsds[10];			//Day sequence number
	  char Checksum[10];
	}MpptMessage;


struct MpptMessage getMppt();									//Writing message from mppt tp struct
void setMpptMode(uint8_t mod);       							//Setting mode for MPPT, in mppt.c can finde all aveable mods
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
#endif /* INC_MPPT_H_ */
