/*
 * BME280 driver file for STM32 NUCLEO L476RG WITH I2C
 * Author: ali arda özdemir
 */

#include "BME280.h"

/*COMPENSATION PARAMETERS*/
uint16_t	dig_T1,dig_P1, \
 			dig_H1,dig_H3;

int16_t		dig_T2, dig_T3, \
 			dig_P2, dig_P3, \
 			dig_P4, dig_P5, \
 			dig_P6, dig_P7, \
 			dig_P8, dig_P9, \
 			dig_H2, dig_H4, \
 			dig_H5, dig_H6  ;
/*COMPENSTATION PARAMETERS END */

/*-----------------------------*/

int32_t temperatureRaw, pressureRaw, humidityRaw;

uint8_t BME280_Init( BME280 *device, I2C_HandleTypeDef *i2cHandle ){

	device->i2cHandle = i2cHandle;

	device->temperature_C = 0.0f;
	device->pressure_hPa  = 0.0f;
	device->humidity_RH   = 0.0f;

	uint8_t errors = 0;
	HAL_StatusTypeDef status;

	uint8_t registerData;

	/*READ ID REGISTER AND CHECK IF VALUE EQUALS TO REAL ID*/
	/*READ*/
	status = BME280_ReadRegister( device, BME280_ID_ADDRESS, &registerData);
	errors += (status != HAL_OK);

	if(registerData != BME280_ID_VALUE){
		return 255;
	}

	trimRead( device );

	coldReset( device );

	/*------------------------------------------------------------------------------------------------------------------------------------
	 *
	 * osrs_h,p,t[2:0]        model[1:0]             t_sb[2:0]				filter[2:0]
	 * 000 - skipped          00 - sleep mode        000 - 0.5  m.s.		000 - off
	 * 001 - O.S.X1           01 - forced mode       001 - 62.5 m.s.        001 - 2 (coefficient)
	 * 010 - O.S.X2           10 - forced mode       010 - 125  m.s.        010 - 4
	 * 011 - O.S.X4           11 - normal mode       011 - 250  m.s.        011 - 8
	 * 100 - O.S.X8                                  100 - 500  m.s.        100 - 16
	 * 101 - O.S.X16                                 101 - 1000 m.s.        others 16
	 * OTHERS - X16                                  110 - 10   m.s.
	 *                                               111 - 20   m.s.
	 *
	 *
	 -------------------------------------------------------------------------------------------------------------------------------------*/

	/*WRITE HUMIDITY OVERSAMPLING TO CTRL_HUM*/
	registerData = 0x01;
	status = BME280_WriteRegister( device, BME280_CTRL_HUM, &registerData  );
	errors += (status != HAL_OK);
	HAL_Delay(100);

	/*SETUP STANDBY TIME AND IIR FILTER*/
	registerData = (0x00 << 5) | (0x04 << 2);
	status = BME280_WriteRegister( device, BME280_CONFIG, &registerData );
	errors += (status != HAL_OK);
	HAL_Delay(100);

	/*SET UP PRESSURE & TEMPERATURE OVERSAMPLING*/
	registerData = (0x02 <<5) |(0x05 << 2) | 0x03;
	status = BME280_WriteRegister( device, BME280_CTRL_MEAS, &registerData  );
	errors += (status != HAL_OK);
	HAL_Delay(100);

	return errors;
}/*END*/







