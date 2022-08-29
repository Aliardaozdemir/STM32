/*
 * NEO6.c
 * Author: arda
 */

/*INCLUDES*/
#include "NEO6.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/*DEFINES*/
extern UART_HandleTypeDef huart3;
#define uart &huart3
#define TIMEOUT_DEF 500

/*VARIABLES*/
uint16_t timeout;
int GMT = +530, inx = 0, hour = 0, minute = 0, day = 0, month = 0, year = 0, daychange = 0;

RINGBUFFER RX = {{0}, 0, 0};
RINGBUFFER TX = {{0}, 0, 0};

RINGBUFFER *RXPTR;
RINGBUFFER *TXPTR;




/*FUNCTION DEFINITIONS*/
int decodeGGA(char *GGABuffer, GGASTRUCT *gga){
	inx = 0;
	char buffer[12];
	int i = 0;
	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	for(;GGABuffer[inx] != ',';inx++);
	inx++;

	if((GGABuffer[inx] == '1')||(GGABuffer[inx] == '2') || (GGABuffer[inx] == '6')){
		gga->isfixValid = 1;/*fix available*/ inx = 0;
	}

	else{
		gga->isfixValid = 0;/*fix is not available*/ return 1;
	}



	for(;GGABuffer[inx] != ',';inx++);

	/*--------------------------------------GET TIME-----------------------------------------*/
	inx++;
	memset(buffer,'\0',12);
	i = 0;
	for(;GGABuffer != ',';buffer[i] = GGABuffer[inx], i++, inx++);

	hour = (atoi(buffer)/1000) + GMT/100;
	minute = ((atoi(buffer)/100)%100) + GMT%100;

	gga->time_instance.hour = hour;
	gga->time_instance.minute = minute;
	gga->time_instance.second = atoi(buffer)%100;
	/*------------------------------------GET LATITUDE--------------------------------------*/
	inx++;
	memset(buffer,'\0',12);
	i=0;

	for(;GGABuffer[inx] != ',';buffer[i] == GGABuffer[inx], i++, inx++);

	if(strlen(buffer)<6) return 2;

	int16_t num = (atoi(buffer));
	int j = 0;

	for(;buffer[j] != '.';j++ ); j++;

	int declen = (strlen(buffer)) -j;
	int dec = atoi((char *)buffer + j);
	float lat = (num/100.0) + (dec/pow(10, (declen+2)));
	gga->location_instance.latitude = lat; inx++;
	gga->location_instance.NS = GGABuffer[inx];
	/*-------------------------------GET LONGITUDE---------------------------------------------*/
	inx += 2;
	memset(buffer, '\0', 12);
	i = 0;
	for(;GGABuffer[inx] != ','; buffer[i] = GGABuffer[inx], i++, inx++ );
	num = (atoi(buffer));
	j = 0;

	for(;buffer[j] != '.'; j++); j++;

	declen = (strlen(buffer))-j;
	dec = atoi ((char *) buffer+j);
	lat = (num/100.0) + (dec/pow(10, (declen+2)));
	gga->location_instance.longitude = lat; inx++;
	gga->location_instance.EW = GGABuffer[inx];

	inx += 4;
	memset(buffer, '\0', 12);
	i=0;

	for(;GGABuffer[inx] != ',';buffer[i] = GGABuffer[inx], i++, inx++);

	gga->numOfSatallite = atoi(buffer);

	inx++;

	for(;GGABuffer[inx] != '!';inx++);

	/*-----------------------------calculate altitude--------------------------------------*/
	inx++;
	memset(buffer,'\0',12);
	i=0;

	for(;GGABuffer[inx] != ','; buffer[i] = GGABuffer[inx], i++, inx++ );

	num = (atoi(buffer));
	j = 0;

	for(;buffer[j] != '.'; j++); j++;

	declen = (strlen(buffer))-j;
	dec = atoi ((char *) buffer+j);
	lat = (num) + (dec/pow(10, (declen)));
	gga->altitude_instance.altitude = lat;	inx++;
	gga->altitude_instance.unit = GGABuffer[inx];	return 0;


}/*END*/

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
int decodeRMC(char *RMCBuffer, RMCSTRUCT *rmc){
	inx = 0;
	char buffer[12];
	int i = 0;

	for(int inc = 0; inc <= 1; inc++){
		for(;RMCBuffer[inx] != ',';inx++);	inx++;
	}



	if(RMCBuffer[inx] == 'A'){
		rmc->isValid = 1;
	}
	else
		rmc->isValid = 0;

	if(rmc->isValid == 0) return 1;

	inx += 2;

	for(int inc = 0; inc <= 3; inc++){
		for(;RMCBuffer[inx] != ',';inx++); inx++;
	}

	/*---------------------GET SPEED-------------------------------------------*/
	inx++;
	i=0;
	memset(buffer,'\0',12);
	for(;RMCBuffer != ','; buffer[i] = RMCBuffer[inx], i++, inx++);

	if(strlen(buffer) > 0){
		int16_t num = (atoi(buffer));
		int j = 0;
		for(;buffer[j] != '.';j++); j++;
		int declen = (strlen(buffer))-j;
		int dec = atoi ((char *) buffer+j);
		float lat = num + (dec/pow(10, (declen)));
		rmc->speed = lat;
	}

	else rmc->speed = 0;

	inx++;
	i = 0;
	memset(buffer,'\0',12);
	for(;RMCBuffer[inx] != ','; buffer[i] = RMCBuffer[inx], i++, inx++);

	if (strlen (buffer) > 0){
		int16_t num = (atoi(buffer));
		int j = 0;

		for(;buffer[j] != '.';j++); j++;

		int declen = (strlen(buffer))-j;
		int dec = atoi ((char *) buffer+j);
		float lat = num + (dec/pow(10, (declen)));
		rmc->course = lat;
	}

	else
		rmc->course = 0;

	/*get date */
	inx++;
	i=0;
	memset(buffer, '\0', 12);

	for(;RMCBuffer[inx] != ',';buffer[i] = RMCBuffer[inx], i++, inx++);

	day = atoi(buffer)/10000;
	month = (atoi(buffer)/100)%100;
	year = atoi(buffer)%100;

	day = day+daychange;

	rmc->date_instance.day = day;
	rmc->date_instance.month = month;
	rmc->date_instance.year = year;

	return 0;
}/*END*/

