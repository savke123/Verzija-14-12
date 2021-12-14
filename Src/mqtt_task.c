/*
 * mqtt_task.c
 *
 *  Created on: 2020. 5. 19.
 *      Author: eziya76@gmail.com
 */

#include "main.h"
#include "mqtt_task.h"
#include "RTC.h"
#include "string.h"
#include "MPPT.h"
#include "funkcije.h"
#include "adc.h"
#include "iwdg.h"

#define SERVER_ADDR		"a36r1007e087sh-ats.iot.eu-west-1.amazonaws.com" 		//Endpoint from AWS Thing
#define MQTT_PORT		"8883"
struct Switch sw1;																//Switch structur for U7, can contain current, voltage and temperature of swich U7, main swich, we use current
struct Switch sw2;																//Switch structur for U8, can contain current, voltage and temperature of swich U8, LedRing1 and LedRing2
struct Switch sw3;																//Switch structur for U9, can contain current, voltage and temperature of swich U9, router and 12V exit
struct Switch sw4;																//Switch structur for U10, can contain current, voltage and temperature of swich U10, USB1 and USB2
struct Switch sw5;																//Switch structur for U11, can contain current, voltage and temperature of swich U11, WCh1 and Wch2
struct Switch sw6;																//Switch structur for U12, can contain current, voltage and temperature of swich U12, Sensors and 5V
TaskHandle_t Task3BtnHandle=NULL;												//Global task handler, so it can be resumd from ISR
uint8_t u8NOffUsb1;																//Counter of OFF state of chargers for USB1
uint8_t u8NOffUsb2;																//Counter of OFF state of chargers for USB2
uint8_t u8NOffWch1;																//Counter of OFF state of chargers for WCh1
uint8_t u8NOffWch2;																//Counter of OFF state of chargers for WCh2
uint8_t u8NumOfUser=0;															//Number of charge, number of users
uint32_t u32PreviousMillis = 0;													//For button Debouncing
uint32_t u32CurrentMillis = 0;
/**
 * @brief 	External Button interupt, this function resume button task (Task3BTN) from yeald.
 * 			Ext Int is conected to pin PG14 of Nucleo board
 * @param   argument: Not used
 * @retval  None
 */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */
  /* USER CODE END EXTI15_10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  BaseType_t checkIfYieldIsrequired;
  checkIfYieldIsrequired=xTaskResumeFromISR(Task3BtnHandle);
  portYIELD_FROM_ISR(checkIfYieldIsrequired);

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
 * @brief   MqttClientSubTask is trying to connect to AWS via MQTT,
 * 			if it is successful it return MqttConnectBroker O.K. message.
 * 			IF not it will try again in 1 secunkd
 * @param   argument: Not used
 * @retval  None
 */
void MqttClientSubTask(void const *argument)
{
	while(1)
	{
		if(!mqttClient.isconnected)
		{
			//try to connect to the broker
			MQTTDisconnect(&mqttClient);
			MqttConnectBroker();
			osDelay(1000);
		}
		else
		{
			/* !!! Need to be fixed
			 * mbedtls_ssl_conf_read_timeout has problem with accurate timeout
			 */
			MQTTYield(&mqttClient, 1000);
		}
	}
}

//mqtt publish task
/**
 * @brief   MqttClientPubTask main task is to send the report to AWS server,
 * 			In function MQTTPublish parametars, secund parametar is the name of TOPIC you want to subscribe to.
 * 			Here we also get the board temperature (tempint), check time for the LED Rings (schuld be on beatween 18-01 h),
 * 			We check battery voltige, if it is to low then schutdown some fraturs.
 * 			Calculating buttery percentig.
 * 			Main report schuld be sent on every hour, on evry 20 secunds it sends just ping to server, becouse TCP/PI socket stays open.
 * @param   argument: NULL
 *
 * @retval  None
 * TODO: Open socket only when you need to send report, then close it.
 */
