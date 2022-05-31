///////////////////////////////////////////////////////////////////////////////
// Headers.

#include <stdint.h>
#include "system.h"
#include <stdio.h>
#include <unistd.h>
#include "chesspieces.h"
#include "chess_logic.h"
#include "emulator.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                // error_message ("error %d from tcgetattr", errno);
				printf("error 1");
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        //tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                //error_message ("error %d from tcsetattr", errno);
				printf("error 2");
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                //error_message ("error %d from tggetattr", errno);
				printf("error 3");
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
				printf("error 4");
                //error_message ("error %d setting term attributes", errno);
}

int main(void) {
////////////////////////////////////////////////////////////////////////////////
// Setup.
	gpu_p32[0] = 3; // RGB333 mode.
	gpu_p32[0x800] = 0x00ff00ff; // Magenta for HUD.

	// Game state.
	game_state_t* gs = setup_game();
	int device = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);
	
	set_interface_attribs (device, B9600, 0);  
	set_blocking (device, 0);

	printf("joystick connection open at %i device\n", device);

	int pickup_piece = -1;
	int key_pressed = 0;
	point_t pos_prev;
	color_type player_to_move = WHITE;

	joystick_t joystick;
	int size = sizeof(joystick);
	char buffer[size];

	while(1){
		joystick.a = 0;
		joystick.b = 0;
		joystick.c = 0;
		joystick.d = 0;
		joystick.x = 0;
		joystick.y = 0;
		memset(buffer, 0, size);
		int n = read(device, (char*)(&joystick), sizeof(joystick));
		sleep(1);
		//memcpy(&joystick, buffer, size);
		printf("%i %i %i %i %i %i %i\n", n, joystick.a, joystick.b, joystick.c, joystick.d, joystick.x, joystick.y);
		
		/////////////////////////////////////
		// Poll controls.
		int mov_x = 0;
		int mov_y = 0;
		int toggle_active = joypad.up + joypad.down + joypad.left + joypad.right ? 1 : 0;
		chess_piece_t* chesspieces;
		/////////////////////////////////////
		// Gameplay.
		
		if(toggle_active){
			key_pressed = joypad.up + (joypad.down << 1) + (joypad.left << 2) + (joypad.right << 3);
		}
		else
		{
			switch(key_pressed)
			{
			//up pressed
			case 1:
				mov_y = -1;
				break;
			//down pressed
			case 2:
				mov_y = +1;
				break;
			//left pressed
			case 4:
				mov_x = -1;
				break;
			//right pressed
			case 8:
				mov_x = +1;
				break;
			}

			if(gs->p1.x + mov_x*STEP < gs->chessboard_offset[0])
			{
				gs->p1.x = gs->chessboard_offset[0];
			}
			else if(gs->p1.x + mov_x*STEP < gs->chessboard_offset[0] + CHESSBOARD - SQ_A)
			{
				gs->p1.x += mov_x*STEP;
			}
			else
			{
				gs->p1.x = gs->chessboard_offset[0] + CHESSBOARD - SQ_A;
			}
			if(gs->p1.y + mov_y*STEP < gs->chessboard_offset[1])
			{
				gs->p1.y = gs->chessboard_offset[1];
			}
			else if(gs->p1.y + mov_y*STEP < gs->chessboard_offset[1] + CHESSBOARD - SQ_A)
			{
				gs->p1.y += mov_y*STEP;
			}
			else
			{
				gs->p1.y = gs->chessboard_offset[1] + CHESSBOARD - SQ_A;
			}

			key_pressed = 0;

			if(player_to_move == WHITE)
				chesspieces = gs->white_pieces;
			else
				chesspieces = gs->black_pieces;

			if(joypad.a && pickup_piece == -1)
			{
				for(uint8_t i = 0; i < PIECE_NUM / 2; i++)
				{
					if(chesspieces[i].pos.x == gs->p1.x && chesspieces[i].pos.y == gs->p1.y && chesspieces[i].c == player_to_move)
					{
						pickup_piece = i;
						pos_prev.x = chesspieces[i].pos.x;
						pos_prev.y = chesspieces[i].pos.y;
						break;
					}
				}
			}
			if(joypad.a && pickup_piece > -1)
			{
				chesspieces[pickup_piece].pos.x = gs->p1.x;
				chesspieces[pickup_piece].pos.y = gs->p1.y;
				
			}
			if(joypad.a == 0 && pickup_piece > -1)
			{

				// tests for valid moves goes here!
				// test 1 no overlap of pieces!
				// TODO expand this test to allow for captures!
				for(uint8_t i = 0; i < PIECE_NUM / 2; i++)
				{
					if(chesspieces[i].pos.x == chesspieces[pickup_piece].pos.x && 
							chesspieces[i].pos.y == chesspieces[pickup_piece].pos.y && 
							i != pickup_piece)
					{
						printf("Invalid move, pieces can't overlap.\n");
						chesspieces[i].pos.x = pos_prev.x;
						chesspieces[i].pos.y = pos_prev.y;
						break;
					}
				}

				switch(chesspieces[pickup_piece].p)
				{
					case KING:
						break;
					case QUEEN:
						break;
					case ROOK:
						break;
					case BISHOP:
						break;
					case KNIGHT:
						break;
					case PAWN:
						if(validate_pawn(gs, chesspieces[pickup_piece], pos_prev) != 0)
						{
							chesspieces[pickup_piece].pos.x = pos_prev.x;
							chesspieces[pickup_piece].pos.y = pos_prev.y;
						}
						break;
				}

				pos_prev.x = 0;
				pos_prev.y = 0;
				pickup_piece = -1;
				player_to_move *= -1;
			}
		}

		/////////////////////////////////////
		// Drawing.
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync.
		
		draw_background();
		draw_chessboard(gs);
		draw_player_cursor(gs, 0x000d);

		for(uint16_t i = 0; i < PIECE_NUM / 2; i++)
		{
			// Draw chesspiece
			draw_sprite(chess_sprites__p, 10, 10, gs->white_pieces[i].pos.x, gs->white_pieces[i].pos.y, gs->white_pieces[i].atlas.x, gs->white_pieces[i].atlas.y);
		}
		for(uint16_t i = 0; i < PIECE_NUM / 2; i++)
		{
			// Draw chesspiece
			draw_sprite(chess_sprites__p, 10, 10, gs->black_pieces[i].pos.x, gs->black_pieces[i].pos.y, gs->black_pieces[i].atlas.x, gs->black_pieces[i].atlas.y);
		}
	}

	free(gs);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
