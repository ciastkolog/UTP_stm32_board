/**
 * Funkcje do obslugi timera
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"
#include "internal.h"

// prescaller
#define _TIM_PRESCALLER 8
// okres pracy
#define _TIM_PERIOD (SystemCoreClock / _TIM_PRESCALLER / 1000)

// struktura konfiguracji timera
typedef struct {
	// funkcja do wywolania
	void (*timerFunction)(void);
	// czas do wywolania [ms]
	__IO uint32_t timeToCall;
	// okres w ms cyklicznego wywolywania (0 - jednorazowe)
	__IO uint32_t reloadPeriod;
} CallbackTimer;

// tablicz z konfiguracja timerow
CallbackTimer timers[_TIMER_MAX_NUMBER];
// czas
extern uint32_t tickCounter;
__IO int tmp = 0;

//#pragma GCC push_options
//#pragma GCC optimize ("0")
// inicjalizacja
void timerInit() {

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_SetPriority(TIM1_UP_IRQn, 3);

	// konfiguracja timera
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = _TIM_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = _TIM_PRESCALLER - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM1, ENABLE);

}

void TIM1_UP_IRQHandler() {
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
		__enable_irq();
//		ledSet(LED3, tmp % 2 == 0);
		tmp++;
		int i;
		// przegladanie timerow
		for (i = 0; i < _TIMER_MAX_NUMBER; i++) {
			// jesli funkcja istnieje i nalezy juz ja wywolac
			if (timers[i].timerFunction != 0 && timers[i].timeToCall == 0) {
				// wywolywanie
				timers[i].timerFunction();
				// jesli jest okresowa
				if (timers[i].reloadPeriod > 0) {
					// planowanie kolejnego wywolania
					timers[i].timeToCall = timers[i].reloadPeriod;
				} else {
					// usuwanie
					timers[i].timerFunction = 0;
					timers[i].timeToCall = 0;
					timers[i].reloadPeriod = 0;
				}
			}
		}
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
//#pragma GCC pop_options

// wywolywane w systicku, odpowiedzialne za liczenie czasu
void timerSystickInterrupt() {
	int i;
	// dekrementacja czasu oczekiwania
	for (i = 0; i < _TIMER_MAX_NUMBER; i++) {
		// dekrementacja czasu oczekiwania
		if (timers[i].timerFunction != 0 && timers[i].timeToCall > 0) {
			timers[i].timeToCall--;
		}
	}
}

/**
 * Planuje jednorazowe wywolanie funkcji
 * timerFunction - funkcja do wywolania
 * delay - czas za jaki funkcja ma byc wywolana
 */
void timerScheduleOnce(void (*timerFunction)(void), int delay) {
	timerCancel(timerFunction);
	int i;
	// poszukiwanie wolnego miejsca i wstawianie funkcji
	for (i = 0; i < _TIMER_MAX_NUMBER; i++) {
		if (timers[i].timerFunction == 0) {
			timers[i].timerFunction = timerFunction;
			timers[i].timeToCall = delay * _SYSTICK_FREQUENCY / 1000;
			timers[i].reloadPeriod = 0;
			return;
		}
	}
}

/**
 * Planuje cykliczne wywolywanie funkcji
 * timerFunction - funkcja do wywolania
 * period - czas co jaki funkcja ma byc wywolana
 */
void timerSchedule(void (*timerFunction)(void), int period) {
	timerCancel(timerFunction);
	int i;
	// poszukiwanie wolnego miejsca i wstawianie funkcji
	for (i = 0; i < _TIMER_MAX_NUMBER; i++) {
		if (timers[i].timerFunction == 0) {
			timers[i].timerFunction = timerFunction;
			timers[i].timeToCall = period * _SYSTICK_FREQUENCY / 1000;
			timers[i].reloadPeriod = period * _SYSTICK_FREQUENCY / 1000;
			return;
		}
	}
}

/**
 * Anulowanie wywolywania funkcji
 */
void timerCancel(void (*timerFunction)(void)) {
	int i;
	for (i = 0; i < _TIMER_MAX_NUMBER; i++) {
		if (timers[i].timerFunction == timerFunction) {
			timers[i].timerFunction = 0;
			timers[i].timeToCall = 0;
			timers[i].reloadPeriod = 0;
		}
	}
}
