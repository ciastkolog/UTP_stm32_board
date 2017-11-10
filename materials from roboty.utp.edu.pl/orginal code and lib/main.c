#include "sumolib.h"

/**
 * Rozmieszczenie czujnikow
 * GRD1 - lewy przod
 * GRD2 - prawy przod
 * GRD3 - lewy tyl
 * GRD4 - prawy tyl
 * DIST1 - lewy przod
 * DIST2 - prawy przod
 * DIST3 - tyl, srodek
 * Rozmieszczenie napedow
 * MOTOR1 - lewy silnik
 * SERVO1 - lewe serwo
 * MOTOR2 - prawy silnik
 * SERVO2 - prawe serwo, pracujace odwrotnie
 * Znaczenie przyciskow, przelacznikow, diod
 * BUTTON1 - start
 * BUTTON2 - start
 * SWITCH1 - wybor predkosci
 * SWITCH2 - wybor strategii
 * LED1-4 - stan czujnikow GRD
 * LED5-7 - stan czujnikow DIST
 * LED8 - miga z okresem 1s
 * LED9-11 - odliczanie binarne i ruch robota
 * LED12 - aktywne USB
 */

// definicja progu widzenia czujnikow odleglosci
#define DIST_TRESHOLD 300
// poziomy mocy silnika
#define POWER_LEVEL1 50
#define POWER_LEVEL2 70
#define POWER_LEVEL3 100

// definicje nazwy czynnosci
#define STOP 0
#define FORWARD 1
#define FORWARD_LEFT 2
#define FORWARD_RIGHT 3
#define FORWARD_ATACK 4
#define BACKWARD 5
#define BACKWARD_LEFT 6
#define BACKWARD_RIGHR 7
#define BACKWARD_ATACK 8
#define ROTATE_LEFT 9
#define ROTATE_RIGHT 10

// deklaracje funkcji
void timer1();
void countdown();
void executeAction();
void selectAction();

// wartosc aktuanie wykonywanej czynnosci
volatile int currentAction = STOP;

int main(void) {
	init();
	// dawanie znaku zycia
	rprintf("Reset\r\n");
	// uruchomienie timera
	timerSchedule(timer1, 1);

	// oczekiwanie na nacisniecie przycosku i odliczanie
	countdown();

	// petla glowna programu
	for (;;) {
		// wybor akcji
		selectAction();
		// wykonywanie akcji
		executeAction();
		// opoznienie
		waitMs(20);
	}
}

// Funkcja wyboru akcji
void selectAction() {
	if (grdDetected(GRD1) && grdDetected(GRD2)) {
		// jesli linia jest z przodu - ucieczka do tylu
		currentAction = BACKWARD;
	} else if (grdDetected(GRD3) && grdDetected(GRD4)) {
		// jesli linia jest z tylu - ucieczka do przodu
		currentAction = FORWARD;
	} else if (grdDetected(GRD1)) {
		// jesli linia jest z przodu po lewej - ucieczka do tylu prawo
		currentAction = BACKWARD_RIGHR;
	} else if (grdDetected(GRD2)) {
		// jesli linia jest z przodu po prawej - ucieczka do tylu lewo
		currentAction = BACKWARD_LEFT;
	} else if (grdDetected(GRD3)) {
		// jesli linia jest z tylu po lewej - ucieczka do przodu prawo
		currentAction = FORWARD_RIGHT;
	} else if (grdDetected(GRD4)) {
		// jesli linia jest z tylu po prawej - ucieczka do przodu lewo
		currentAction = FORWARD_LEFT;
	} else if (distValue(DIST1) > DIST_TRESHOLD
			&& distValue(DIST2) > DIST_TRESHOLD) {
		// jesi przeciwnik jest z przodu na wprost - atak
		currentAction = FORWARD_ATACK;
	} else if (distValue(DIST1) > DIST_TRESHOLD) {
		// jesli przeciwnik jest z przodu po lewej - do przodu na lewo
		currentAction = FORWARD_LEFT;
	} else if (distValue(DIST2) > DIST_TRESHOLD) {
		// jesli przeciwnik jest z przodu po prawej - do przodu na prawo
		currentAction = FORWARD_RIGHT;
	} else if (distValue(DIST3) > DIST_TRESHOLD) {
		// jesli przeciwnik jest z tylu - obrot
		currentAction = ROTATE_LEFT;
	} else {
		// nie ma ani lini, ani przeciwnika
		if (currentAction == STOP) {
			// jesli robot stoi, to podejmuje akcje
			if (switchIsOn(SWITCH2)) {
				currentAction = FORWARD_LEFT;
			} else {
				currentAction = FORWARD_RIGHT;
			}
		}
	}
}

