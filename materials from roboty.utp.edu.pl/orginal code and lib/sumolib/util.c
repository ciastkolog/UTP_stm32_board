/**
 * Funkcje roznego przeznaczenia
 */
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "internal.h"
// funkcje konfiguracyjne
void rccAndSystemInit();
void systickInit();
// milisekundy
__IO uint32_t tickCounter = 0;

void init() {
	// inicjalizacja zegarow systemowych
	rccAndSystemInit();

	switchLedInit();
	sensorInit();
	motorInit();
	servoInit();
	timerInit();
#if _USART_ENABLE
	usartInit();
#endif
	systickInit();
}

/**
 * Przerwanie od SysTicka
 */
void SysTick_Handler() {
//	ledOn(LED3);
	// inkrementacja milisekund
	tickCounter++;
//	if ((tickCounter / _SYSTICK_FREQUENCY) % 2 == 1) {
//		ledOn(LED4);
//	} else {
//		ledOff(LED4);
//	}
	// wywolywanie funkcji
	sensorSystickInterrupt();
	motorSystickInterrupt();
	timerSystickInterrupt();
}

void rccAndSystemInit() {
	// inicjalizacja procesora
	SystemInit();

#if _BOOTLOADER_PRESENT
	// ustawienie adresu wektora przerwan
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, _BOOTLOADER_OFFSET);
#endif

	// wlaczenie urzadzen
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	RCC_APB1PeriphClockCmd(
			RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4,
			ENABLE);
	RCC_APB2PeriphClockCmd(
			RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
					| RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO
					| RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1
					| RCC_APB2Periph_USART1, ENABLE);
}

void systickInit() {
	// priorytet dla systicka
	NVIC_SetPriority(SysTick_IRQn, 0);

	// start systicka
	SysTick_Config(SystemCoreClock / _SYSTICK_FREQUENCY);
}

/**
 * Oczekiwanie okreslonej ilosci milisekund
 */
void waitMs(int ms) {
	uint32_t startTime = tickCounter;
	while (startTime + (ms * _SYSTICK_FREQUENCY) / 1000 > tickCounter)
		;
}

/**
 * Oczekiwanie okreslonej ilosci sekund
 */
void wait(int s) {
	uint32_t startTime = tickCounter;
	while (startTime + s * _SYSTICK_FREQUENCY > tickCounter)
		;
}

/**
 * Wartosc pseudolosowa
 * range - zakres wartosci
 */
int randomValue(int range) {
	// TODO zmienic na bardziej skomplikowane
	return tickCounter % range;
}

/**
 * Ilosc milisekund od startu programu
 */
int timeInMilis() {
	return (tickCounter * 1000) / _SYSTICK_FREQUENCY;
}