void MqttClientPubTask(void const *argument)
{
	MQTTMessage message;						//Contains MQTTMes that need to be sent
	float ftemp;								//Contains value of temperature that is read from sensors
	uint8_t u8tempint;							//Temperature in int format, Mqtt message format need int value
	volatile uint16_t u16Vbat;					//Contains battery voltage, that is read from MPPT
	volatile struct MyTimeStruct time;			//Time structure, contains secund,minuts, hours, day of week, day of month, month and year information from RTC
	volatile struct MpptMessage PorMppt;		//Mppt structur, contains all information from MPPT (buttery voltage, panel voltage, main current, panel power, etc.)
	uint8_t FlegLDRTime=0;						//Check if it is time for LED Ring to turn on (1 - turn on, 0 - turn off) in case of battery voltage being to low
	volatile float fprc;						//Calculating percent of the buttery
	uint8_t u8Prcint;							// Converting float to int
	uint16_t u16vBatMax=13500;					//Max buttery voltage in mV
	static char *str=NULL;						//String in which we write uor message

	while(1)
	{
		ftemp=getTemp();						//getTemp() reads teperature from Temp sensor via I2C, return float type and put it in temp variable
		u8tempint=ftemp/1;						//Converting float type to int
		HAL_IWDG_Refresh(&hiwdg);				//Refreshing Watchdog timer (30 secunds timer period)


		time=getTime();							//getTime() return time Struct from RTC, aslo reading it via I2C, put it in variable time, struct type
		if((time.sat>=18 && time.sat<=23) || (time.sat>=0 && time.sat<1))		//chech is it time to turn LED Rings
		{
			LedRingOn();						//Turn ON LED Rings, if yes
			FlegLDRTime=1;
		}else
		{
			LedRingOff();						//If not, turn OFF LED Rings
			FlegLDRTime=0;
		}

		PorMppt=getMppt();						//getMppt() reciving message from MPPT via UART and separete it in parts
		u16Vbat=atoi(PorMppt.V);					//atoi convert string that contains buttert voltige in int type, put it in Vbat variable

		if(u16Vbat<12000)							//Check is if the buttery voltage is OK, if not turn of some feature
		{
			//TODO: Uncomment sensors on and off function, This is for Trg testing
			if(u16Vbat<11300)
			{
				if(u16Vbat<10800)
				{
					ChargersOff();				//Turn OFF chargers if Bat Volt is lower than 10.8 V
				}
				//SensorsOff();					//Turn OFF sensors if Bat Volt is lower than 11.3 V
			}else
			{
				//SensorsOn();					//Turning on sensors
			}
			LedRingOff();						//Turn OFF LED Rings if Bat Volt is lower than 12 V
		}else
		{
			if(FlegLDRTime==1)					//If battery level is above 12 V and it is time for Led Ring turn it on
			{
				LedRingOn();					//Turning on Led Rings
			}
			//SensorsOn();
		}
		HAL_IWDG_Refresh(&hiwdg);				//Refreshing Watchdog timer (30 secunds timer period)
		fprc=((float)(u16Vbat)/u16vBatMax)*100;		//Calculating buttery percent
		if(fprc>=100)							//If prc is above 100, fix it to 100
		{
			fprc=100;
		}
		u8Prcint=fprc/1;							//Convert float to Int type
		if(mqttClient.isconnected)
		{

			if(time.min==0)
			{
				//char *str=pvPortMalloc(370*sizeof(char));			//160 kanc test
				str=(char*)pvPortMalloc(512*sizeof(char));			//Reserving memory for message
				sprintf((char*)str,"{\n\"TempCelsius\" : %d, \n "
						"\"MainSwCur\" : %d, \n "
						"\"RouterCur\" : %d, \n \"12VExtCur\" : %d, \n "
						"\"Usb1Cur\" : %d, \n \"Usb2Cur\" : %d, \n "
						"\"Wch1Cur\" : %d, \n \"Wch2Cur\" : %d, \n "
						"\"Ldr1Cur\" : %d, \n \"Ldr2Cur\" : %d, \n "
						"\"SensCur\" : %d, \n \"5VExtCur\" : %d, \n "
						"\"PanVolmV\" : %s, \n "
						"\"BatVolmV\" : %s, \n "
						"\"BatPrc\" : %d, \n "
						"\"LoadCurmA\" : %s, \n "
						"\"PanPowW\" : %s, \n "
						"\"MaxPowTodayW\" : %s, \n "
						"\"YieldTodaykWh\" : %s, \n "
						"\"MainBatCurmA\" : %s, \n "
						"\"NumOfUser\" : %d, \n "
						"\"Vreme\" : \"%d : %d : %d\" \n}", u8tempint,sw1.I1,sw2.I1,sw2.I2,sw3.I1,sw3.I2,sw4.I1,sw4.I2,sw5.I1,sw5.I2,sw6.I1,sw6.I2,PorMppt.Vpv,PorMppt.V,u8Prcint,PorMppt.Il,PorMppt.Ppv,PorMppt.H21,PorMppt.H19,PorMppt.I,u8NumOfUser,time.sat,time.min,time.sec);

				u8NumOfUser=0;
				message.payload = (void*)str;
				message.payloadlen = strlen(str);
				MQTTPublish(&mqttClient, "test", &message); 		//publish a message to "test" topic, secund argument topic name
				vPortFree((void*)str);								// Releas that memory
			}else
			{
				str=(char*)pvPortMalloc(40*sizeof(char));			//Reserving memory for message
				sprintf((char*)str,"{\"Vreme\" : \"%d : %d : %d\",\n \"BrKor\" : %d \n}",time.sat,time.min,time.sec,u8NumOfUser);

				message.payload = (void*)str;
				message.payloadlen = strlen(str);
				MQTTPublish(&mqttClient, "heartbeat", &message); 	//publish a message to "heartbeat" topic, just to socket stay alive
				vPortFree((void*)str);								// Releas that memory
			}
		}

//		if(time.sat==0 && time.min==0)		//Reseting Nucleo in midnight, so it can do new initialization with new IP adress
//		{
//			HAL_NVIC_SystemReset();
//		}
		osDelay(20000);
	}
}
/**
 * @brief	StratT3Btn this will be done after Ext Int via button, function is turnning all chargers on.
 * 			Debouncing for taster, adding 1 every time on user number
 * @param   argument: NULL
 * @retval  None
 */
