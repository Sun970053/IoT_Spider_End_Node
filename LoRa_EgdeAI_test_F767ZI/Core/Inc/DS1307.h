/*
 * DS1307.h
 *
 *  Created on: Jul 7, 2023
 *      Author: spacelab-cute-PC
 */

#ifndef INC_DS1307_H_
#define INC_DS1307_H_

#include "stm32f767xx.h"
#include "main.h"

/*Register addresses*/
#define DS1307_ADDR_SEC			0x00
#define DS1307_ADDR_MIN			0x01
#define DS1307_ADDR_HRS			0x02
#define DS1307_ADDR_DAY			0x03
#define DS1307_ADDR_DATE		0x04
#define DS1307_ADDR_MONTH		0x05
#define DS1307_ADDR_YEAR		0x06


#define TIME_FORMAT_12HRS_AM 	0
#define TIME_FORMAT_12HRS_PM 	1
#define TIME_FORMAT_24HRS 		2

#define DS1307_I2C_ADDRESS		(0x68 << 1) // p.12 1101000

#define SUNDAY					1;
#define MONDAY					2;
#define TUESDAY					3;
#define WEDNESDAY				4;
#define THURSDAY				5;
#define FRIDAY					6;
#define SATURDAY				7;

#define DS1307_TIMEOUT 			1000

typedef struct
{
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t day;
}RTC_date_t;

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t time_format;
}RTC_time_t;

// Function prototypes

uint8_t ds1307_init(I2C_HandleTypeDef* );
void ds1307_set_current_time(RTC_time_t*);
void ds1307_get_current_time(RTC_time_t*);
void ds1307_set_current_date(RTC_date_t*);
void ds1307_get_current_date(RTC_date_t*);


#endif /* INC_DS1307_H_ */
