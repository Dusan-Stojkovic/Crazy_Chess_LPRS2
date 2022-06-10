#ifndef JOYSTICK_H

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEBUG_EXTREMA 0

#define MAGIC 0xabbaceca

typedef struct __attribute__((__packed__))
{
  uint32_t magic;
  int16_t x;
  int16_t y;
  uint8_t buttons;
}joystick_t;

int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	if (tcgetattr (fd, &tty) != 0)
	{
	        // error_message ("error %d from tcgetattr", errno);
			printf("error 1");
	        return -1;
	}
	
	tty.c_cflag = speed | CS8 | CLOCAL | CREAD;		//<Set baud rate
	tty.c_iflag = IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &tty);
	
	return 0;
}

void shield_input(joystick_t* joystick, int device)
{
	tcflush(device, TCIFLUSH);

	int size = sizeof(joystick_t);
	uint8_t* bits = (uint8_t*)joystick;
	uint8_t bit = 0;
	uint8_t prev_bit = 0;

	//Magic calculations
	char magic_size = sizeof(MAGIC);
	char valid = 0;
	uint8_t magic_shift = 0;
	uint32_t shift_mask = 0xff; 

	while(valid != magic_size)
	{
		int n = read(device, &bit, 1);
		if((uint8_t)((MAGIC & shift_mask) >> magic_shift) == bit)
		{
			bits[valid] = bit;
			magic_shift = 8*(++valid);
			shift_mask = shift_mask << 8;
		}
		else
		{
			memset(bits, 0, size);
			valid = 0;
			magic_shift = 0;
			shift_mask = 0xff;
		}
	}
	//Valid data get rest 
	int i;
	for(i = 0; i < size - magic_size; i++)
	{
		int n = read(device, &bit, 1);
		bits[valid++] = bit;
	}
}


#endif
