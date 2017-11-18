#include "../Src/BA66/BA66.h"

/*
 * UART configuration BA66:
 * 	Baudrate		: 9600
 * 	Data Bits		: 8
 * 	Folow Control	: none
 * 	Parity			: odd
 * 	Stop Bits		: 2
 */

void BA66_clean(BA66_HandleTypeDef* handle){
	uint8_t buf[4] = {0x1B, 0x5B , 0x32, 0x4A};
	HAL_UART_Transmit(handle->uart,buf,4,200);
}





