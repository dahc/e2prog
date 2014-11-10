/* main.c for e2prog: a simple program to read/write 2716/6116 pin-compatible
 * memory chips and display content on a 20464 LCD display using an 8-bit
 * AVR microcontroller (specifically an ATmega32u4, though others may work).
 *
 * Hardware assumptions:
 * 	LCD:	data on B0-7, E on C6, RS on C7
 * 	MEM:	data on B0-7, CS on F0, OE on F1, WE on E6
 * 		addr 0-7 on D0-7, addr 8-10 on F4-6
 */
#define F_CPU		16000000
#define LCD_DELAY_MS	2
#define MEM_DELAY_MS	1
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

void lcd_init();
void lcd_clear();
void lcd_send(unsigned char rs, unsigned char data);
void lcd_print(const char *str);
void lcd_printf(const char *format, ...);
void mem_init();
unsigned char mem_read(int addr);
void mem_dump(int start, int end);
void mem_write(int addr, unsigned char data);
void mem_prog_grossblatt_test();

int main()
{
	mem_init();
	lcd_init();
	mem_prog_grossblatt_test();
	mem_dump(0x0000, 0x0800); /* display the whole space */
	mem_dump(0x07F0, 0x07F8); /* end back on the interesting part */

	return 0;
}

/* write the 8255 test program from Grossblatt page 113 */
void mem_prog_grossblatt_test()
{
	int addr;

	for (addr = 0x0000; addr < 0x07F0; addr++)
		mem_write(addr, 0x00);
	mem_write(0x7F0, 0xB0);
	mem_write(0x7F1, 0x90);
	mem_write(0x7F2, 0xE6);
	mem_write(0x7F3, 0x03);
	mem_write(0x7F4, 0xB0);
	mem_write(0x7F5, 0x01);
	mem_write(0x7F6, 0xE6);
	mem_write(0x7F7, 0x01);
	for (addr = 0x07F8; addr < 0x0800; addr++)
		mem_write(addr, 0x00);
}

/* display memory range [start, end) in 8-byte pages at one second intervals
 * (assumes start and end are 8-byte aligned)
 */
void mem_dump(int start, int end)
{
	int addr;

	for (addr = start; addr < end; addr++) {
		if (addr % 8 == 0) {
			_delay_ms(1000);
			lcd_clear();
		}
		if (addr % 4 == 0) {
			lcd_printf("0x%04X: %02X ", addr, mem_read(addr));
		} else {
			lcd_printf("%02X ", mem_read(addr));
		}
	}
}

void mem_init()
{
	DDRF = 0x73;
	DDRE = 0x40;
	PORTF = 0x03; /* CS and OE off */
	PORTE = 0x40; /* WE off */
}

unsigned char mem_read(int addr)
{
	unsigned char data;

	DDRB = 0x00;
	DDRD = 0xFF;
	PORTD = addr;
	PORTF = 0x70 & addr >> 4; /* 0 in the low nibble enables CS and OE */
	_delay_ms(MEM_DELAY_MS);
	data = PINB;
	PORTF = 0x03;		  /* CS and OE back off */
	
	return data;
}

void mem_write(int addr, unsigned char data)
{
	DDRB = 0xFF;
	DDRD = 0xFF;
	PORTD = addr;
	PORTF = 0x02 | (0x70 & addr >> 4); /* 0x02: CS on, OE off */
	PORTE = 0x00;			   /* WE on */
	PORTB = data;
	_delay_ms(MEM_DELAY_MS);
	PORTE = 0x40;			   /* WE off */
	_delay_ms(MEM_DELAY_MS);
	PORTF = 0x03;
}

void lcd_init()
{
	lcd_send(0, 0x38); /* function set: 8-bit, 2 lines (?), 5x7 font */
	lcd_send(0, 0x06); /* entry mode set: auto-increment, no shift */
	lcd_send(0, 0x0E); /* display control: display & cursor on, no blink */
	lcd_clear();
}

void lcd_clear()
{
	lcd_send(0, 0x01);
	_delay_ms(LCD_DELAY_MS); /* clear is a little slow, let it finish */
}

void lcd_printf(const char *format, ...)
{
	va_list values;
	char line[21];

	va_start(values, format);
	vsnprintf(line, 20, format, values);
	va_end(values);
	lcd_print(line);
}

void lcd_print(const char *str)
{
	while (*str)
		lcd_send(1, *str++);
}

void lcd_send(unsigned char rs, unsigned char data)
{
	DDRB = 0xFF;
	DDRC = 0xC0;
	PORTB = data;
	PORTC = 0x40 | rs << 7; /* enable on, register select as given */
	_delay_ms(LCD_DELAY_MS);
	PORTC = rs << 7;        /* enable off */
}

