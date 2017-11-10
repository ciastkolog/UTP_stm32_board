/**
 * Funkcje obslugi serwomechanizmow
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "internal.h"

// czestotliwosc pracy pwm
#define _PWM_FREQUENCY 50
// preskaller PWMa
#define _PWM_PRESCALLER 32
// okres pracy
#define _PWM_PERIOD (SystemCoreClock / _PWM_PRESCALLER / _PWM_FREQUENCY)

/**
 * Piny:
 * PB9 - SERVO1 - TIM4_CH4
 * PB8 - SERVO2 - TIM4_CH3
 */

/**
 * Inicjalizacja serwomechanizmow
 */
void servoInit() {
	// konfiguracja portow
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// konfiguracja timera
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = _PWM_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = _PWM_PRESCALLER - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_Cmd(TIM4, ENABLE);
}

/**
 * Ustawia zadany kat wychylenia serwa
 * servoNumber - numer serwa
 * angle - kat o - 180 stopni
 */
void servoSetAngle(int servoNumber, int angle) {
#if _SERVO_SIGNED_ANGLE
	angle += 90;
#endif
	int timeInUs = ((_SERVO_MAX_POSITION - _SERVO_MIN_POSITION) * angle) / 180 + _SERVO_MIN_POSITION;
	switch (servoNumber) {
	case SERVO1:
		TIM_SetCompare4(TIM4, (_PWM_PERIOD * timeInUs) / (1000000 / _PWM_FREQUENCY));
		break;
	case SERVO2:
		TIM_SetCompare3(TIM4, (_PWM_PERIOD * timeInUs) / (1000000 / _PWM_FREQUENCY));
		break;
	}
}

/**
 * Ruch do przodu
 * Uzywac tylko jesli jako naped jest zastosowany przerobiony serwomechanizm
 */
void servoForward(int servoNumber) {
#if _SERVO_SIGNED_ANGLE
	servoSetAngle(servoNumber, -90);
#else
	servoSetAngle(servoNumber, 0);
#endif
}

/**
 * Ruch do tylu
 * Uzywac tylko jesli jako naped jest zastosowany przerobiony serwomechanizm
 */
void servoBackward(int servoNumber) {
#if _SERVO_SIGNED_ANGLE
	servoSetAngle(servoNumber, 90);
#else
	servoSetAngle(servoNumber, 180);
#endif
}

/**
 * Zatrzymanie serwomechanizmu
 */
void servoStop(int servoNumber) {
	switch (servoNumber) {
	case SERVO1:
		TIM_SetCompare4(TIM4, 0);
		break;
	case SERVO2:
		TIM_SetCompare3(TIM4, 0);
		break;
	}
}
