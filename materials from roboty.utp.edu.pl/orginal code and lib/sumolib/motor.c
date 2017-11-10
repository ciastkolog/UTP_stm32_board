/**
 * Funkcje obslugi napedow
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "internal.h"

// okresla czy pwm ma byc podawany na gorna galaz mostka
#define _PWM_ON_UPPER 1
// prescaller
#define _PWM_PRESCALLER 8
// okres pracy
#define _PWM_PERIOD (SystemCoreClock / _PWM_PRESCALLER / _MOTOR_PWM_FREQUENCY)
// czas martwy pomiedzy operacjami [ms]
#define _PWM_DEAD_TIME 5
// domyslna moc
#define _DEFAULT_POWER (_MOTOR_MAX_POWER / 2)
// operacje wykonywane przez silnik
#define _STOP 0
#define _FORWARD 1
#define _BACKWARD 2
#define _BREAK 3

/**
 * Piny:
 * PA0 - MOT1_R1 - TIM2_CH1
 * PA1 - MOT1_L1 - TIM2_CH2
 * PA2 - MOT1_R2 - TIM2_CH3
 * PA3 - MOT1_L2 - TIM2_CH4
 * PC6 - MOT2_R2 - TIM3_CH1 Remap
 * PC7 - MOT2_L2 - TIM3_CH2 Remap
 * PC8 - MOT2_R1 - TIM3_CH3 Remap
 * PC9 - MOT2_L1 - TIM3_CH4 Remap
 */

// funkcje lokalne
void doStop(int motorNumber);
void doForward(int motorNumber);
void doBackward(int motorNumber);
void doBreak(int motorNumber);
void doSetPower(int motorNumber, int power);
void doSetPowerL(int motorNumber, int power);
void doSetPowerR(int motorNumber, int power);
// zmienne
__IO uint8_t currentOperation[2] = { _STOP, _STOP };
__IO uint8_t requiredOperation[2] = { _STOP, _STOP };
__IO uint8_t requestDowncounter[2] = { 0, 0 };
__IO uint16_t currentPower[2] = { 0, 0 };
__IO uint16_t requiredPower[2] = { 0, 0 };
extern uint32_t tickCounter;

/**
 * Inicjalizacja silnikow
 */
void motorInit() {
	// konfiguracja portow
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
#if _PWM_ON_UPPER
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#else
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

	// konfiguracja timera
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = _PWM_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = _PWM_PRESCALLER - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//	TIM_OCInitStructure.TIM_Pulse = _PWM_PERIOD / 2;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
#if _PWM_ON_UPPER
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
#else
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);
//	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
#endif

	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	doStop(MOTOR1);
	doStop(MOTOR2);
	motorSetPower(MOTOR1, _DEFAULT_POWER);
	motorSetPower(MOTOR2, _DEFAULT_POWER);
}

/**
 * Ruch silnika do przodu
 * motorNumber - numer silnika
 */
void motorForward(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		if (requiredOperation[0] != _FORWARD) {
			doStop(MOTOR1);
			requestDowncounter[0] = _PWM_DEAD_TIME + 1;
			requiredOperation[0] = _FORWARD;
		}
		break;
	case MOTOR2:
		if (requiredOperation[1] != _FORWARD) {
			doStop(MOTOR2);
			requestDowncounter[1] = _PWM_DEAD_TIME + 1;
			requiredOperation[1] = _FORWARD;
		}
		break;
	}
}

/**
 * Ruch silnika do tylu
 * motorNumber - numer silnika
 */
void motorBackward(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		if (requiredOperation[0] != _BACKWARD) {
			doStop(MOTOR1);
			requestDowncounter[0] = _PWM_DEAD_TIME + 1;
			requiredOperation[0] = _BACKWARD;
		}
		break;
	case MOTOR2:
		if (requiredOperation[1] != _BACKWARD) {
			doStop(MOTOR2);
			requestDowncounter[1] = _PWM_DEAD_TIME + 1;
			requiredOperation[1] = _BACKWARD;
		}
		break;
	}
}

/**
 * Zatrzymanie silnika
 * motorNumber - numer silnika
 */
void motorStop(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		doStop(MOTOR1);
		requiredOperation[0] = _STOP;
		break;
	case MOTOR2:
		doStop(MOTOR2);
		requiredOperation[1] = _STOP;
		break;
	}
}

/**
 * Hamowanie silnikiem
 * motorNumber - numer silnika
 */
void motorBreak(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		if (requiredOperation[0] != _BREAK) {
			doStop(MOTOR1);
			requestDowncounter[0] = _PWM_DEAD_TIME;
			requiredOperation[0] = _BREAK;
		}
		break;
	case MOTOR2:
		if (requiredOperation[1] != _BREAK) {
			doStop(MOTOR2);
			requestDowncounter[1] = _PWM_DEAD_TIME;
			requiredOperation[1] = _BREAK;
		}
		break;
	}
}

/**
 * Hamowanie silnikiem
 * motorNumber - numer silnika
 */
void motorSetPower(int motorNumber, int power) {
	// zabezpieczenie
	if (power < 0) {
		power = 0;
	}
	if (power > _MOTOR_MAX_POWER) {
		power = _MOTOR_MAX_POWER;
	}
	switch (motorNumber) {
	case MOTOR1:
		requiredPower[0] = (power * _PWM_PERIOD) / 100;
		break;
	case MOTOR2:
		requiredPower[1] = (power * _PWM_PERIOD) / 100;
		break;
	}
}

