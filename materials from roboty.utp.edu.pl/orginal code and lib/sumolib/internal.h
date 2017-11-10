/**
 * Plik naglowkowy wewnetrzych elementow biblioteki
 */

#ifndef _INTERNAL
#define _INTERNAL 1

#include "sumolib.h"
#include "sumoconf.h"


// czestotliwosc systicka w Hz (wielokrotnosc 1000Hz)
#define _SYSTICK_FREQUENCY 3000

// funkcje inicjalizujace
void switchLedInit();
void sensorInit();
void motorInit();
void servoInit();
void timerInit();

#if _USART_ENABLE
void usartInit();
#endif

// funkcje wywolywane w systicku
void motorSystickInterrupt();
void sensorSystickInterrupt();
void timerSystickInterrupt();


#endif // _INTERNAL
