/** \file printf.c
 * Simplified printf() and sprintf() implementation - prints formatted string to
 * USART (or whereever). Most common % specifiers are supported. It costs you about
 * 3k FLASH memory - without floating point support it uses only 1k ROM!
 * \author Freddie Chopin, Martin Thomas, Marten Petschke and many others
 * \date 16.2.2012
 * reduced scanf() added by Andrzej Dworzynski on the base of reduced sscanf() written by
 * some nice gay from this post: http://www.fpgarelated.com/usenet/fpga/show/61760-1.php
 * thanks to everybody:)
 * \date 12.2.2013
 */

#include <stdarg.h>     // (...) parameter handling
#include <stdlib.h>     //NULL pointer definition
#include "stm32f10x.h"	// only this headerfile is used
#include "internal.h"

#define _SCANF_BUFFER_SIZE 12 //chang this if needed
//
#if _RPRINTF_ENABLE
static void putc_strg(char); // the put() function for sprintf()
static int rvfprintf(void (*)(char), const char *format, va_list arg); //generic print
static void long_itoa(long, int, int, void (*)(char)); //heavily used by printf

char *SPRINTF_buffer;
#endif
#if _RSCANF_ENABLE
static int rsscanf(const char* str, const char* format, va_list ap); //used by rscanf
#endif

#if _RPRINTF_ENABLE

int rprintf(const char *format, ...) {
	va_list arg;

	va_start(arg, format);
#if _USART_ENABLE
	//rvfprintf((&usartSendChar), format, arg);
	// TODO zrobic prosciej, nie na okolo
	static char buff[128];
	char *ptr = buff;
	SPRINTF_buffer = buff;
	rvfprintf((&putc_strg), format, arg);
	int len = SPRINTF_buffer - ptr;
	usartSendRaw(buff, len);
	*SPRINTF_buffer = '\0';
#endif
	va_end(arg);
	return 0;
}

int rsprintf(char *buffer, const char *format, ...) {
	va_list arg;
	SPRINTF_buffer = buffer;

	va_start(arg, format);
	rvfprintf((&putc_strg), format, arg);
	va_end(arg);
	*SPRINTF_buffer = '\0'; // append end of string
	return 0;
}

/**
 * @def debug(format...)
 * @brief prints the timestamp, file name, line number, printf-formated @a format string and the
 * optional parameters to stdout
 *
 * The output looks like this:<br>
 * <pre>
 * 12345     filename.c[123]: format string
 * ^    ^    ^          ^
 * |    |    |          line number
 * |    |    +--------- file name
 * |    +-------------- tab character
 * +------------------- timestamp (ms since reset)
 * </pre>
 *
 * */
// putc_strg() is the putc()function for sprintf_()
static void putc_strg(char character) {
	*SPRINTF_buffer = (char) character; // just add the character to buffer
	SPRINTF_buffer++;
}

/*--------------------------------------------------------------------------------+
 * rvfprintf()
 * Prints a string to stream. Supports %s, %c, %d, %ld %ul %02d %i %x  %lud  and %%
 *     - partly supported: long long, float (%l %f, %F, %2.2f)
 *     - not supported: double float and exponent (%e %g %p %o \t)
 *--------------------------------------------------------------------------------+
 */
