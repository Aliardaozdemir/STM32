/*
 * NEO6.h
 * Author: arda
 */

#ifndef INC_NEO6_H_
#define INC_NEO6_H_

#include "stm32f7xx_hal.h"

/* change the size of the buffer */
#define UART_BUFFER_SIZE 512

/*DEFINE STRUCTS*/

typedef struct{
	unsigned char buffer[UART_BUFFER_SIZE];
	volatile unsigned int head, tail;
}RINGBUFFER;

typedef struct{
	int hour, minute, second;
}TIME;

typedef struct{
	float latitude, longitude;
	char NS,EW;
}LOCATION;


typedef struct{
	float altitude;
	char unit;
}ALTITUDE;


typedef struct{
	int day, month, year;
}DATE;


typedef struct{
	LOCATION location_instance;
	TIME time_instance;
	ALTITUDE altitude_instance;
	int isfixValid, numOfSatallite;
}GGASTRUCT;


typedef struct{
	DATE date_instance;
	float speed, course;
	int isValid;
}RMCSTRUCT;


typedef struct{
	GGASTRUCT ggastruct_instance;
	RMCSTRUCT rmcstruct_instance;
}GPSSTRUCT;

/*DEFINE FUNCTIONS*/
void ringBufferInit(void);
void uartWrite(int character);
void uartSendString(const char *s);
void uartPrintBase(long n, uint8_t base);
void getDataFromBuffer(char *startString, char *endString, char *from, char *to);
void uartFlush(void);
void uartIsr(UART_HandleTypeDef *huart);
void store_char(unsigned char c, RINGBUFFER *buffer);
int decodeGGA(char *GGABuffer, GGASTRUCT *gga);
int decodeRMC(char *RMCBuffer, RMCSTRUCT *rmc);
int uartRead(void);
int isAvailable(void);

int uartPeek(void);
int copyUpto(char *string, char *into);
int getAfter(char *str, uint8_t numOfChar, char *toSave);
int waitFor(char *str);



#endif /* INC_NEO6_H_ */
