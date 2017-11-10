/**
 * Funkcje obslugi diod LED, przyciskow i przelacznikow
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "internal.h"

/**
 * Piny:
 * PB0 - LED1
 * PB1 - LED2
 * PB10 - LED3
 * PB11 - LED4
 * PC15 - LED5
 * PC14 - LED6
 * PC13 - LED7
 * PB5 - LED8
 * PB4 - LED9
 * PB3 - LED10
 * PA15 - LED11
 * PD2 - LED12 - USB_ON
 * PA12 - LED13 - USB_DP
 * PA11 - LED14 - USB_DM
 * PC4 - BUTTON1
 * PC5 - BUTTON2
 * PC12 - SW1
 * PC11 - SW2
 * PB2 - SW3 - BOOT1
 */

#define _LED_FIRST LED1
#if !_USB_ENABLE && _LED_EXT_ENABLE
#define _LED_LAST LED14
#else
#define _LED_LAST LED11
#endif

void switchLedInit() {
	// inicjalizacja portow przyciskow i przelacznikow
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11
			| GPIO_Pin_12;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// wylaczenie pinow JTAGa
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	// inicjalizacja portow diod LED
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3
			| GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#if !_USB_ENABLE && _LED_EXT_ENABLE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
	// wygaszenie wszystkich
	int i;
	for (i = _LED_FIRST; i <= _LED_LAST; i++) {
		ledSet(i, LED_OFF);
	}
}

/**
 * Sprawdzenie czy przycisk jest nacisniety
 * buttonNumber - numer przycisku
 * zwraca 1 jesli tak, 0 jesli nie
 */
int buttonPressed(int buttonNumber) {
	switch (buttonNumber) {
	case BUTTON1:
		return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4);
	case BUTTON2:
		return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5);
	}
	return 0;
}

/**
 * Oczekiwanie na nacisniecie przycisku
 * buttonNumber - numer przycisku
 */
void waitForButton(int buttonNumber) {
	while (!buttonPressed(buttonNumber));
}

/**
 * Sprzwdzenie, czy przelacznik jest wlaczony
 * switchNumber - numer przelacznika
 * zwraca 1 jesli tak, 0 jesli nie
 */
int switchIsOn(int switchNumber) {
	switch (switchNumber) {
	case SWITCH1:
		return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_12);
	case SWITCH2:
		return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11);
	case SWITCH3:
		return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_2);
	}
	return 0;
}

/**
 * Zapalenie okreslonej diody
 * ledNumber - numer diody
 */
void ledOn(int ledNumber) {
	ledSet(ledNumber, LED_ON);
}

/**
 * Gaszenie okreslonej diody
 * ledNumber - numer diody
 */
void ledOff(int ledNumber) {
	ledSet(ledNumber, LED_OFF);
}

/**
 * Ustawienie stanu okreslonej diody
 * ledNumber - numer diody
 * ledState - stan diody 0 - zgaszona, 1 - zapalona
 */
void ledSet(int ledNumber, int ledState) {
	switch (ledNumber) {
	case LED1:
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, !ledState);
		break;
	case LED2:
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, !ledState);
		break;
	case LED3:
		GPIO_WriteBit(GPIOB, GPIO_Pin_10, !ledState);
		break;
	case LED4:
		GPIO_WriteBit(GPIOB, GPIO_Pin_11, !ledState);
		break;
	case LED5:
		GPIO_WriteBit(GPIOC, GPIO_Pin_15, !ledState);
		break;
	case LED6:
		GPIO_WriteBit(GPIOC, GPIO_Pin_14, !ledState);
		break;
	case LED7:
		GPIO_WriteBit(GPIOC, GPIO_Pin_13, !ledState);
		break;
	case LED8:
		GPIO_WriteBit(GPIOB, GPIO_Pin_5, !ledState);
		break;
	case LED9:
		GPIO_WriteBit(GPIOB, GPIO_Pin_4, !ledState);
		break;
	case LED10:
		GPIO_WriteBit(GPIOB, GPIO_Pin_3, !ledState);
		break;
	case LED11:
		GPIO_WriteBit(GPIOA, GPIO_Pin_15, !ledState);
		break;
#if !_USB_ENABLE && _LED_EXT_ENABLE
		case LED12:
		GPIO_WriteBit(GPIOD, GPIO_Pin_2, !ledState);
		break;
		case LED13:
		GPIO_WriteBit(GPIOA, GPIO_Pin_12, !ledState);
		break;
		case LED14:
		GPIO_WriteBit(GPIOA, GPIO_Pin_11, !ledState);
		break;
#endif
	}

}

