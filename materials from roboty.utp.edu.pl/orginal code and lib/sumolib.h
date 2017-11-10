/**
 * Plik naglowkowy biblioteki
 */

#ifndef _SUMOLIB
#define _SUMOLIB 1 // wersja biblioteki

#include "sumoconf.h"

// deklaracje funkcji

// -------------------- Util --------------------

/**
 * Inicjalizacja procesora
 */
void init();

/**
 * Oczekiwanie okreslonej ilosci milisekund
 */
void waitMs(int ms);

/**
 * Oczekiwanie okreslonej ilosci sekund
 */
void wait(int s);

/**
 * Wartosc pseudolosowa
 * range - zakres wartosci
 */
int randomValue(int range);

/**
 * Ilosc milisekund od startu programu
 */
int timeInMilis();

// ------------------------- Switch & Led -------------------------

// definicja przyciskow i przelacznikow
#define BUTTON1 1
#define BUTTON2 2
#define SWITCH1 1
#define SWITCH2 2
#define SWITCH3 3

// definicje diod
#define LED1 1
#define LED2 2
#define LED3 3
#define LED4 4
#define LED5 5
#define LED6 6
#define LED7 7
#define LED8 8
#define LED9 9
#define LED10 10
#define LED11 11
#if !_USB_ENABLE && _LED_EXT_ENABLE
#define LED12 12
#define LED13 13
#define LED14 14
#endif
#define LED_ON 1
#define LED_OFF 0

/**
 * Sprawdzenie czy przycisk jest nacisniety
 * buttonNumber - numer przycisku
 * zwraca 1 jesli tak, 0 jesli nie
 */
int buttonPressed(int buttonNumber);

/**
 * Oczekiwanie na nacisniecie przycisku
 * buttonNumber - numer przycisku
 */
void waitForButton(int buttonNumber);

/**
 * Sprzwdzenie, czy przelacznik jest wlaczony
 * switchNumber - numer przelacznika
 * zwraca 1 jesli tak, 0 jesli nie
 */
int switchIsOn(int switchNumber);

/**
 * Zapalenie okreslonej diody
 * ledNumber - numer diody
 */
void ledOn(int ledNumber);

/**
 * Gaszenie okreslonej diody
 * ledNumber - numer diody
 */
void ledOff(int ledNumber);

/**
 * Ustawienie stanu okreslonej diody
 * ledNumber - numer diody
 * ledState - stan diody 0 - zgaszona, 1 - zapalona
 */
void ledSet(int ledNumber, int ledState);

// ------------------------- Sensor -------------------------

// definicje czujnikow
#define DIST1 1
#define DIST2 2
#define DIST3 3
#define DIST4 4
#if _DIST_EXT_ENABLE
#define DIST5 5
#define DIST6 6
#define DIST7 7
#define DIST8 8
#endif
#define GRD1 1
#define GRD2 2
#define GRD3 3
#define GRD4 4
#define GRD5 5

/**
 * Pobranie wartosci czujnika w mv
 * distNumber - numer czujnika
 */
int distValue(int distNumber);

/**
 * Sprawdzenie, czy czujnik widzi biala linie
 * grdNumber - numer czujnika
 */
int grdDetected(int grdNumber);

/**
 * Ustawienie czulosci czujnikow, w zakresie 0-100%
 * sensivity czulosc
 */
void grdSetSensivity(int sensivity);

// ------------------------- Motor -------------------------

// definicje silnikow
#define MOTOR1 1
#define MOTOR2 2

/**
 * Ruch silnika do przodu
 * motorNumber - numer silnika
 */
void motorForward(int motorNumber);

/**
 * Ruch silnika do tylu
 * motorNumber - numer silnika
 */
void motorBackward(int motorNumber);

/**
 * Zatrzymanie silnika
 * motorNumber - numer silnika
 */
void motorStop(int motorNumber);

/**
 * Hamowanie silnikiem
 * motorNumber - numer silnika
 */
void motorBreak(int motorNumber);

/**
 * Ustawia moc (wypelnienie) silnika
 * motorNumber - numer silnika
 * power - moc w % (0-100)
 */
void motorSetPower(int motorNumber, int power);

// ------------------------- Servo -------------------------

// definicje serwomechanizmow
#define SERVO1 1
#define SERVO2 2

/**
 * Ustawia zadany kat wychylenia serwa
 * servoNumber - numer serwa
 * angle - kat o - 180 stopni
 */
void servoSetAngle(int servoNumber, int angle);

/**
 * Ruch do przodu
 * Uzywac tylko jesli jako naped jest zastosowany przerobiony serwomechanizm
 */
void servoForward(int servoNumber);

/**
 * Ruch do tylu
 * Uzywac tylko jesli jako naped jest zastosowany przerobiony serwomechanizm
 */
void servoBackward(int servoNumber);

/**
 * Zatrzymanie serwomechanizmu
 */
void servoStop(int servoNumber);

// -------------------- Util --------------------

/**
 * Planuje jednorazowe wywolanie funkcji
 * timerFunction - funkcja do wywolania
 * delay - czas za jaki funkcja ma byc wywolana
 */
void timerScheduleOnce(void (*timerFunction)(void), int delay);

/**
 * Planuje cykliczne wywolywanie funkcji
 * timerFunction - funkcja do wywolania
 * period - czas co jaki funkcja ma byc wywolana
 */
void timerSchedule(void (*timerFunction)(void), int period);

/**
 * Anulowanie wywolywania funkcji
 */
void timerCancel(void (*timerFunction)(void));


// -------------------- USART --------------------
#if _USART_ENABLE

/**
 * Wysyla znak na port szeregwy
 */
void usartSendChar(char c);

/**
 * Wysyla zawartosc bufora na port szeregowy
 * buff - dlugosc bufora
 * len - ilosc bajtw do wyslania
 */
void usartSendRaw(char* buff, int len);

/**
 * Wysyla tekst na port szeregowy
 */
void usartWrite(char* str);

/**
 * Zwraca 1 jesli sa dane odebrane z portu szeregowego
 */
int usartHasData();

/**
 * Zwraca pojedynczy znak odebrany z portu szeregowego
 */
char usartGetChar();

/**
 * zapisuje w buforze linijke tekstu odebrana z portu seregowego
 * buf - bufor do ktorego beda zapisane dane
 * zwraca ilosc danych
 */
int usartReadLine(char *buff);

#endif

// -------------------- rprintf --------------------

#if _RPRINTF_ENABLE
/**
 * Odpowiednik funkcji printf z C
 */
int rprintf(const char *format, ...);

/**
 * Odpowiednik funkcji sprintf z C
 */
int rsprintf(char *buffer, const char *format, ...);
#endif

#if _RSCANF_ENABLE
/**
 * Odpowiednik funkcji scanf z C
 */
int rscanf(const char* format, ...);
#endif

#endif // _SUMOLIB
