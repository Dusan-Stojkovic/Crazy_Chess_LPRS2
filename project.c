///////////////////////////////////////////////////////////////////////////////
// Headers.
#define _XOPEN_SOURCE 500

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "system.h"
#include "chesspieces.h"
#include "background.h"
#include "chess_logic.h"
#include "joystick.h"
#include "emulator.h"
#include "7SEGM_green.h"
#include "entry.h"
#include "white_win.h"
#include "black_win.h"
#include "credits.h"
#include "7SEGM_red.h"
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <sys/time.h>

//Change this to 0 if no Nokia 5110 shield is available
#define TWO_PLAYER 0
#define PERIOD 125000 //us

game_state_t* gs;
#if TWO_PLAYER
joystick_t joystick;
#endif

void update_movement(int signum);

int main(void) {
	auto int start_menu(game_state_t*);
	auto int game(game_state_t*);

	gpu_p32[0] = 3; // RGB333 mode.
	gpu_p32[0x800] = 0x00ff00ff; // Magenta for HUD.

	do
	{
		free(gs);
		gs = setup_game();
	}while(start_menu(gs));

	int start_menu(game_state_t* gs)
	{
		uint8_t mode = 0;
		uint16_t *screen;
		screen = entry__p;
		point_t buttons[3] = {
			(point_t){48, 45},
			(point_t){48, 70},
			(point_t){48, 96}
		};
		point_t game_over[2] = {
			(point_t){48, 70},
			(point_t){48, 96}
		};
		point_t arrow = (point_t){137, 97};
		uint8_t pos = 0;
		uint32_t falling_edge = 0;
		uint8_t pos_max = 3;
		while(1)
		{
			if(mode == 0 || mode == 2)
			{
				falling_edge += (joypad.start << 24) + (joypad.up << 16) + joypad.down;
				if(joypad.down == 0 && (falling_edge & 0x0000ffff) > 0)
				{
					pos = pos + 1 == pos_max ? 0 : pos + 1;
					falling_edge = 0;
				}
				if(joypad.up == 0 && (falling_edge & 0x00ff0000) > 0)
				{
					pos = pos - 1 == -1 ? 2 : pos - 1;
					falling_edge = 0;
				}
				if(joypad.start == 0 && (falling_edge & 0xff000000) > 0)
				{
					if(mode == 0)
					{
						switch(pos)
						{
						case 0:
								
							falling_edge = 0;
							if(game(gs) > 0)
							{
								screen = white_win__p;
								mode = 2;
							}
							else
							{
								screen = black_win__p;
								mode = 2;
							}
							pos_max = 2;
							break;
						case 1:
							mode = 1;
							falling_edge = 0;
							screen = credits__p;	
							break;
						case 2:
							return 1;
							break;
						}
					}
					else if(mode == 2)
					{
						if(pos == 0)
						{
							return 1;
						}
						else
						{
							return 0;
						}
					}
				}
			}
			else if(mode == 1)
			{
				falling_edge += joypad.start;
				if(joypad.start == 0 && falling_edge > 0)
				{
					mode = 0;
					screen = entry__p;
					falling_edge = 0;
				}
			}
			// Detecting rising edge of VSync.
			WAIT_UNITL_0(gpu_p32[2]);
			WAIT_UNITL_1(gpu_p32[2]);

			draw_sprite(screen, SCREEN_RGB333_W, SCREEN_RGB333_H, 0, 0, 0, 0, SCREEN_RGB333_W);

			if(mode == 0)
			{
				gs->p2 = buttons[pos];
				draw_player_cursor(gs->p2, 0x0f00, BUTTON_W, BUTTON_H, 2);
			}
			if(mode == 1)
			{
				gs->p2 = arrow;
				draw_player_cursor(gs->p2, 0x00f0, ARROW_W, ARROW_H, 1);
			}
			if(mode == 2)
			{
				gs->p2 = game_over[pos];
				draw_player_cursor(gs->p2, 0x0f00, BUTTON_W, BUTTON_H, 2);
			}
		}
		return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Setup.
	int game(game_state_t* gs)
	{
		struct sigaction sa; 
  		struct itimerval timer; 
  		/* Install timer_handler as the signal handler for SIGVTALRM.  */ 
  		memset (&sa, 0, sizeof (sa)); 
  		sa.sa_handler = &update_movement; 
  		sigaction (SIGVTALRM, &sa, NULL); 
  		/* Configure the timer to expire after 125 msec...  */ 
  		timer.it_value.tv_sec = 0;  
  		timer.it_value.tv_usec = PERIOD;
  		/* ... and every 250 msec after that.  */ 
  		timer.it_interval.tv_sec = 0;  
  		timer.it_interval.tv_usec = PERIOD; 
  		/* Start a virtual timer. It counts down whenever this process is executing.  */ 
  		setitimer (ITIMER_VIRTUAL, &timer, NULL);

#if	TWO_PLAYER
		int device = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK | O_NOCTTY);
 		
		set_interface_attribs (device, B38400, 0);  

		printf("joystick connection open at %i device\n", device);

		int pickup_piece_white = -1;
		point_t pos_prev_white;
		int a;
		int b;
		int c;
		int d;

		uint8_t pool_select_white = 0;
		uint8_t show_pool_white = 0;
		uint8_t pool_size_white = 0;
		chess_piece_t *spawn_white = NULL;
#endif
		int white_num = PIECE_NUM / 2;
		int black_num = PIECE_NUM / 2;

		int score_white = 0;
		int score_black = 0;
		int score = 0;

		uint8_t pool_select_black = 0;
		uint8_t show_pool_black = 0;
		uint8_t pool_size_black = 0;
		chess_piece_t *spawn_black = NULL;
		uint8_t keyboard_toggle = 0;
		uint8_t key_a = 0;
		uint8_t key_b = 0;
		uint8_t key_z = 0;
		uint8_t key_start = 0;

		int pickup_piece_black = -1;
		point_t pos_prev_black;

		gs->p1.x = 80;
		gs->p1.y = 100;

		gs->p2.x = 80;
		gs->p2.y = 30;

		while(1){
#if TWO_PLAYER
			shield_input(&joystick, device);	

			a = joystick.buttons & 0x1;
			b = (joystick.buttons & 0x2) >> 1;
			c = (joystick.buttons & 0x4) >> 2;
			d = (joystick.buttons & 0x8) >> 3;
			//printf("%x %i %i %i %i %i %i\n", joystick.magic, a, b, c, d, joystick.x, joystick.y);

			/////////////////////////////////////
			// Gameplay.

			//TODO integrate this validation better
			if(pickup_piece(a, gs->white_pieces, white_num, &(gs->p1), &pickup_piece_white))
			{
				pos_prev_white = gs->p1;
			}

			if(overlap_piece(a, gs->white_pieces, white_num, pickup_piece_white))
			{
				printf("Invalid move, pieces can't overlap.\n");
				gs->white_pieces[pickup_piece_white].pos= pos_prev_white;
				gs->p1 = pos_prev_white;
			}
			else if(b && pickup_piece_white > -1)
			{
				//TODO test for captures
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
			}
			else
			{
				pos_prev_white = gs->p1;
			}

			//Spawn mode turned on
			if(c)
			{
				if(show_pool_white == 0 && score_white > 0)
				{
					show_pool_white = 1;
					pool_size_white = spawn_pool(&spawn_white, WHITE, &score_white);
					pool_size_white--;
				}
				else if(show_pool_white)
				{
					//Try to append new piece to array
					gs->white_pieces[white_num] = spawn_white[pool_select_white];
					gs->white_pieces[white_num++].pos = gs->p1;
					if(overlap_piece(0, gs->white_pieces, white_num, white_num - 1))
					{
						printf("Invalid spawn spot, pieces can't overlap.\n");
						white_num--;
					}
					//TODO think about adding piece combat to spawn
					else
					{
						score_white -= spawn_white[pool_select_white].t;	
						pool_select_white = 0;
						show_pool_white = 0;
						pool_size_white = -1;
						free(spawn_white);
						pickup_piece_white = white_num - 1;
					}
				}
			}

			score = piece_combat(gs->white_pieces, gs->black_pieces, pickup_piece_white, &white_num, &black_num);
			if(score > 0)
			{
				if(score == 10)
				{
					score = 20;
					printf("Game over, white wins!\n");
					break;
				}
				score_white += score;
				printf("Updated score for white to: %i\n", score_white);
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
			}
			else if(score < 0)
			{
				if(score == -10)
				{
					score = -10;
					printf("Game over, black wins!\n");
					break;
				}
				score_black -= score;
				printf("Updated score for black to: %i\n", score_black);
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
			}
			if(d)
			{
				pool_select_white = ++pool_select_white > pool_size_white ? 0 : pool_select_white;
			}
#endif
			keyboard_toggle = keyboard_input();
			key_a = keyboard_toggle & 1;
			key_b = (keyboard_toggle & 2) >> 1;
			key_z = (keyboard_toggle & 4) >> 2;
			key_start = (keyboard_toggle & 8) >> 3;

			if(pickup_piece(key_a, gs->black_pieces, black_num, &(gs->p2), &pickup_piece_black))
			{
				pos_prev_black = gs->p2;
			}

			if(overlap_piece(key_a, gs->black_pieces, black_num, pickup_piece_black))
			{
				printf("Invalid move, pieces can't overlap.\n");
				gs->black_pieces[pickup_piece_black].pos= pos_prev_black;
				gs->p2 = pos_prev_black;
			}
			else if(key_z && pickup_piece_black > -1)
			{
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}
			else
			{
				pos_prev_black = gs->p2;
			}
			
			if(key_start)
			{
				if(show_pool_black == 0 && score_black > 0)
				{
					show_pool_black = 1;
					pool_size_black = spawn_pool(&spawn_black, BLACK, &score_black);
					pool_size_black--;
				}
				else if(show_pool_black)
				{
					gs->black_pieces[black_num] = spawn_black[pool_select_black];
					gs->black_pieces[black_num++].pos = gs->p2;
					if(overlap_piece(0, gs->black_pieces, black_num, black_num - 1))
					{
						printf("Invalid spawn spot, pieces can't overlap.\n");
						black_num--;
					}
					else
					{
						score_black -= spawn_black[pool_select_black].t;	
						pool_select_black = 0;
						show_pool_black = 0;
						pool_size_black = 0;
						free(spawn_black);
						pickup_piece_black = black_num - 1;
					}
				}
			}
			score = piece_combat(gs->black_pieces, gs->white_pieces, pickup_piece_black, &black_num, &white_num);
			if(score > 0)
			{
				if(score == 10)
				{
					score = -20;
					printf("Game over, black wins!\n");
					break;
				}
				score_black += score;
				printf("Updated score for black to: %i\n", score_black);
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}
			else if(score < 0)
			{
				if(score == -10)
				{
					score = 10;
					printf("Game over, white wins!\n");
					break;
				}
				score_white -= score;
				printf("Updated score for white to: %i\n", score_white);
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}

			if(key_b)
			{
				pool_select_black = ++pool_select_black > pool_size_black ? 0 : pool_select_black;
			}
			/////////////////////////////////////
			// Drawing.
			
			// Detecting rising edge of VSync.
			WAIT_UNITL_0(gpu_p32[2]);
			WAIT_UNITL_1(gpu_p32[2]);

			// Draw in buffer while it is in VSync.
			draw_sprite(background_sprites__p, SCREEN_RGB333_W, SCREEN_RGB333_H, 0, 0, 0, 0, background_sprites__w);
			
			draw_sprite(green__p[score_black / 10], 10, 20, 121, 30, 0, 0, GREEN_W);
			draw_sprite(green__p[score_black % 10], 10, 20, 132, 30, 0, 0, GREEN_W);

			draw_sprite(green__p[score_white / 10], 10, 20, 121, 90, 0, 0, GREEN_W);
			draw_sprite(green__p[score_white % 10], 10, 20, 132, 90, 0, 0, GREEN_W);

			//draw_chessboard(gs->color); TODO remove this function as chessboard is already drawn on the sprite.
#if	TWO_PLAYER
			if(show_pool_white)
			{
				draw_sprite(chess_sprites__p, 10, 10, spawn_white[pool_select_white].pos.x, spawn_white[pool_select_white].pos.y, spawn_white[pool_select_white].atlas.x, spawn_white[pool_select_white].atlas.y, chess_sprites__w);
			}
			draw_player_cursor(gs->p1, 0x000f, 10, 10, 1);
#endif
			draw_player_cursor(gs->p2, 0x00f0, 10, 10, 1);

			for(uint16_t i = 0; i < white_num; i++)
			{
				// Draw chesspiece
				draw_sprite(chess_sprites__p, 10, 10, gs->white_pieces[i].pos.x, gs->white_pieces[i].pos.y, gs->white_pieces[i].atlas.x, gs->white_pieces[i].atlas.y, chess_sprites__w);
			}
			for(uint16_t i = 0; i < black_num; i++)
			{
				// Draw chesspiece
				draw_sprite(chess_sprites__p, 10, 10, gs->black_pieces[i].pos.x, gs->black_pieces[i].pos.y, gs->black_pieces[i].atlas.x, gs->black_pieces[i].atlas.y, chess_sprites__w);
			}

			if(show_pool_black)
			{
				//draw spawn pool black
				draw_sprite(chess_sprites__p, 10, 10, spawn_black[pool_select_black].pos.x, spawn_black[pool_select_black].pos.y, spawn_black[pool_select_black].atlas.x, spawn_black[pool_select_black].atlas.y, chess_sprites__w);
			}
		}
  		timer.it_interval.tv_usec = 0; 

#if TWO_PLAYER
		close(device);
#endif
		return score;
	}

	free(gs);
	return 0;
}

void update_movement(int signum)
{
	int mov_x = 0;
	int mov_y = 0;
#if TWO_PLAYER
	// ADC logic 
	if(joystick.x < -THRESHOLD)
	{
		mov_x += -1;
	}
	if(joystick.x > THRESHOLD)
	{
		mov_x += 1;
	}
	if(joystick.y < -THRESHOLD)
	{
		mov_y += 1;
	}
	if(joystick.y > THRESHOLD)
	{
		mov_y += -1;
	}

	update_cursor(&(gs->p1), mov_x, mov_y);
#endif
	mov_x = 0;
	mov_y = 0;
	
	//Regular logic
	if(joypad.up)
	{
		mov_y += -1;
	}
	if(joypad.down)
	{
		mov_y += 1;
	}
	if(joypad.left)
	{
		mov_x += -1;
	}
	if(joypad.right)
	{
		mov_x += 1;
	}
	
	update_cursor(&(gs->p2), mov_x, mov_y);
}
///////////////////////////////////////////////////////////////////////////////
