/**
 * Plik konfiguracyjny biblioteki
 */

#ifndef _SUMOCONF
#define _SUMOCONF 1

/**
 * Bootloader obecny (zmiana adresacji)
 * 0 - nie, 1 - tak
 */
#define _BOOTLOADER_PRESENT 1

/**
 * Ofset adresu programu przy obecnosci bootloadera
 */
#define _BOOTLOADER_OFFSET 0x3000

/**
 * Port szeregowy uzywany
 * 0 - nie, 1 - tak
 */
#define _USART_ENABLE 1

/**
 * Predkosc pracy portu szeregowego
 */
#define _USART_SPEED 115200

/**
 * Wlaczenie funkcji rprintf
 * 0 - wylaczona, 1 wlaczona
 */
#define _RPRINTF_ENABLE 1

/**
 * Wlaczenie funkcji rprintf
 * 0 - wylaczona, 1 wlaczona
 */
#define _RSCANF_ENABLE 1

/**
 * Czestotliwosc pracy PWM od silnikow [Hz]
 */
#define _MOTOR_PWM_FREQUENCY 1000

/**
 * Maksymalna moc (wypelnienie w %) pracy silnikow
 */
#define _MOTOR_MAX_POWER 100

/**
 * Kalibracja serwomechanizmu
 * Wartosc [us] dla minimalnego wychylenia serwa
 */
#define _SERVO_MIN_POSITION 1000

/**
 * Kalibracja serwomechanizmu
 * Wartosc [us] dla maksymalnego wychylenia serwa
 */
#define _SERVO_MAX_POSITION 2000

/**
 * Jesli 0 - wychylenie serwa podawane w zakresie 0 - +180 stopni
 * Jesli 1 - wychylenie serwa podawane w zakresie -90 - +90 stopni
 */
#define _SERVO_SIGNED_ANGLE 0

/**
 * Czestotliwosc pracy PWM od czujnikow podloza [Hz]
 */
#define _GRD_PWM_FREQUENCY 100

/**
 * Maksymalna ilosc timeow jednoczesnie wykorzystywanych
 */
#define _TIMER_MAX_NUMBER 16

/**
 * Uzywane wejscie DIST5-DIST8
 * 0 - nie, 1 - tak
 * UWAGA: Maksymalne napiecie nie moze przekraczac 3V3,
 * aby pracowac z napieciem 5V zastosowac dzielnik napieciowy
 */
#define _DIST_EXT_ENABLE 0

/**
 * Port USB uzywany
 * 0 - nie, 1 - tak
 */
#define _USB_ENABLE 0

/**
 * Uzywane dody led12, led13, led14
 * 0 - nie, 1 - tak
 * UWAGA: Wlaczac tylko, jesli USB nie jest podlaczane,
 * niewlasciwe uzycie grozi uszkodzeniem portu
 */
#define _LED_EXT_ENABLE 0



#endif // _SUMOCONF
