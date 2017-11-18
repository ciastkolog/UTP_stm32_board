#ifndef _BA66_H_
#define _BA66_H_

#include "stm32f1xx_hal.h"

typedef struct{
	UART_HandleTypeDef 		*uart;
	uint8_t					uartBuf[300];
	uint8_t					uartRX[10];
	uint8_t					newLineFlag;
	uint8_t					UART_STATUS;
	volatile uint8_t					UART_INDEX;
	//uint8_t					defaultSSID;
	//uint8_t					defaultPASSWORD;
} BA66_HandleTypeDef;
void BA66_clean(BA66_HandleTypeDef* handle);
#endif /* _BA66_H_ */