void StartT3Btn(void const * argument)
{
	for(;;)
	{
		vTaskSuspend(NULL);							//suspeding task so it can be resume from ISR
		ChargersOn();								//Turnig chargers ON
		SensorsOn();								//Curently using 5V exit as charger
		u32CurrentMillis=HAL_GetTick();
		if((u32CurrentMillis - u32PreviousMillis) > 200)
		{
			u8NumOfUser++;
		}
		u32PreviousMillis=HAL_GetTick();
		u8NOffUsb1=0;									// reseting number of off state of chargers
		u8NOffUsb2=0;									// reseting number of off state of chargers
		u8NOffWch1=0;									// reseting number of off state of chargers
		u8NOffWch2=0;									// reseting number of off state of chargers
	}
}
/**
 * @brief	StartT4ADC, doing all adc conversion with chargers and calculate the current of each charger individual,
 * 			if on some charger current if low enough for long enough time then it will be turnd OFF
 * @param   argument: NULL
 *
 * @retval  None
 *
 */
void StartT4ADC(void const * argument)
{
	uint8_t prekidac=1;																	//To go thrugh all chargers from 1 to 6
	uint8_t kanal=0;																	//Chargers from 2 to 6 have 2 chanals,go thrugh both chanals

	for(;;)
	{
		HAL_IWDG_Refresh(&hiwdg);														//Regreshing Watchdog timer
		for(kanal=0;kanal<2;kanal++)
		{
			do
			{
				if(prekidac==1 && kanal==0)  											//Switch U7 Main switch, power supply for all other switches
				{
					init_adc1_ch10();													//Initilization for chanal 10 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_12VIN_GPIO_Port, SEL0_12VIN_Pin,RESET);		//Setting select pins on switch 1 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_12VIN_GPIO_Port, SEL1_12VIN_Pin,RESET);
					sw1.I1=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
				}
///////////////////////////////////////////////
				if(prekidac==2 && kanal==0)   											//Switch U8, chanal 1 Router, chanal 2 12V exit
				{
					init_adc3_ch9();													//Initilization for chanal 9 of ADC 3 on Nucleo
					HAL_GPIO_WritePin(SEL0_RA12_GPIO_Port, SEL0_RA12_Pin,RESET);		//Setting select pins on switch 2 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_RA12_GPIO_Port, SEL1_RA12_Pin,RESET);
					sw2.I1=DoADCon3();													//DoADCon3 return the value of conversion puting it in switch struct, ADC3
				}
				if(prekidac==2 && kanal==1)
				{
					init_adc3_ch9();													//Initilization for chanal 9 of ADC 3 on Nucleo
					HAL_GPIO_WritePin(SEL0_RA12_GPIO_Port, SEL0_RA12_Pin, SET);			//Setting select pins on switch 2 to read current of chanal 2
					HAL_GPIO_WritePin(SEL1_RA12_GPIO_Port, SEL1_RA12_Pin, RESET);
					sw2.I2=DoADCon3();													//DoADCon3 return the value of conversion puting it in switch struct, ADC3
				}
//////////////////////////////////////////////
				if(prekidac==3 && kanal==0) 											//Switch U9, USB1 and USB2 12V output
				{
					init_adc1_ch13();													//Initilization for chanal 13 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_USB_GPIO_Port, SEL0_USB_Pin, RESET);			//Setting select pins on switch 3 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_USB_GPIO_Port, SEL1_USB_Pin, RESET);
					sw3.I1=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
					if(sw3.I1<300)														//If current is lower then 300 for more then 3 times repetedly then it will shut down charger
					{
						u8NOffUsb1++;
						if(u8NOffUsb1==3)
						{
							HAL_GPIO_WritePin(USB0_CTRL_GPIO_Port, USB0_CTRL_Pin, RESET);	//Turning USB charger off
							u8NOffUsb1=0;														//Reseting number of off state of charger
						}
					}else
					{
						u8NOffUsb1=0;														//Reseting number of off state of charger
					}
				}
				if(prekidac==3 && kanal==1)
				{
					init_adc1_ch13();													//Initilization for chanal 13 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_USB_GPIO_Port, SEL0_USB_Pin, SET);			//Setting select pins on switch 3 to read current of chanal 2
					HAL_GPIO_WritePin(SEL1_USB_GPIO_Port, SEL1_USB_Pin, RESET);
					sw3.I2=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
					if(sw3.I2<300)														//If current is lower then 300 for more then 3 times repetedly then it will shut down charger
					{
						u8NOffUsb2++;
						if(u8NOffUsb2==3)
						{
							HAL_GPIO_WritePin(USB1_CTRL_GPIO_Port, USB1_CTRL_Pin, RESET);	//turning USB charger off
							u8NOffUsb2=0;														//Reseting number of off state of charger
						}
					}else
					{
						u8NOffUsb2=0;														//Reseting number of off state of charger
					}
				}
/////////////////////////////////////////////
				if(prekidac==4 && kanal==0)  											//u10, 9V, WCh1 and WCh2 control
				{
					init_adc1_ch12();													//Initilization for chanal 12 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_WICH_GPIO_Port, SEL0_WICH_Pin, RESET);		//Setting select pins on switch 4 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_WICH_GPIO_Port, SEL1_WICH_Pin, RESET);
					sw4.I1=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
//					if(sw4.I1<150)																	// Zbog testiranja na trgu, koristimo sens5V i sensor izlaze, pa nam WIch nije potrbno
//					{
//						u8NOffWch1++;
//						if(u8NOffWch1==4)
//						{
//							HAL_GPIO_WritePin(WICH0_CTRL_GPIO_Port, WICH0_CTRL_Pin, 0);		//turning WCH charger off
//							u8NOffWch1=0;														//Reseting number of off state of charger
//						}
//					}else
//					{
//						u8NOffWch1=0;
//					}
				}
				if(prekidac==4 && kanal==1)
				{
					init_adc1_ch12();													//Initilization for chanal 12 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_WICH_GPIO_Port, SEL0_WICH_Pin, SET);			//Setting select pins on switch 4 to read current of chanal 2
					HAL_GPIO_WritePin(SEL1_WICH_GPIO_Port, SEL1_WICH_Pin, RESET);
					sw4.I2=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
//					if(sw4.I2<150)
//					{
//						u8NOffWch2++;
//						if(u8NOffWch2==4)
//						{
//							HAL_GPIO_WritePin(WICH1_CTRL_GPIO_Port, WICH1_CTRL_Pin, 0);		//turning WCH charger off
//							u8NOffWch2=0;														//Reseting number of off state of charger
//						}
//					}else
//					{
//						u8NOffWch2=0;
//					}
				}
///////////////////////////////////////////
				if(prekidac==5 && kanal==0)  											//U11 , LEDRing1 and LEDRing2 control
				{
					init_adc3_ch14();													//Initilization for chanal 14 of ADC 3 on Nucleo
					HAL_GPIO_WritePin(SEL0_LL_GPIO_Port, SEL0_LL_Pin, RESET);			//Setting select pins on switch 5 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_LL_GPIO_Port, SEL1_LL_Pin, RESET);
					sw5.I1=DoADCon3();													//DoADCon3 return the value of conversion puting it in switch struct, ADC3
				}
				if(prekidac==5 && kanal==1)												//Ledring2
				{
					init_adc3_ch14();													//Initilization for chanal 14 of ADC 3 on Nucleo
					HAL_GPIO_WritePin(SEL0_LL_GPIO_Port, SEL0_LL_Pin, SET);				//Setting select pins on switch 5 to read current of chanal 2
					HAL_GPIO_WritePin(SEL1_LL_GPIO_Port, SEL1_LL_Pin, RESET);
					sw5.I2=DoADCon3();													//DoADCon3 return the value of conversion puting it in switch struct, ADC3
				}
///////////////////////////////////////////
				if(prekidac==6 && kanal==0)												//u12 , Sensors and 5V exit
				{
					init_adc1_ch9();													//Initilization for chanal 9 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_SA5_GPIO_Port, SEL0_SA5_Pin, RESET);			//Setting select pins on switch 6 to read current of chanal 1
					HAL_GPIO_WritePin(SEL1_SA5_GPIO_Port, SEL1_SA5_Pin, RESET);
					sw6.I1=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
					if(sw6.I1<150)														//Delete to line 377 for real use, this is for testing purpose on Trg
					{
						u8NOffWch1++;
						if(u8NOffWch1==4)
						{
							HAL_GPIO_WritePin(SENSORS_CTRL_GPIO_Port, SENSORS_CTRL_Pin, RESET);
							u8NOffWch1=0;
						}
					}else
					{
						u8NOffWch1=0;
					}																	// Delete to this lane
				}
				if(prekidac==6 && kanal==1)
				{
					init_adc1_ch9();													//Initilization for chanal 9 of ADC 1 on Nucleo
					HAL_GPIO_WritePin(SEL0_SA5_GPIO_Port, SEL0_SA5_Pin, SET);			//Setting select pins on switch 6 to read current of chanal 2
					HAL_GPIO_WritePin(SEL1_SA5_GPIO_Port, SEL1_SA5_Pin, RESET);
					sw6.I2=DoADCon1();													//DoADCon1 return the value of conversion puting it in switch struct, ADC1
					if(sw6.I2<150)														//Delete to line 396 for real use, this part is for test
					{
						u8NOffWch2++;
						if(u8NOffWch2==4)
						{
							HAL_GPIO_WritePin(AUX_5V_CTRL_GPIO_Port, AUX_5V_CTRL_Pin, RESET);
							u8NOffWch2=0;
						}
					}else
					{
						u8NOffWch2=0;
					}																	//delete to this line
				}
				prekidac++;
			}while(prekidac<=6);														//Circle through all 6 swiches
			prekidac=1;																	//Reset the loop to swich 1
		}
		HAL_IWDG_Refresh(&hiwdg);														//Refeshing WatchDog timer
		osDelay(5000);
	}
}
/**
 * @brief 	MqttConnectBroker connect Nucleo to specific Thing and Topic on aws side,
 * 			Return MQTT_SUCCESS if it succeeded and write message MqttConnectBroker O.K. on serial.
 * @param   argument: Not used
 *
 * @retval  int
 */
