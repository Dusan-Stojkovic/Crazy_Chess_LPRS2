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

void* shield_input(void *p_joy)
{
	int device = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK | O_NOCTTY);
 	
	set_interface_attribs (device, B38400, 0);  

	printf("joystick connection open at %i device\n", device);
	joystick_t* joystick = (joystick_t*)p_joy;

	int minX = 0, maxX = 0;
	int minY = 0, maxY = 0;

	int size = sizeof(joystick_t);
	uint8_t* bits = (uint8_t*)joystick;
	printf("%i\n", size);

	while(1)
	{
		for(int i = 0; i < size; i++)
		{
			int n = read(device, &(bits[i]), 1);
		}
		if(joystick->magic != MAGIC)
		{
			//printf("ns : %x\n", joystick.magic);
			memset(bits, 0, size);
			continue;
		}
		//printf("Sync \n");
		//TODO sync these values now to our main program
		int a = joystick->buttons & 0x1;
		int b = (joystick->buttons & 0x2) >> 1;
		int c = (joystick->buttons & 0x4) >> 2;
		int d = (joystick->buttons & 0x8) >> 3;
		printf("%i %i %i %i %i %i\n", a, b, c, d, joystick->x, joystick->y);
		memset(bits, 0, size);
		//delay might not be needed

#if DEBUG_EXTREMA
		if(minX > joystick.x)
		{
			minX = joystick.x;
		}
		if(minY > joystick.y)
		{
			minY = joystick.y;
		}
		if(maxX < joystick.x)
		{
			maxX = joystick.x;

		}
		if(maxY < joystick.y)
		{
			maxY = joystick.y;
		}
		printf("extrema X: %i %i\nextrema Y: %i %i\n", minX, maxX, minY, maxY);
#endif
	}
}


#endif
