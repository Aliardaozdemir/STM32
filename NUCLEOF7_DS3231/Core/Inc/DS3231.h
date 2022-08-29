/*
 * HEADER FILE FOR DS3231 WITH I2C
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_


#include "stm32f7xx_hal.h" /*FOR I2C*/

#define DS3231_ADDRESS 0xD0 /*SLAVE ADDRESS OF DS3231*/

/*ADDRESS MAP*/
#define	SECONDS			0x00
#define	MINUTES			0x01
#define	HOURS			0x02
#define	DAY				0x03
#define	DATE			0x04
#define	MONTH_CENTURY	0x05
#define	YEAR			0x06
#define ALARM_ONE_SEC	0x07
#define	ALARM_ONE_MIN	0x08
#define	ALARM_ONE_HOUR	0x09
#define	ALARM_ONE_DATE	0x0A
#define	ALARM_TWO_MIN	0x0B
#define	ALARM_TWO_HOUR	0x0C
#define ALARM_TWO_DATE	0x0D
#define ALARM_CONTROL	0x0E
#define ALARM_STATUS	0x0F
#define ALARM_AGING_OS	0x10
#define MSB_TEMP		0x11
#define LSB_TEMP		0x12
/*ADDRESS MAP END*/

/*DEFINE SENSOR STRUCT HERE*/
typedef struct{
	I2C_HandleTypeDef *i2cHandle;

	uint8_t seconds;
	uint8_t minutes;
	uint8_t hour;
	uint8_t dayOfWeek;
	uint8_t dayOfMonth;
	uint8_t month;
	uint8_t year;

}DS3231;
/*SENSOR STRUCT END*/

/*FUNCTIONS*/
/*INITIALISE DS3231*/
void DS3231_Init( DS3231 *device, I2C_HandleTypeDef *i2cHandle );

/*DATA WRITTEN TO DS3231 MUST BE IN BCD FORMAT THUS WE NEED THIS FUNCTIONS*/
uint8_t decToBcd(int value);
int bcdToDec(uint8_t value);

/*FUNTION TO SET TIME FOR DS3231*/
void DS3231_Set(DS3231 *device, uint8_t seconds, uint8_t minutes, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year);

/*FUNCTION TO GET TIME FEOM MEMORY*/
void DS3231_Get(DS3231 *device);

/*CONVERT TEMPERATURE BIT OF CONTROL REGISTER TO FORCE CONVERSION*/
void force_Temperature_Conversion(DS3231 *device);

/*FUNCTION FOR GET AND CALCULATE TEMPERATURE FROM MEMORY 0X11 AND 0X12*/
float DS3231_Get_Temperature(DS3231 *device);

/*BASIC FUNCTIONS*/
HAL_StatusTypeDef DS3231_ReadRegister( DS3231 *device, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef DS3231_ReadRegisters( DS3231 *device, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef DS3231_WriteRegister( DS3231 *device, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef DS3231_WriteRegisters( DS3231 *device, uint8_t reg, uint8_t *data, uint8_t length );
/*----------------*/

#endif /* INC_DS3231_H_ */