// Funkcja wykonuje akcje
void executeAction() {
	if (currentAction == STOP) {
		motorStop(MOTOR1);
		motorStop(MOTOR2);
		servoStop(SERVO1);
		servoStop(SERVO2);
	} else if (currentAction == FORWARD) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorForward(MOTOR1);
		motorForward(MOTOR2);
		servoForward(SERVO1);
		servoBackward(SERVO2);
	} else if (currentAction == FORWARD_LEFT) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2 / 3);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1 / 3);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorForward(MOTOR1);
		motorForward(MOTOR2);
		servoStop(SERVO1);
		servoBackward(SERVO2);
	} else if (currentAction == FORWARD_RIGHT) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2 / 3);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1 / 3);
		}
		motorForward(MOTOR1);
		motorForward(MOTOR2);
		servoForward(SERVO1);
		servoStop(SERVO2);
	} else if (currentAction == FORWARD_ATACK) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL3);
			motorSetPower(MOTOR2, POWER_LEVEL3);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		}
		motorForward(MOTOR1);
		motorForward(MOTOR2);
		servoForward(SERVO1);
		servoBackward(SERVO2);
	} else if (currentAction == BACKWARD) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorBackward(MOTOR1);
		motorBackward(MOTOR2);
		servoBackward(SERVO1);
		servoForward(SERVO2);
	} else if (currentAction == BACKWARD_LEFT) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2 / 3);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1 / 3);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorBackward(MOTOR1);
		motorBackward(MOTOR2);
		servoStop(SERVO1);
		servoForward(SERVO2);
	} else if (currentAction == BACKWARD_RIGHR) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2 / 3);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1 / 3);
		}
		motorBackward(MOTOR1);
		motorBackward(MOTOR2);
		servoBackward(SERVO1);
		servoStop(SERVO2);
	} else if (currentAction == BACKWARD_ATACK) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL3);
			motorSetPower(MOTOR2, POWER_LEVEL3);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		}
		motorBackward(MOTOR1);
		motorBackward(MOTOR2);
		servoBackward(SERVO1);
		servoForward(SERVO2);
	} else if (currentAction == ROTATE_LEFT) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorBackward(MOTOR1);
		motorForward(MOTOR2);
		servoBackward(SERVO1);
		servoBackward(SERVO2);
	} else if (currentAction == ROTATE_RIGHT) {
		if (switchIsOn(SWITCH1)) {
			motorSetPower(MOTOR1, POWER_LEVEL2);
			motorSetPower(MOTOR2, POWER_LEVEL2);
		} else {
			motorSetPower(MOTOR1, POWER_LEVEL1);
			motorSetPower(MOTOR2, POWER_LEVEL1);
		}
		motorForward(MOTOR1);
		motorBackward(MOTOR2);
		servoForward(SERVO1);
		servoForward(SERVO2);
	}
}

