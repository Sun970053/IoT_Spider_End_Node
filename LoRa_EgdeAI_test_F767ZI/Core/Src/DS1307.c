/*
 * DS1307.c
 *
 *  Created on: Jul 7, 2023
 *      Author: spacelab-cute-PC
 */
#include "DS1307.h"

I2C_HandleTypeDef *ds1307_i2c;

void DS1307_set_clock_halt(uint8_t);
uint8_t DS1307_read_reg_byte(uint8_t);
void DS1307_write_reg_byte(uint8_t, uint8_t);
uint8_t binary2bcd(uint8_t val);
uint8_t bcd2binary(uint8_t val);

uint8_t ds1307_init(I2C_HandleTypeDef* hi2c)
{
	ds1307_i2c = hi2c;
	//DS1307_set_clock_halt(0);
	DS1307_write_reg_byte(DS1307_ADDR_SEC, 0x00);

	uint8_t clock_state = DS1307_read_reg_byte(DS1307_ADDR_SEC);

	return (uint8_t)((clock_state >> 7) & 0x1);
}

void ds1307_set_current_time(RTC_time_t* rtc_time)
{
	uint8_t seconds, hrs;
	seconds = binary2bcd(rtc_time->seconds);
	seconds &= ~(1 << 7);
	DS1307_write_reg_byte(DS1307_ADDR_SEC, seconds);
	DS1307_write_reg_byte(DS1307_ADDR_MIN, binary2bcd(rtc_time->minutes));

	hrs = binary2bcd(rtc_time->hours);
	// p.8 When Bit 6 is high, the 12-hour mode is selected. Otherwise, 24-hour mode.
	if(rtc_time->time_format == TIME_FORMAT_24HRS)
	{
		hrs &= ~(1 << 6);
	}
	else
	{
		hrs |= (1 << 6);
		hrs = (rtc_time->time_format == TIME_FORMAT_12HRS_PM) ? hrs | (1 << 5) : hrs & ~(1 << 5);
	}

	DS1307_write_reg_byte(DS1307_ADDR_HRS, hrs);
}

void ds1307_get_current_time(RTC_time_t* rtc_time)
{
	uint8_t seconds, hrs;

	seconds = DS1307_read_reg_byte(DS1307_ADDR_SEC);
	seconds &= ~(1 << 7);

	rtc_time->seconds = bcd2binary(seconds);
	rtc_time->minutes = bcd2binary(DS1307_read_reg_byte(DS1307_ADDR_MIN));

	hrs = DS1307_read_reg_byte(DS1307_ADDR_HRS);
	// p.8 When Bit 6 is high, the 12-hour mode is selected. Otherwise, 24-hour mode.
	if(hrs & (1 << 6))
	{
		// p.8 12-hour format Bit 5 is the AM/PM bit with logic high being PM.
		rtc_time->time_format = (hrs & ((1 << 5) == 1)) ? TIME_FORMAT_12HRS_PM : TIME_FORMAT_12HRS_AM;
		// Clear bit 5 bit 6
		hrs &= ~(0x3 << 5);
	}
	else
	{
		// 24-hour format
		rtc_time->time_format = TIME_FORMAT_24HRS;
	}

	rtc_time->hours = bcd2binary(hrs);
}

void ds1307_set_current_date(RTC_date_t* rtc_date)
{
	DS1307_write_reg_byte(DS1307_ADDR_DAY, rtc_date->day);
	DS1307_write_reg_byte(DS1307_ADDR_DATE, binary2bcd(rtc_date->date));
	DS1307_write_reg_byte(DS1307_ADDR_MONTH, binary2bcd(rtc_date->month));
	DS1307_write_reg_byte(DS1307_ADDR_YEAR, binary2bcd(rtc_date->year));
}

void ds1307_get_current_date(RTC_date_t* rtc_date)
{
	rtc_date->day = bcd2binary(DS1307_read_reg_byte(DS1307_ADDR_DAY));
	rtc_date->date = bcd2binary(DS1307_read_reg_byte(DS1307_ADDR_DATE));
	rtc_date->month = bcd2binary(DS1307_read_reg_byte(DS1307_ADDR_MONTH));
	rtc_date->year = bcd2binary(DS1307_read_reg_byte(DS1307_ADDR_YEAR));
}



uint8_t DS1307_read_reg_byte(uint8_t regAddr)
{
	uint8_t val;
	HAL_I2C_Master_Transmit(ds1307_i2c, DS1307_I2C_ADDRESS, &regAddr, 1, DS1307_TIMEOUT);
	HAL_I2C_Master_Receive(ds1307_i2c, DS1307_I2C_ADDRESS, &val, 1, DS1307_TIMEOUT);

	return val;
}

void DS1307_write_reg_byte(uint8_t regAddr, uint8_t val)
{
	uint8_t tx[2] = {regAddr, val};
	HAL_I2C_Master_Transmit(ds1307_i2c, DS1307_I2C_ADDRESS, tx, 2, DS1307_TIMEOUT);
}

void DS1307_set_clock_halt(uint8_t halt)
{
	uint8_t ch = (halt ? 1<<7 : 0);
	DS1307_write_reg_byte(DS1307_ADDR_SEC, ch | (DS1307_read_reg_byte(DS1307_ADDR_SEC) & 0x7f));
}

uint8_t binary2bcd(uint8_t val)
{
	uint8_t m, n;
	if(val >= 10)
	{
		m = val/10;
		n = val%10;
		return (uint8_t)((m << 4) | n);
	}
	else
		return val;

}

uint8_t bcd2binary(uint8_t val)
{
	uint8_t m, n;
	m = (val >> 4)*10;
	n = val & (uint8_t)0x0f;
	return m + n;
}