static int rvfprintf(void (*putc)(char), const char* str, va_list arp) {
	int d, r, w, s, l; //d=char, r = radix, w = width, s=zeros, l=long
	char *c; // for the while loop only

#ifdef INCLUDE_FLOAT
	float f;
	long int m, w2;
#endif

	while ((d = *str++) != 0) {
		if (d != '%') { //if it is not format qualifier
			(*putc)(d);
			continue; //get out of while loop
		}
		d = *str++; //if it is '%'get next char
		w = r = s = l = 0;
		if (d == '%') { //if it is % print silmpy %
			(*putc)(d);
			d = *str++;
		}
		if (d == '0') {
			d = *str++;
			s = 1; //padd with zeros
		}
		while ((d >= '0') && (d <= '9')) {
			w += w * 10 + (d - '0');
			d = *str++;
		}
		if (s) {
			w = -w; //padd with zeros if negative
		}
#ifdef INCLUDE_FLOAT
		w2 = 0;
		if (d == '.')
		d = *str++;
		while ((d >= '0')&&(d <= '9')) {
			w2 += w2 * 10 + (d - '0');
			d = *str++;
		}
#endif
		if (d == 's') { // if string
			c = va_arg(arp, char*); //get buffer addres
			while (*c) {
				(*putc)(*(c++)); //write the buffer out
			}
			continue;
		}
		if (d == 'c') {
			(*putc)((char) va_arg(arp, int));
			continue;
		}
		if (d == 'u') { // %ul
			r = 10;
			d = *str++;
		}
		if (d == 'l') { // long =32bit
			l = 1;
			if (r == 0)
				r = -10;
			d = *str++;
		}
		if (d == 'u')
			r = 10; //     %lu,    %llu
		else if (d == 'd' || d == 'i') {
			if (r == 0)
				r = -10;
		} //can be 16 or 32bit int
		else if (d == 'X' || d == 'x') {
			r = 16; // 'x' added by mthomas
		} else if (d == 'b') {
			r = 2;
		} else {
			str--; // normal character
		}
#ifdef INCLUDE_FLOAT
		if (d == 'f' || d == 'F') {
			f=va_arg(arp, double);
			if (f>0) {
				r=10;
				m=(int)f;
			}
			else {
				r=-10;
				f=-f;
				m=(int)(f);
			}
			long_itoa(m, r, w, (putc));
			f=f-m; m=f*(10^w2); w2=-w2;
			long_itoa(m, r, w2, (putc));
			l=3; //do not continue with long
		}
#endif
		if (!r) {
			continue; //
		}
		if (l == 0) {
			if (r > 0) { //unsigned{
				long_itoa((unsigned long) va_arg(arp, int), r, w, (putc)); //needed for 16bit int, no harm to 32bit int
			} else {
				//signed
				long_itoa((long) va_arg(arp, int), r, w, (putc));
			}
		} else if (l == 1) { // long =32bit
			long_itoa((long) va_arg(arp, long), r, w, (putc)); //no matter if signed or unsigned
		}
	}
	return 0;
}

static void long_itoa(long val, int radix, int len, void (*putc)(char)) {
	char c, sgn = 0, pad = ' ';
	char s[20];
	int i = 0;
	if (radix < 0) {
		radix = -radix;
		if (val < 0) {
			val = -val;
			sgn = '-';
		}
	}
	if (len < 0) {
		len = -len;
		pad = '0';
	}
	if (len > 20)
		return;
	do {
		c = (char) ((unsigned long) val % radix); //cast!
		if (c >= 10) {
			c += ('A' - 10); //ABCDEF
		} else {
			c += '0'; //0123456789
		}
		s[i++] = c;
		val = (unsigned long) val / radix; //cast!
	} while (val);
	if (sgn) {
		s[i++] = sgn;
	}
	while (i < len) {

		s[i++] = pad;
	}
	do {
		(*putc)(s[--i]);
	} while (i);
}
#endif //_RPRINTF_ENABLE
#if _RSCANF_ENABLE

/**
 * Reads data from usart1  and stores them according to parameter format
 * into the locations given by the additional arguments, as if
 * scanf was used
 * Reduced version of scanf (%d, %x, %c, %n are supported)
 * %d dec integer (E.g.: 12)
 * %x hex integer (E.g.: 0xa0)
 * %b bin integer (E.g.: b1010100010)
 * %n hex, de or bin integer (e.g: 12, 0xa0, b1010100010)
 * %c any character
 * buffer support 12 bytes
 */
