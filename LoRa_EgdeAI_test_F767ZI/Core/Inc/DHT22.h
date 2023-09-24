/*
 * DHT22.h
 *
 *  Created on: Jun 10, 2023
 *      Author: e1406
 */

#ifndef INC_DHT22_H_
#define INC_DHT22_H_

#include "stm32f4xx_hal.h"

void delay(uint16_t );
void DHT22_Start(void);
uint8_t DHT22_Check_Response(void);
uint8_t DHT22_Read(void);
uint8_t DHT22_Get_Data(float*, float*);


#endif /* INC_DHT22_H_ */
