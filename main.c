/* LCD: data on PORTB, E on C6, RS on C7
 * EEPROM: data on PORTB, CS on F0, OE on F1, WE on E6
 *         addr 0-7 on PORTD, addr 8-10 on F4-6
 */
#define F_CPU		16000000
#define LCD_DELAY_MS	5
#define MEM_DELAY_MS	1
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

void lcd_init();
void lcd_print(const char *str);
void lcd_printf(const char *format, ...);
void lcd_send(unsigned char rs, unsigned char data);
void mem_init();
unsigned char mem_read(int addr);
void mem_write(int addr, unsigned char data);

int main()
{
	int addr;

	mem_init();
	lcd_init();

	/*for (addr = 0x0000; addr < 0x07F0; addr++)
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
		mem_write(addr, 0x00);*/

	for (addr = 0x0000; addr < 0x0800; addr++) {
		if (addr % 8 == 0) {
			_delay_ms(1000);
			lcd_send(0, 0x01);
		}
		if (addr % 4 == 0) {
			lcd_printf("0x%04x: %02x ", addr, mem_read(addr));
		} else {
			lcd_printf("%02x ", mem_read(addr));
		}
	}

	return 0;
}

void mem_init()
{
	DDRF = 0x73;
	DDRE = 0x40;
	PORTF = 0x03; /* CS,OE off */
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
	lcd_send(0, 0x38);
	lcd_send(0, 0x06);
	lcd_send(0, 0x0E);
	lcd_send(0, 0x01);
}

void lcd_printf(const char *format, ...)
{
	va_list values;
	char line[21];

	va_start(values, format);
	vsnprintf(line, 21, format, values);
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
	PORTC = (rs << 7) | 0x40;
	_delay_ms(LCD_DELAY_MS);
	PORTC = (rs << 7);
}

