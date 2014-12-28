all:
	avr-gcc -Os -mmcu=atmega32u4 -Wall main.c
install:
	avrdude -p m32u4 -P /dev/ttyACM0 -c avr109 -U flash:w:a.out:e
clean:
	rm -f a.out