void trimRead( BME280 *device ){
	uint8_t trimData[32]; /*WILL READ 32 MEMORY LOCATION THAN WILL STORE 8 BIT VALUES IN LOCATIONS*/

	/*READ VALUES FROM 0X88 TO 0XA1		DATASHEET PAGE 24*/
	BME280_ReadRegisters( device, 0x88, trimData, 25 );

	/*READ VALUES FROM 0xE1 TO 0xE7		DATASHEET PAGE 24*/
	BME280_ReadRegisters( device, 0xE1, (uint8_t *)trimData+25, 7 );

	/*CALIBRATE DATA 	DATASHEET PAGE 24*/
	dig_T1 = (trimData[1]<<8) | trimData[0];
	dig_T2 = (trimData[3]<<8) | trimData[2];
	dig_T3 = (trimData[5]<<8) | trimData[4];

	dig_P1 = (trimData[7]<<8) | trimData[5];
	dig_P2 = (trimData[9]<<8) | trimData[6];
	dig_P3 = (trimData[11]<<8) | trimData[10];
	dig_P4 = (trimData[13]<<8) | trimData[12];
	dig_P5 = (trimData[15]<<8) | trimData[14];
	dig_P6 = (trimData[17]<<8) | trimData[16];
	dig_P7 = (trimData[19]<<8) | trimData[18];
	dig_P8 = (trimData[21]<<8) | trimData[20];
	dig_P9 = (trimData[23]<<8) | trimData[22];

	dig_H1 = trimData[24];
	dig_H2 = (trimData[26]<<8) | trimData[25];
	dig_H3 = (trimData[27]);
	dig_H4 = (trimData[28]<<4) | (trimData[29] & 0x0f);
	dig_H5 = (trimData[30]<<4) | (trimData[29]>>4);
	dig_H6 = (trimData[31]);

}/*END*/





void coldReset( BME280 *device ){
	uint8_t datatowrite = 0xB6;

	BME280_WriteRegister( device, BME280_RESET, &datatowrite  );
}/*reset*/

void readRaw( BME280 *device ){
	uint8_t rawData[8];

	/*READ REGISTERS 0xF7 to 0xFC TO BURST READ 	DATASHEET PAGE 23*/
	BME280_ReadRegisters( device,BME280_PRESS_MSB, rawData, 8 );

	/*TEMPERATURE & PRESSURE 20 BIT , HUMID 16 BIT CHECK PAGE 27 OF DATASHEET*/
	pressureRaw    = (rawData[0]<<12)|(rawData[1]<<4)|(rawData[2]>>4);
	temperatureRaw = (rawData[3]<<12)|(rawData[4]<<4)|(rawData[5]>>4);
	humidityRaw    = (rawData[6]<<8)|(rawData[7]);

}/*END*/

/*-----------------------DATASHEET PAGE 25 CALCULATIONS ----------------------------------------------------------------------------------*/
/*RETURN TEMPERATURE*/
int32_t t_fine;
int32_t BME280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1)))>> 12) *((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}
/*RETURN PRESSURE*/
uint32_t BME280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) *var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}
/*RETURN HUMIDITY*/
uint32_t bme280_compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) *\
			v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *\
					((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +\
							((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) +\
					8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *\
			((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}


/*----------------------------------------------------------------------------------------------------------------------------------------*/
void measureData( BME280 *device ){
	if (temperatureRaw == 0x800000){
		device->temperature_C = 0;
	}
	else{
		device->temperature_C = ( BME280_compensate_T_int32(temperatureRaw) ) / 100.0;
	}

	if (pressureRaw == 0x800000){
		device->pressure_hPa = 0;
	}
	else{
		device->pressure_hPa = ( BME280_compensate_P_int32(pressureRaw) );
	}

	if (humidityRaw == 0x8000){
		device->humidity_RH = 0;
	}
	else{
		device->humidity_RH = ( bme280_compensate_H_int32(humidityRaw)) / 1024.0;
	}
}/*END*/


/*BASIC READ AND WRITE FUNCTIONS*/

HAL_StatusTypeDef BME280_ReadRegister( BME280 *device, uint8_t reg, uint8_t *data ){
	return HAL_I2C_Mem_Read( device->i2cHandle, BME280_IC2_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}/*end*/

HAL_StatusTypeDef BME280_ReadRegisters( BME280 *device, uint8_t reg, uint8_t *data, uint8_t length ){
	return HAL_I2C_Mem_Read( device->i2cHandle, BME280_IC2_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY );
}/*end*/

HAL_StatusTypeDef BME280_WriteRegister( BME280 *device, uint8_t reg, uint8_t *data ){
	return HAL_I2C_Mem_Write( device->i2cHandle, BME280_IC2_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY );
}/*end*/