void ringBufferInit(void){
	RXPTR = &RX;
	TXPTR = &TX;

	/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	__HAL_UART_ENABLE_IT(uart, UART_IT_ERR);

	/* Enable the UART Data Register not empty Interrupt */
	__HAL_UART_ENABLE_IT(uart, UART_IT_RXNE);

}/*END*/
void store_char(unsigned char c, RINGBUFFER *buffer){
	int i = (unsigned int)(buffer->head + 1) % UART_BUFFER_SIZE;

	if(i != buffer->tail) {
	   buffer->buffer[buffer->head] = c;
	   buffer->head = i;
	 }
}/*END*/


void uartWrite(int character){
	if (character>=0)
	{
		int i = (TXPTR->head + 1) % UART_BUFFER_SIZE;
		while (i == TXPTR->tail);

		TXPTR->buffer[TXPTR->head] = (uint8_t)character;
		TXPTR->head = i;

		__HAL_UART_ENABLE_IT(uart, UART_IT_TXE); // Enable UART transmission interrupt
	}

}/*END*/
void uartSendString(const char *s){
	while(*s) uartWrite(*s++);

}/*END*/
void uartPrintBase(long n, uint8_t base){

}/*END*/
void getDataFromBuffer(char *startString, char *endString, char *from, char *to){
	int startStringLength = strlen (startString);
	int endStringLength   = strlen (endString);
	int so_far = 0;
	int indx = 0;
	int startposition = 0;
	int endposition = 0;

repeat1:
	while (startString[so_far] != from[indx]) indx++;
	if (startString[so_far] == from[indx])
	{
		while (startString[so_far] == from[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == startStringLength) startposition = indx;
	else
	{
		so_far =0;
		goto repeat1;
	}

	so_far = 0;

repeat2:
	while (endString[so_far] != from[indx]) indx++;
	if (endString[so_far] == from[indx])
	{
		while (endString[so_far] == from[indx])
		{
			so_far++;
			indx++;
		}
	}

	if (so_far == endStringLength) endposition = indx-endStringLength;
	else
	{
		so_far =0;
		goto repeat2;
	}

	so_far = 0;
	indx=0;

	for (int i=startposition; i<endposition; i++)
	{
		to[indx] = from[i];
		indx++;
	}

}/*END*/
void uartFlush(void){
	memset(RXPTR->buffer,'\0', UART_BUFFER_SIZE);
	RXPTR->head = 0;
	RXPTR->tail = 0;

}/*END*/
void uartIsr(UART_HandleTypeDef *huart){

	 uint32_t isrflags   = READ_REG(huart->Instance->ISR);
	 uint32_t cr1its     = READ_REG(huart->Instance->CR1);

  /* if DR is not empty and the Rx Int is enabled */
  if (((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
  {

		huart->Instance->ISR;                       /* Read status register */
      unsigned char c = huart->Instance->RDR;     /* Read data register */
      store_char (c, RXPTR);  // store data in buffer
      return;
  }


  if (((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
  {
  	if(TX.head == TX.tail)
  	    {

  	      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

  	    }

  	 else
  	    {

  	      unsigned char c = TX.buffer[TX.tail];
  	      TX.tail = (TX.tail + 1) % UART_BUFFER_SIZE;


  	      huart->Instance->ISR;
  	      huart->Instance->RDR = c;

  	    }
  	return;
  }
}/*END*/

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

int uartRead(void){

	if(RXPTR->head == RXPTR->tail)
	{
	  return -1;
	}
	else
	{
	  unsigned char c = RXPTR->buffer[RXPTR->tail];
	  RXPTR->tail = (unsigned int)(RXPTR->tail + 1) % UART_BUFFER_SIZE;
	  return c;
	}

}/*END*/
int isAvailable(void){
	return (uint16_t)(UART_BUFFER_SIZE + RXPTR->head - RXPTR->tail) % UART_BUFFER_SIZE;
}/*END*/


int uartPeek(void){
	if(RXPTR->head == RXPTR->tail)
	 {
	   return -1;
	 }
	 else
	 {
	   return RXPTR->buffer[RXPTR->tail];
	 }

}/*END*/

int copyUpto(char *string, char *into){

	int so_far =0;
	int len = strlen (string);
	int indx = 0;

again:
	while (uartPeek() != string[so_far])
		{
			into[indx] = RXPTR->buffer[RXPTR->tail];
			RXPTR->tail = (unsigned int)(RXPTR->tail + 1) % UART_BUFFER_SIZE;
			indx++;
			while (!isAvailable());

		}
	while (uartPeek() == string [so_far])
	{
		so_far++;
		into[indx++] = uartRead();
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!isAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;
}/*END*/

int getAfter(char *str, uint8_t numOfChar, char *toSave){

	for (int indx=0; indx<numOfChar; indx++)
	{
		timeout = TIMEOUT_DEF;
		while ((!isAvailable())&&timeout);  // wait until some data is available
		if (timeout == 0) return 0;  // if data isn't available within time, then return 0
		toSave[indx] = uartRead();  // save the data into the buffer... increments the tail
	}
	return 1;

}/*END*/

int waitFor(char *str){
	int so_far =0;
	int len = strlen (str);

again:
	timeout = TIMEOUT_DEF;
	while ((!isAvailable())&&timeout);  // let's wait for the data to show up
	if (timeout == 0) return 0;
	while (uartPeek() != str[so_far])  // peek in the rx_buffer to see if we get the string
	{
		if (RXPTR->tail != RXPTR->head)
		{
			RXPTR->tail = (unsigned int)(RXPTR->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		}

		else
		{
			return 0;
		}
	}
	while (uartPeek() == str[so_far]) // if we got the first letter of the string
	{
		// now we will peek for the other letters too
		so_far++;
		RXPTR->tail = (unsigned int)(RXPTR->tail + 1) % UART_BUFFER_SIZE;  // increment the tail
		if (so_far == len) return 1;
		timeout = TIMEOUT_DEF;
		while ((!isAvailable())&&timeout);
		if (timeout == 0) return 0;
	}

	if (so_far != len)
	{
		so_far = 0;
		goto again;
	}

	if (so_far == len) return 1;
	else return 0;

}/*END*/



