/*
 * RTC.c
 *
 *  Created on: May 6, 2021
 *  Function for setting and reading time on/from RTC and reading temperature from Temp sensors
 *  Connection
 *  STM NUCLEO F429ZI-> RTC DS1307
 *  PB8 (D15 na CN7 (pin2)) I2C1 SCL-> SCL RTC
 *  PB9 (D14 na CN7 (pin4)) I2C1 SDA-> SDA RTC
 *  5V->VCC
 *  GND->GND
 *      Author: Aleksandar
 */
#include "RTC.h"
I2C_HandleTypeDef hi2c1;										//I2C1 handler
struct MyTimeStruct emptyTimeStruct={0};

/**
 * @brief  Function conversion from binary to decimal system.
 * @param  argument: Binary value to be converted
 * @retval Decimal value after conversion
 */
int bcdToDec(uint8_t val)
{
   return (int)( (val/16*10) + (val%16) );
}

/**
 * @brief  Function conversion from decimal to binary system.
 * @param  argument: Decimal value to be converted
 * @retval Binary value after conversion
 */
uint8_t decToBcd(int val)
{
  return (uint8_t)( (val/10*16) + (val%10) );
}
/**
 * @brief  Function reads time from RTC.
 * @param  argument: none
 * @retval Strutc Vreme type variable
 */
 struct MyTimeStruct getTime()
 {

	 uint8_t buffer[7]={0};															//Array to recive data from RTC
	 struct MyTimeStruct currTime={0};												//Struct to retunr
	 currTime=emptyTimeStruct;														//Initialize struct to empty


	 HAL_I2C_Mem_Read(&hi2c1, ((0x68<<1) | 0x01), 0x00, 1, buffer, 7, 50); 			//Read from memory of RTC
	 currTime.sec=bcdToDec(buffer[0]);												//Writing data to struct, after binary to decimal conversion
	 currTime.min=bcdToDec(buffer[1]);
	 currTime.sat=bcdToDec(buffer[2]);
	 currTime.dow=bcdToDec(buffer[3]);
	 currTime.dom=bcdToDec(buffer[4]);
	 currTime.mon=bcdToDec(buffer[5]);
	 currTime.year=bcdToDec(buffer[6]);

	 return currTime;																//returnig struct type variable
 }
 /**
  * @brief  Function set time on RTC.
  * @param  argument: Decimal value of secund, minut, hour, day of week, day of mounth and year to write it down in memeory
  * @retval none
  */
 void setTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year)
{

		uint8_t CurTimeBuff[7];																	//Array that need to be send to RTC and conntain all values from argument in binary format
		CurTimeBuff[0] = decToBcd(sec);															//Conversion from decimal to binary format
		CurTimeBuff[1] = decToBcd(min);
		CurTimeBuff[2] = decToBcd(hour);
		CurTimeBuff[3] = decToBcd(dow);
		CurTimeBuff[4] = decToBcd(dom);
		CurTimeBuff[5] = decToBcd(month);
		CurTimeBuff[6] = decToBcd(year);

	 HAL_I2C_Mem_Write(&hi2c1, (0x68<<1), 0x00, 1, CurTimeBuff, sizeof(CurTimeBuff), 100);		//Writing information to RTC memory

}
 /**
  * @brief  Function readint temperature from Temp Sensor.
  * @param  argument: non
  * @retval Float type temperature value
  */
float getTemp()
{
	int16_t value;
	HAL_StatusTypeDef ret;
	uint8_t buffer[12];
	float temperament;

	ret=HAL_I2C_Master_Transmit(&hi2c1, (0x48<<1), 0x00, 1, HAL_MAX_DELAY);				//Sending Command to RTC to start sending data from memory
	if(ret!=HAL_OK)																		//Check if communication was success
	{
		temperament=-254;																//If not set samo random value
	}

	ret=HAL_I2C_Master_Receive(&hi2c1, (0x48<<1), buffer, 2, HAL_MAX_DELAY);			//Receving data from RTC
	if(ret!=HAL_OK)																		//Check if communication was success
	{
		temperament=-200;
	}else
	{
		value=((int16_t)buffer[0]<<4) | (buffer[1]>>4);									//making from 2 4bit data one 8-bit bianry number to convert to decimal

		if(value>0x7FF)
		{
			value|=0xF000;
		}

		temperament=value*0.0625;														//Calculating to tepm
	}
	return temperament;																	//return the calculated value
}