void motorSystickInterrupt() {
	// co 1 ms
	if (tickCounter % (_SYSTICK_FREQUENCY / 1000) == 0) {
		int i;
		for (i = 0; i < 2; i++) {
			if (currentOperation[i] != requiredOperation[i]) {
				requestDowncounter[i]--;
				if (requestDowncounter[i] == 0) {
					switch (requiredOperation[i]) {
					case _STOP:
						doStop(MOTOR1 + i);
						break;
					case _FORWARD:
						doForward(MOTOR1 + i);
						break;
					case _BACKWARD:
						doBackward(MOTOR1 + i);
						break;
					case _BREAK:
						doBreak(MOTOR1 + i);
						break;
					}
				}
			}
			if ((currentPower[i] != requiredPower[i])
					&& (currentOperation[i] == _FORWARD
							|| currentOperation[i] == _BACKWARD)) {
				doSetPower(MOTOR1 + i, requiredPower[i]);
			}
		}
	}
}

void doStop(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		currentOperation[0] = _STOP;
		currentPower[0] = 0;
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3);
		TIM_SetCompare1(TIM2, 0);
		TIM_SetCompare2(TIM2, 0);
#else
		GPIO_SetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
		TIM_SetCompare3(TIM2, 0);
		TIM_SetCompare4(TIM2, 0);
#endif
		break;
	case MOTOR2:
		currentOperation[1] = _STOP;
		currentPower[1] = 0;
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7);
		TIM_SetCompare3(TIM3, 0);
		TIM_SetCompare4(TIM3, 0);
#else
		GPIO_SetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);
		TIM_SetCompare1(TIM3, 0);
		TIM_SetCompare2(TIM3, 0);
#endif
		break;
	}
}

void doForward(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		currentOperation[0] = _FORWARD;
		currentPower[0] = requiredPower[0];
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOA, GPIO_Pin_3);
		TIM_SetCompare1(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
		TIM_SetCompare2(TIM2, currentPower[0]);
#else
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		TIM_SetCompare4(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		TIM_SetCompare3(TIM2, currentPower[0]);
#endif
		break;
	case MOTOR2:
		currentOperation[1] = _FORWARD;
		currentPower[1] = requiredPower[1];
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOC, GPIO_Pin_7);
		TIM_SetCompare3(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_6);
		TIM_SetCompare4(TIM3, currentPower[1]);
#else
		GPIO_SetBits(GPIOC, GPIO_Pin_8);
		TIM_SetCompare2(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
		TIM_SetCompare1(TIM3, currentPower[1]);
#endif
		break;
	}
}

void doBackward(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		currentOperation[0] = _BACKWARD;
		currentPower[0] = requiredPower[0];
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
		TIM_SetCompare2(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_3);
		TIM_SetCompare1(TIM2, currentPower[0]);
#else
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
		TIM_SetCompare3(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		TIM_SetCompare4(TIM2, currentPower[0]);
#endif
		break;
	case MOTOR2:
		currentOperation[1] = _BACKWARD;
		currentPower[1] = requiredPower[1];
#if _PWM_ON_UPPER
		GPIO_SetBits(GPIOC, GPIO_Pin_6);
		TIM_SetCompare4(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_7);
		TIM_SetCompare3(TIM3, currentPower[1]);
#else
		GPIO_SetBits(GPIOC, GPIO_Pin_9);
		TIM_SetCompare1(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8);
		TIM_SetCompare2(TIM3, currentPower[1]);
#endif
		break;
	}
}

void doBreak(int motorNumber) {
	switch (motorNumber) {
	case MOTOR1:
		currentOperation[0] = _BREAK;
		currentPower[0] = 0;
#if _PWM_ON_UPPER
		TIM_SetCompare1(TIM2, 0);
		TIM_SetCompare2(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_2 | GPIO_Pin_3);
#else
		TIM_SetCompare3(TIM2, 0);
		TIM_SetCompare4(TIM2, 0);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1);
#endif
		break;
	case MOTOR2:
		currentOperation[1] = _BREAK;
		currentPower[1] = requiredPower[1];
#if _PWM_ON_UPPER
		TIM_SetCompare3(TIM3, 0);
		TIM_SetCompare4(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_6 | GPIO_Pin_7);
#else
		TIM_SetCompare1(TIM3, 0);
		TIM_SetCompare2(TIM3, 0);
		GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);
#endif
		break;
	}
}

void doSetPower(int motorNumber, int power) {
	switch (motorNumber) {
	case MOTOR1:
		if (currentOperation[0] == _FORWARD) {
			currentPower[0] = requiredPower[0];
#if _PWM_ON_UPPER
			TIM_SetCompare2(TIM2, currentPower[0]);
#else
			TIM_SetCompare3(TIM2, currentPower[0]);
#endif
		} else if (currentOperation[0] == _BACKWARD) {
			currentPower[0] = requiredPower[0];
#if _PWM_ON_UPPER
			TIM_SetCompare1(TIM2, currentPower[0]);
#else
			TIM_SetCompare4(TIM2, currentPower[0]);
#endif
		}
		break;
	case MOTOR2:
		if (currentOperation[1] == _FORWARD) {
			currentPower[1] = requiredPower[1];
#if _PWM_ON_UPPER
			TIM_SetCompare4(TIM2, currentPower[1]);
#else
			TIM_SetCompare1(TIM2, currentPower[1]);
#endif
		} else if (currentOperation[0] == _FORWARD) {
			currentPower[1] = requiredPower[1];
#if _PWM_ON_UPPER
			TIM_SetCompare3(TIM2, currentPower[1]);
#else
			TIM_SetCompare2(TIM2, currentPower[1]);
#endif
		}
		break;
	}
}
