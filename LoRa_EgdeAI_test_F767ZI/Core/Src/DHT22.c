/*
 * DHT22.c
 *
 *  Created on: Jun 10, 2023
 *      Author: e1406
 */
#include "stm32f7xx_hal.h"
#include "DHT22.h"


#define DHT22_PORT GPIOB
#define DHT22_PIN GPIO_PIN_1

extern TIM_HandleTypeDef htim6;

uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;

void delay(uint16_t time){
	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while((__HAL_TIM_GET_COUNTER(&htim6)) < time);
}

void Set_Pin_Output(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

//----------------------------------------------------------------
void DHT22_Start(void){
	Set_Pin_Output(DHT22_PORT, DHT22_PIN);
	HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, 0);
	delay(1200); // wait for > 1ms

	HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, 1);
	delay(20);

	Set_Pin_Input(DHT22_PORT, DHT22_PIN);
}

uint8_t DHT22_Check_Response(void){
	uint8_t Response = 0;
	delay(40);
	if(!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))){ // if the pin is low
		delay(80);

		if((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))) Response = 1;
		else Response = -1;
	}

	while((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))); // wait for the pin to go low
	return Response;
}

uint8_t DHT22_Read(void){
	uint8_t i, j;
	for(j = 0; j < 8; j++){
		while(!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))); // wait for the pin to go high
		delay(40);

		if(!(HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))){ // if the pin is low
			i &= ~(1<<(7-j)); // write 0
		}
		else i |= (1<<(7-j)); // write 1
		while((HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN))); // wait for the pin to go low
	}

	return i;
}

uint8_t DHT22_Get_Data(float* Temperature, float* Humidity){
	DHT22_Start();
	if(DHT22_Check_Response()){
		Rh_byte1 = DHT22_Read();
		Rh_byte2 = DHT22_Read();
		Temp_byte1 = DHT22_Read();
		Temp_byte2 = DHT22_Read();
		SUM = DHT22_Read();
		///if(SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2)){
			TEMP = ((Temp_byte1 << 8)|Temp_byte2);
			RH = ((Rh_byte1 << 8)|Rh_byte2);
		//}
		//else return -1;
	}
	else return -1;

	*Humidity = (float)(RH/10.0);
	*Temperature = (float)(TEMP/10.0);

	return 1;
}