int rscanf(const char* format, ...) {
	va_list args;
	va_start( args, format);

	int count = 0;
	char ch = 0;
	char buffer[_SCANF_BUFFER_SIZE];
	SPRINTF_buffer = buffer;

	//get string
	while (count <= _SCANF_BUFFER_SIZE) {
		count++;
#if _USART_ENABLE
		while (!usartHasData())
			;
		ch = usartGetChar();
#endif
		if (ch != '\n' && ch != '\r') {
			*SPRINTF_buffer++ = ch;
		} else {
			break;
		}
	}
	*SPRINTF_buffer = '\0';
	//end of string

	SPRINTF_buffer = buffer;
	count = rsscanf(SPRINTF_buffer, format, args);
	va_end(args);
	return count;
}

/**
 * Reduced version of sscanf (%d, %x, %c, %n are supported)
 * %d dec integer (E.g.: 12)
 * %x hex integer (E.g.: 0xa0)
 * %b bin integer (E.g.: b1010100010)
 * %n hex, de or bin integer (e.g: 12, 0xa0, b1010100010)
 * %c any character
 */
static int rsscanf(const char* str, const char* format, va_list ap) {
	int value, tmp;
	int count;
	int pos;
	char neg, fmt_code;
	//const char* pf;
	char* sval;
	//for (pf = format, count = 0; *format != 0 && *str != 0; format++, str++) {
	for (count = 0; *format != 0 && *str != 0; format++, str++) {
		while (*format == ' ' && *format != 0) {
			format++; //
		}
		if (*format == 0) {
			break;
		}
		while (*str == ' ' && *str != 0) {
			str++; //increment pointer of input string
		}
		if (*str == 0) {
			break;
		}
		//recognize how to format
		if (*format == '%') {
			format++;
			if (*format == 'n') {
				//if in str sth like 0xff
				if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
					fmt_code = 'x';
					str += 2;
				} else if (str[0] == 'b') {
					fmt_code = 'b';
					str++;
				} else {
					fmt_code = 'd';
				}
			} else {
				fmt_code = *format; //it is format letter
			}
			switch (fmt_code) {
			case 'x':
			case 'X':
				for (value = 0, pos = 0; *str != 0; str++, pos++) {
					if ('0' <= *str && *str <= '9') {
						tmp = *str - '0';
					} else if ('a' <= *str && *str <= 'f') {
						tmp = *str - 'a' + 10;
					} else if ('A' <= *str && *str <= 'F') {
						tmp = *str - 'A' + 10;
					} else {
						break;
					}
					value *= 16;
					value += tmp;
				}
				if (pos == 0) {
					return count;
				}
				*(va_arg(ap, int*)) = value;
				count++;
				break;
			case 'b':
				for (value = 0, pos = 0; *str != 0; str++, pos++) {
					if (*str != '0' && *str != '1') {
						break;
					}
					value *= 2;
					value += *str - '0';
				}
				if (pos == 0) {
					return count;
				}
				*(va_arg(ap, int*)) = value;
				count++;
				break;
			case 'd':
				if (*str == '-') {
					neg = 1;
					str++;
				} else {
					neg = 0;
				}
				for (value = 0, pos = 0; *str != 0; str++, pos++) {
					if ('0' <= *str && *str <= '9') {
						value = value * 10 + (int) (*str - '0');
					} else {
						break;
					}
				}
				if (pos == 0) {
					return count;
				}
				*(va_arg(ap, int*)) = neg ? -value : value;
				count++;
				break;
			case 'c':
				*(va_arg(ap, char*)) = *str;
				count++;
				break;
			case 's':
				sval = va_arg(ap, char*);
				while (*str) {
					*sval++ = *str++;
					count++;
				}
				*sval = NULL;
				break;
			default:
				return count;
			}
		} else {
			if (*format != *str) {
				break;
			}
		}
	}
	return count;
}
#endif //_RSCANF_ENABLE