int MqttConnectBroker()
{
	int ret;
	TaskHandle_t Task4ADCHandle=NULL;

	net_clear();
	ret = net_init(&net, SERVER_ADDR);
	if(ret != MQTT_SUCCESS)
	{
		printf("net_init failed.\n");
		return -1;
	}

	ret = net_connect(&net, SERVER_ADDR, MQTT_PORT);
	if(ret != MQTT_SUCCESS)
	{
		printf("net_connect failed.\n");
		return -1;
	}

	MQTTClientInit(&mqttClient, &net, 1000, sndBuffer, sizeof(sndBuffer), rcvBuffer, sizeof(rcvBuffer));

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = ""; //no client id required
	data.username.cstring = ""; //no user name required
	data.password.cstring = ""; //no password required
	data.keepAliveInterval = 60;
	data.cleansession = 1;

	ret = MQTTConnect(&mqttClient, &data);
	if(ret != MQTT_SUCCESS)
	{
		printf("MQTTConnect failed.\n");
		return ret;
	}

	ret = MQTTSubscribe(&mqttClient, "test", QOS0, MqttMessageArrived);
	if(ret != MQTT_SUCCESS)
	{
		printf("MQTTSubscribe failed.\n");
		return ret;
	}

	printf("MqttConnectBroker O.K.\n");
	xTaskCreate(StartT4ADC, "Task4ADC", 1024, (void*)0, osPriorityNormal, &Task4ADCHandle);				//Creating Task4, ADC conversion task
	xTaskCreate(StartT3Btn, "Task3Btn", 165, (void*)0, osPriorityNormal, &Task3BtnHandle);				//Creating Task3, turning charger form ISR

	return MQTT_SUCCESS;
}
/**
 * @brief	MqttMessageArrived printing anything that came from AWS to Nucelo board.
 *
 * @param  argument: msg
 *
 * @retval None
 */
void MqttMessageArrived(MessageData* msg)
{
	HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin); //toggle pin when new message arrived

	MQTTMessage* message = msg->message;
	memset(msgBuffer, 0, sizeof(msgBuffer));
	memcpy(msgBuffer, message->payload,message->payloadlen);

	//printf("MQTT MSG[%d]:%s\n", (int)message->payloadlen, msgBuffer);
}
