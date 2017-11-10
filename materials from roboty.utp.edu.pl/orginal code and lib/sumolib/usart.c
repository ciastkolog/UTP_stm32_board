/**
 * Funkcje obslugi portu szeregowego
 */
// jesli usart uruchomiony
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "internal.h"
// jesli usart uruchomiony
#if _USART_ENABLE
/**
 * Piny:
 * PA9 - USART_TX
 * PA10 - USART_RX
 */
// wielkosci buforow
#define _USART_TXBUF_SIZE 1024
#define _USART_RXBUF_SIZE 128
// bufory i pozycje w buforach
__IO uint8_t usartTxBuf[_USART_TXBUF_SIZE];
__IO uint8_t usartRxBuf[_USART_RXBUF_SIZE];
__IO int usartTxEmpty = 0;
__IO int usartTxBusy = 0;
__IO int usartRxEmpty = 0;
__IO int usartRxBusy = 0;

void usartInit() {
	usartTxEmpty = 0;
	usartTxBusy = 0;
	usartRxEmpty = 0;
	usartRxBusy = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	//USART RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// USART TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_SetPriority(USART1_IRQn, 2);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = _USART_SPEED;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}

void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		usartRxBuf[usartRxEmpty] = USART_ReceiveData(USART1);
		usartRxEmpty++;
		if (usartRxEmpty >= _USART_RXBUF_SIZE) {
			usartRxEmpty = 0;
		}
	}

	if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
		if (usartTxBusy != usartTxEmpty) {
			USART_SendData(USART1, usartTxBuf[usartTxBusy]);
			usartTxBusy++;
			if (usartTxBusy >= _USART_TXBUF_SIZE) {
				usartTxBusy = 0;
			}
		} else { //nic do wyslania
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}
}

/**
 * Wysyla bufor na usart
 */
void usartSendChar(char c) {
//	int i;
//	__IO int idx;
//
//	idx = usartTxEmpty;
//	usartTxBuf[idx] = c;
//	idx++;
//	if (idx >= _USART_TXBUF_SIZE) {
//		idx = 0;
//	}
//
//	__disable_irq();
//	if (usartTxEmpty == usartTxBusy) {
//
//		usartTxEmpty = idx;
//		USART_SendData(USART1, usartTxBuf[usartTxBusy]);
//
//		usartTxBusy++;
//		if (usartTxBusy >= _USART_TXBUF_SIZE) {
//			usartTxBusy = 0;
//		}
//
//		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//	} else {
//		usartTxEmpty = idx;
//	}
//	__enable_irq();
	char buff[] = {c, '\0'};
	usartSendRaw(buff, 1);
}

/**
 * Wysyla bufor na usart
 */
void usartSendRaw(char* buff, int len) {
	int i;
	__IO int idx;

	// kopiowanie bufora
	idx = usartTxEmpty;
	for (i = 0; i < len; i++) {
		usartTxBuf[idx] = buff[i];
		idx++;
		if (idx >= _USART_TXBUF_SIZE) {
			idx = 0;
		}
	}

	__disable_irq();
	if (usartTxEmpty == usartTxBusy) {

		usartTxEmpty = idx;
		USART_SendData(USART1, usartTxBuf[usartTxBusy]);

		usartTxBusy++;
		if (usartTxBusy >= _USART_TXBUF_SIZE) {
			usartTxBusy = 0;
		}

		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	} else {
		usartTxEmpty = idx;
	}
	__enable_irq();
}

/**
 * Wysyla tekst na port szeregowy
 */
void usartWrite(char* str) {
	int len = 0;
	while (str[len] != 0 && len < _USART_TXBUF_SIZE) {
		len++;
	}
	usartSendRaw(str, len);
}

/**
 * Zwraca 1 jesli sa jakies odczytane dane
 */
int usartHasData() {
	if (usartRxEmpty == usartRxBusy) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * Zwraca pojedynczy znak
 */
char usartGetChar() {
	uint8_t tmp;
	if (usartRxEmpty != usartRxBusy) {
		tmp = usartRxBuf[usartRxBusy];
		usartRxBusy++;
		if (usartRxBusy >= _USART_RXBUF_SIZE) {
			usartRxBusy = 0;
		}
		return tmp;
	}
	return 0;
}

/**
 * zapisuje w buforze odebrana linijke tekstu
 * zwraca ilosc danych
 */
int usartReadLine(char *buff) {

	static uint8_t bf[_USART_RXBUF_SIZE];
	static uint8_t idx = 0;
	int i;
	int ret;

	while (usartHasData()) {

		bf[idx] = usartGetChar();
		// czy znak konca lini
		if (((bf[idx] == 10) || (bf[idx] == 13))) {
			bf[idx] = 0;
			for (i = 0; i <= idx; i++) {
				//kopiowanie do bufora do bufora
				buff[i] = bf[i];
			}
			ret = idx;
			idx = 0;
			return ret; //odebrano linie

		} else {
			idx++;
			// jesli tekst za dlugi - zawijamy
			if (idx >= _USART_RXBUF_SIZE) {
				idx = 0;
			}
		}
	}
	return 0;
}

#endif // _USART_ENABLE
