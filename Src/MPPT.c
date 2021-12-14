/*
 * MPPT.c
 *
 *  Created on: May 7, 2021
 *  Function for comunication with MPPT via UART 6.
 *  Connection
 *  STM32 F429ZI- MPPT 75/15 BlueSolar
 *  PA3 (A0 na CN9 (pin 1)) UART2_RX-> MPPT_TX (pin3)
 *  PD5 (D53 na CN9 (pin 6)) UART2_TX-> MPPT_RX (pin2)
 *  5V-> Power+ (pin 4)
 *  GND->GND (pin 1)
 *  Mods:
 *  uint8_t mod=1;					//Always off
 *	uint8_t mod=2;					//Battery life
 *	uint8_t mod=3;					//Conv. algoritam 1 OFF < 11.1V ON > 13.1V
 *	uint8_t mod=4;					//Conv. algoritam 2 OFF < 11.8V ON > 14V
 *	uint8_t mod=5;					//Always on
 *	uint8_t mod=6;					//User def. algoritam 1 OFF < 10V ON > 14V (mozemo podesavati zeljene vrednosti preko alpikacije)
 *	uint8_t mod=7;					//User def. algoritam 2 OFF < 10V ON > 14V
 *
 *      Author: Aleksandar
 */

#include "MPPT.h"
#include <string.h>
#include "main.h"				//Ukljucijemo radi prepoznavanja HAL funkcija
#include "cmsis_os.h"
UART_HandleTypeDef huart6;		//UART for communication of MPPT-a and STM-a
UART_HandleTypeDef huart3;      //To send data to virtual com Port


/**
 * @brief  Setting up the selected mode for MPPT, avaliable mods are listed above, via uart6 (19200 baud).
 * @param  argument: Wanted mode, int value from to
 * @retval None
 */
void setMpptMode(uint8_t mod)
{
	const char off[] = ":8ABED0000B5\n";
	const char on[] = ":8ABED0004B1\n";
	const char baterryLife[] = ":8ABED0001B4\n";
	const char ConvAl1[] = ":8ABED0002B3\n";
	const char ConvAl2[] = ":8ABED0003B2\n";
	const char UserDefAl1[] = ":8ABED0005B0\n";
	const char UserDefAl2[] = ":8ABED0006AF\n";

	switch(mod)
	{
	case 1: HAL_UART_Transmit(&huart6, &off, sizeof(off), 1000);							//Sending appropriate command via uart to MPPT
			break;
	case 2: HAL_UART_Transmit(&huart6, &baterryLife, sizeof(baterryLife), 1000);
			break;
	case 3: HAL_UART_Transmit(&huart6, &ConvAl1, sizeof(ConvAl1), 1000);
			break;
	case 4: HAL_UART_Transmit(&huart6, &ConvAl2, sizeof(ConvAl2), 1000);
			break;
	case 5: HAL_UART_Transmit(&huart6, &on, sizeof(on), 1000);
			break;
	case 6: HAL_UART_Transmit(&huart6, &UserDefAl1, sizeof(UserDefAl1), 1000);
			break;
	case 7: HAL_UART_Transmit(&huart6, &UserDefAl2, sizeof(UserDefAl2), 1000);
			break;
	}

}
/**
 * @brief  Receving the message from Mppt via uasrt6, then separate it in appropriate format, each inforamtion for themself.
 * @param  argument: Not used
 * @retval Struct Poruka type por variable
 */
struct MpptMessage getMppt()
{
		char buffer[170]={0};															//Buffer for receving message from mppt
		volatile uint8_t u8CheckPor=0;													//If message is correct it will be 1
		uint8_t i=0, k=0, j=0, m=0;														//i,j for moving thrught buffer, k word couter
		uint8_t u8UpisaoPoruku=0;
		struct MpptMessage por={.Pid=""};													//Struct Poruka that conntains inforamtion from MPPT
		while(u8CheckPor==0)
		{
			m=0;
			HAL_UART_Receive(&huart6, buffer, 170,1000);								//Receving the message
			do
			{
				if(buffer[m]=='P' && buffer[m+1]=='I' && buffer[m+2]=='D' && m<5)	//Checking if the message is in right format
				{
					u8CheckPor=1;
				}
				m++;
			}while(m<=5 && u8CheckPor==0);												//if after 5 character it is not correct, recive message again.
		}
		u8CheckPor=0;

		for(i=1;i<sizeof(buffer);i++)													//Dividing whole message from MPPT in separate words
		{
			j=0;
			if(u8UpisaoPoruku==0)
			{
				if(buffer[i]=='\t')													//When character '\t' appired new word is started
				{
					k++;																//Moving on to next word
					i++;																// i and j counter to move trhugh the buffer

					switch(k)															//k counter for words
					{
					case 1: do
						{
							por.Pid[j]=buffer[i];										//Copying ona character form buffer to appropriate struct word
							i++;														//Moving on to next charachter
							j++;
						}while(buffer[i]!='\r');										//Looping that until '\r' character appired
						i++; 															//Skipping charachter '\n'
						break;															//Doing this for every word

					case 2: do
						{
							por.FW[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 3: do
						{
							por.SER[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 4: do
						{
							por.V[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 5: do
						{
							por.I[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 6: do
						{
							por.Vpv[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 7: do
						{
							por.Ppv[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 8: do
						{
							por.Cs[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 9: do
						{
							por.Mppt[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 10: do
						{
							por.ERR[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 11: do
						{
							por.Load[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 12: do
						{
							por.Il[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 13: do
						{
							por.H19[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 14: do
						{
							por.H20[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 15: do
						{
							por.H21[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 16: do
						{
							por.H22[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 17: do
						{
							por.H23[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 18: do
						{
							por.Hsds[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							i++;
							break;

					case 19: do
						{
							por.Checksum[j]=buffer[i];
							i++;
							j++;
						}while(buffer[i]!='\r');
							k=0;
							i++;
							u8UpisaoPoruku=1;
							break;
					}
				}
			}
		}

		return por;																				//Returning Poruka structur

}


