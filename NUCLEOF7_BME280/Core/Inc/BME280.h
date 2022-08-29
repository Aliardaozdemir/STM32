/*
 *  BME280 driver header file for STM32 NUCLEO L476RG WITH I2C
 *  Author: ali arda özdemir
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include "stm32l4xx_hal.h" /*FOR I2C*/

/*DEFINE NECESSARY CONSTANTS*/
#define BME280_IC2_ADDRESS	(0x76 << 1)		/*IF SDO TO VDDIO, ADRESS = 0X77		DATASHEET PAGE 32*/
#define BME280_ID_VALUE		0x60			/*DATASHEET PAGE 27*/

/*REGISTER MAP	DATASHEET PAGE 27*/
#define	BME280_HUM_LSB		0xFE
#define	BME280_HUM_MSB		0xFD
#define	BME280_TEMP_XLSB	0xFC
#define	BME280_TEMP_LSB		0xFB
#define	BME280_TEMP_MSB		0xFA
#define	BME280_PRESS_XLSB	0xF9
#define	BME280_PRESS_LSB	0xF8
#define BME280_PRESS_MSB	0xF7
#define	BME280_CONFIG		0xF5
#define	BME280_CTRL_MEAS	0xF4
#define	BME280_STATUS		0xF3
#define	BME280_CTRL_HUM		0xF2
#define	BME280_RESET		0xE0
#define BME280_ID_ADDRESS	0xD0
/*REGISTER MAP END*/





/*-----------------------------*/

/*DEFINE SENSOR STRUCT*/
typedef struct{
	I2C_HandleTypeDef *i2cHandle;

	float temperature_C;
	float pressure_hPa;
	float humidity_RH;

}BME280;   /*END OF STRUCT*/

void coldReset( BME280 *device );
/*FOR COLD RESET*/
void trimRead( BME280 *device );
/*READ THE TRIMMING PARAMETERS INTO COMPESATION PARAMETERS*/

uint8_t BME280_Init( BME280 *device, I2C_HandleTypeDef *i2cHandle );
/*INITIALISE SENSOR - DATA ACQ.*/

void readRaw( BME280 *device );
/*READ RAW DATA*/
void measureData( BME280 *device );
/*DO CALCULATIONS ON RAW DATA TO GET REAL VALUES*/

/*BASIC FUNCTIONS*/
HAL_StatusTypeDef BME280_ReadRegister( BME280 *device, uint8_t reg, uint8_t *data );
HAL_StatusTypeDef BME280_ReadRegisters( BME280 *device, uint8_t reg, uint8_t *data, uint8_t length );
HAL_StatusTypeDef BME280_WriteRegister( BME280 *device, uint8_t reg, uint8_t *data );
/*----------------*/

/*COMPENSATE FUNCTIONS*/
int32_t BME280_compensate_T_int32(int32_t adc_T);
/*COMPENSATE TEMPERATURE*/
uint32_t BME280_compensate_P_int32(int32_t adc_P);
/*COMPENSATE PRESSURE*/
uint32_t bme280_compensate_H_int32(int32_t adc_H);
/*COMPENSATE HUMIDITY*/


#endif /* INC_BME280_H_ */
