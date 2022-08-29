/*
 * SOURCE FILE FOR DS3231 WITH I2C
 */

#include "DS3231.h"

/*FUNCTIONS*/

void DS3231_Init(DS3231 *device , I2C_HandleTypeDef *i2cHandle){

	device->i2cHandle = i2cHandle;

	device->seconds 	= 0;
	device->minutes 	= 0;
	device->hour		= 0;
	device->dayOfWeek   = 0;
	device->dayOfMonth  = 0;
	device->month    	= 0;
	device->year    	= 0;

}/*end of function*/


uint8_t decToBcd(int value){
	return (uint8_t)((value/10*16) + (value%10));
}/*end of function*/

int bcdToDec(uint8_t value){
	return (int)((value/16*10) + (value%16));
}/*end of function*/


void DS3231_Set(DS3231 *device, uint8_t seconds, uint8_t minutes, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year){
	uint8_t setDS3231[7];

	/*SECONDS-MINUTES-HOURS-DAY-DATE-MONTH : 00H TO 06H*/
	setDS3231[0] = decToBcd(seconds);
	setDS3231[1] = decToBcd(minutes);
	setDS3231[2] = decToBcd(hour);
	setDS3231[3] = decToBcd(dayOfWeek);
	setDS3231[4] = decToBcd(dayOfMonth);
	setDS3231[5] = decToBcd(month);
	setDS3231[6] = decToBcd(year);

	/*WRITE VALUES TO REGISTERS 0X00 TO 0X06*/
	DS3231_WriteRegisters(device, SECONDS	, setDS3231, 7 );
}/*end of function*/


void DS3231_Get(DS3231 *device){
	uint8_t getDS3231[7];

	DS3231_ReadRegisters(device, SECONDS	, getDS3231, 7);
	device->seconds 	= bcdToDec(getDS3231[0]);
	device->minutes 	= bcdToDec(getDS3231[1]);
	device->hour		= bcdToDec(getDS3231[2]);
	device->dayOfWeek   = bcdToDec(getDS3231[3]);
	device->dayOfMonth  = bcdToDec(getDS3231[4]);
	device->month    	= bcdToDec(getDS3231[5]);
	device->year    	= bcdToDec(getDS3231[6]);
}/*end of function*/

void force_Temperature_Conversion(DS3231 *device){
	uint8_t status = 0;
	uint8_t control = 0;

	DS3231_ReadRegister(device, ALARM_STATUS, &status );

	/*IF BSY BIT NOT SET READ CONTROL REGISTER*/
	if(!(status&0x04)){
		DS3231_ReadRegister(device, ALARM_CONTROL, &control );
		DS3231_WriteRegister(device, ALARM_CONTROL, (uint8_t *)(control|(0x20)) );
	}
}

float DS3231_Get_Temperature(DS3231 *device){

	uint8_t temperature[2];
	/*READ MEM 0x11 to 0x12*/
	DS3231_ReadRegisters(device, MSB_TEMP, temperature, 2 );
	/*COMBINE MSB AND LSB*/
	return ((temperature[0]) + (temperature[1]>>6)/4.0); /*RESOLUTION IS 0.25 DEGREES CELCIUS PER BIT THUS RESULT IS DIVIDED BY 4*/
}/*end of function*/


/*BASIC READ AND WRITE FUNCTIONS*/

HAL_StatusTypeDef DS3231_ReadRegister( DS3231 *device, uint8_t reg, uint8_t *data ){
	return HAL_I2C_Mem_Read( device->i2cHandle, DS3231_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}/*end*/

HAL_StatusTypeDef DS3231_ReadRegisters( DS3231 *device, uint8_t reg, uint8_t *data, uint8_t length ){
	return HAL_I2C_Mem_Read( device->i2cHandle, DS3231_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY );
}/*end*/

HAL_StatusTypeDef DS3231_WriteRegister( DS3231 *device, uint8_t reg, uint8_t *data ){
	return HAL_I2C_Mem_Write( device->i2cHandle, DS3231_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}/*end*/

HAL_StatusTypeDef DS3231_WriteRegisters( DS3231 *device, uint8_t reg, uint8_t *data, uint8_t length ){
	return HAL_I2C_Mem_Write( device->i2cHandle, DS3231_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY );
}/*end*/