// funkcja wywoluwana co 1 ms, realizuje operacje diagnostyczne
void timer1() {
	// miganie diada co 0,5 s
	ledSet(LED8, (timeInMilis() / 500) % 2);
	// wyswietlenie na diodach stanow czujnikow
	ledSet(LED1, grdDetected(GRD1));
	ledSet(LED2, grdDetected(GRD2));
	ledSet(LED3, grdDetected(GRD3));
	ledSet(LED4, grdDetected(GRD4));
	ledSet(LED5, distValue(DIST1) > DIST_TRESHOLD);
	ledSet(LED6, distValue(DIST2) > DIST_TRESHOLD);
	ledSet(LED7, distValue(DIST3) > DIST_TRESHOLD);
	// mruganie w zaleznsic od ruchu
	if (currentAction == FORWARD || currentAction == FORWARD_ATACK) {
		ledOn(LED9);
		ledOff(LED10);
		ledOn(LED11);
	} else if (currentAction == FORWARD_LEFT || currentAction == BACKWARD_LEFT
			|| currentAction == ROTATE_LEFT) {
		ledOn(LED9);
		ledOff(LED10);
		ledOff(LED11);
	} else if (currentAction == FORWARD_RIGHT || currentAction == BACKWARD_RIGHR
			|| currentAction == ROTATE_RIGHT) {
		ledOff(LED9);
		ledOff(LED10);
		ledOn(LED11);
	} else if (currentAction == BACKWARD || currentAction == BACKWARD_ATACK) {
		ledOff(LED9);
		ledOn(LED10);
		ledOff(LED11);
	}
	// wysylanie pomiarow na uart co 2s
	if ((timeInMilis() % 2000) == 0) {
		// wyswietlanie stanow czujnikow
		rprintf("G1=%d G2=%d G3=%d G4=%d D1=%d D2=%d D3=%d D4=%d ",
				grdDetected(GRD1), grdDetected(GRD2), grdDetected(GRD3),
				grdDetected(GRD4), distValue(DIST1), distValue(DIST2),
				distValue(DIST3), distValue(DIST4));
		// wyswietlanie aktualnej akcji
		if (currentAction == STOP) {
			rprintf("STOP\r\n");
		} else if (currentAction == FORWARD) {
			rprintf("FORWARD\r\n");
		} else if (currentAction == FORWARD_LEFT) {
			rprintf("FORWARD_LEFT\r\n");
		} else if (currentAction == FORWARD_RIGHT) {
			rprintf("FORWARD_RIGHT\r\n");
		} else if (currentAction == FORWARD_ATACK) {
			rprintf("FORWARD_ATACK\r\n");
		} else if (currentAction == BACKWARD) {
			rprintf("BACKWARD\r\n");
		} else if (currentAction == BACKWARD_LEFT) {
			rprintf("BACKWARD_LEFT\r\n");
		} else if (currentAction == BACKWARD_RIGHR) {
			rprintf("BACKWARD_RIGHR\r\n");
		} else if (currentAction == BACKWARD_ATACK) {
			rprintf("BACKWARD_ATACK\r\n");
		} else if (currentAction == ROTATE_LEFT) {
			rprintf("ROTATE_LEFT\r\n");
		} else if (currentAction == ROTATE_RIGHT) {
			rprintf("ROTATE_RIGHT\r\n");
		}
	}
}

// funkcja oczekuje na nacisniecie przycisku i
void countdown() {
	// oczekiwanie na nacisniecie przycisku
	while (!buttonPressed(BUTTON1) && !buttonPressed(BUTTON2))
		;
	// odliczanie
	rprintf("5... \r\n");
	ledOn(LED9);
	ledOff(LED10);
	ledOn(LED11);
	wait(1);
	rprintf("4... \r\n");
	ledOff(LED11);
	wait(1);
	rprintf("3... \r\n");
	ledOff(LED9);
	ledOn(LED10);
	ledOn(LED11);
	wait(1);
	rprintf("2... \r\n");
	ledOff(LED11);
	wait(1);
	rprintf("1... \r\n");
	ledOff(LED10);
	ledOn(LED11);
	wait(1);
	rprintf("START\r\n");
	ledOn(LED9);
	ledOn(LED10);
}
