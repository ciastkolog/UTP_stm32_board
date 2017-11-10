/**
 * Funkcje obslugi czujnikow
 */
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "internal.h"
/**
 * Piny:
 * PC3 - DIST1 - IN13
 * PC2 - DIST2 - IN12
 * PC1 - DIST3 - IN11
 * PC0 - DIST4 - IN10
 * PA4 - AIN1 - IN4
 * PA5 - AIN2 - IN3
 * PA6 - AIN3 - IN2
 * PA7 - AIN4 - IN1
 * PB15 - GRD1
 * PB14 - GRD2
 * PB13 - GRD3
 * PB12 - GRD4
 * PC10 - GRD5
 * PA8- GRD_PWM - TIM1_CH1
 */
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#if _USE_EXT_DIST
#define _CHANNEL_NUMBER 8
#else
#define _CHANNEL_NUMBER 4
#endif
// wartosci pomiarow
__IO uint16_t distValues[_CHANNEL_NUMBER];
// czulosc czujnikow w %
__IO uint8_t grdSensivity = 100;
// licznik na potrzeby programowego PWMa
__IO uint16_t grdSensCounter = 0;

void sensorInit() {

	// konfiguracja portow - DIST
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2
			| GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#if _DIST_EXT_ENABLE
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

	// konfiguracja DMA
	DMA_DeInit(DMA1_Channel1);
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) & distValues;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = _CHANNEL_NUMBER;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);

	// konfiguracja ADC
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = _CHANNEL_NUMBER;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, ADC_SampleTime_55Cycles5);
#if _DIST_EXT_ENABLE
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 6, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 7, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 8, ADC_SampleTime_55Cycles5);
#endif

	// uruchomienie przetwarzania
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1))
		;
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1))
		;
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	// konfiguracja portow - GRD
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

/**
 * Pobranie wartosci czujnika w mv
 * distNumber - numer czujnika
 */
int distValue(int distNumber) {
	int value = 0;
	switch (distNumber) {
	case DIST1:
		value = distValues[0];
		break;
	case DIST2:
		value = distValues[1];
		break;
	case DIST3:
		value = distValues[2];
		break;
	case DIST4:
		value = distValues[3];
		break;
#if _DIST_EXT_ENABLE
		case DIST5:
		value = distValues[4];
		break;
		case DIST6:
		value = distValues[5];
		break;
		case DIST7:
		value = distValues[6];
		break;
		case DIST8:
		value = distValues[7];
		break;
#endif
	}
	return (value * 807L) / 1000;
}

/**
 * Sprawdzenie, czy czujnik widzi biala linie
 * grdNumber - numer czujnika
 */
int grdDetected(int grdNumber) {
	switch (grdNumber) {
	case GRD1:
		return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);
	case GRD2:
		return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
	case GRD3:
		return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
	case GRD4:
		return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
	case GRD5:
		return !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_10);
	}
	return 0;
}

/**
 * Ustawienie czulosci czujnikow, w zakresie 0-100%
 * sensivity czulosc
 */
void grdSetSensivity(int sensivity) {
	if (sensivity < 0) {
		sensivity = 0;
	} else if (sensivity > 100) {
		sensivity = 100;
	}
	grdSensivity = sensivity;
}

void sensorSystickInterrupt() {
	// programowy PWM do jasnosci diod czujnikow podloza
	grdSensCounter++;
	// przelaczanie
	if (grdSensivity < 100) {
		if (grdSensCounter * 100
				== (_SYSTICK_FREQUENCY / _GRD_PWM_FREQUENCY) * grdSensivity) {
			GPIO_ResetBits(GPIOA, GPIO_Pin_8);
		}
	}
	// sprawdzanie czy nie nastapilo przepelnienie
	if (grdSensCounter >= _SYSTICK_FREQUENCY / _GRD_PWM_FREQUENCY) {
		grdSensCounter = 0;
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
}
