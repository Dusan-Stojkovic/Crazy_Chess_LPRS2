///////////////////////////////////////////////////////////////////////////////
// Headers.

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
#include "7SEGM_red.h"
#include <pthread.h>

//Change this to 0 if no Nokia 5110 shield is available
#define TWO_PLAYER 1

int main(void) {
////////////////////////////////////////////////////////////////////////////////
// Setup.

#if TWO_PLAYER
	int device = open("/dev/ttyUSB0", O_RDWR | O_NONBLOCK | O_NOCTTY);
 	
	set_interface_attribs (device, B38400, 0);  

	printf("joystick connection open at %i device\n", device);

	joystick_t joystick;

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

	gpu_p32[0] = 3; // RGB333 mode.
	gpu_p32[0x800] = 0x00ff00ff; // Magenta for HUD.

	// Game state.
	game_state_t* gs = setup_game();
	int white_num = PIECE_NUM / 2;
	int black_num = PIECE_NUM / 2;

	int score_white = 0;
	int score_black = 0;
	int score = 0;

	uint8_t pool_select_black = 0;
	uint8_t show_pool_black = 0;
	uint8_t pool_size_black = 0;
	chess_piece_t *spawn_black = NULL;

	int pickup_piece_black = -1;
	point_t pos_prev_black;

	int frame_counter = 0;
	int mov_x;
	int mov_y;

	while(1){
		//TODO instead of polling register an interrupt
		//Seems like this moment adds significant delay.
		//I think that we sould update button logic each frame
		//but slow down the movement with the frame counter 
		//temporarily
		if(frame_counter == 5)
		{
			/////////////////////////////////////
			// Poll controls.
#if TWO_PLAYER
			shield_input(&joystick, device);	

			a = joystick.buttons & 0x1;
			b = (joystick.buttons & 0x2) >> 1;
			c = (joystick.buttons & 0x4) >> 2;
			d = (joystick.buttons & 0x8) >> 3;
		//printf("%x %i %i %i %i %i %i\n", joystick.magic, a, b, c, d, joystick.x, joystick.y);

			/////////////////////////////////////
			// Gameplay.
			mov_x = 0;
			mov_y = 0;

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

			score = piece_combat(gs->white_pieces, gs->black_pieces, pickup_piece_white, &white_num, &black_num);
			if(score > 0)
			{
				score_white += score;
				printf("Updated score for white to: %i\n", score_white);
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
			}
			else if(score < 0)
			{
				score_black -= score;
				printf("Updated score for black to: %i\n", score_black);
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
			}

			//Spawn mode turned on
			if(c)
			{
				if(pool_size_white == 0)
				{
					show_pool_white = 1;
					pool_size_white = spawn_pool(&spawn_white, WHITE, &score_white);
					if(pool_size_white == 0)
					{

						pool_select_white = 0;
						show_pool_white = 0;
						free(spawn_white);
					}
				}
				else
				{
					score_white -= spawn_white[pool_select_white].t;	
					spawn_white[pool_select_white].pos = gs->p1;
					gs->white_pieces[white_num++] = spawn_white[pool_select_white];
					pool_select_white = 0;
					show_pool_white = 0;
					pool_size_white = 0;
					free(spawn_white);
				}
			}

			if(d)
			{
				pool_select_white = ++pool_select_white > pool_size_white ? 0 : pool_select_white;
			}
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

			if(pickup_piece(joypad.a, gs->black_pieces, black_num, &(gs->p2), &pickup_piece_black))
			{
				pos_prev_black = gs->p2;
			}

			if(overlap_piece(joypad.a, gs->black_pieces, black_num, pickup_piece_black))
			{
				printf("Invalid move, pieces can't overlap.\n");
				gs->black_pieces[pickup_piece_black].pos= pos_prev_black;
				gs->p2 = pos_prev_black;
			}
			else if(joypad.z && pickup_piece_black > -1)
			{
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}
			else
			{
				pos_prev_black = gs->p2;
			}
			
			score = piece_combat(gs->black_pieces, gs->white_pieces, pickup_piece_black, &black_num, &white_num);
			if(score > 0)
			{
				score_black += score;
				printf("Updated score for black to: %i\n", score_black);
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}
			else if(score < 0)
			{
				score_white -= score;
				printf("Updated score for white to: %i\n", score_white);
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}
			if(joypad.start)
			{
				if(pool_size_black == 0)
				{
					show_pool_black = 1;
					pool_size_black = spawn_pool(&spawn_black, BLACK, &score_black);
					if(pool_size_black == 0)
					{
						pool_select_black = 0;
						show_pool_black = 0;
						free(spawn_black);
					}
				}
				else
				{
					score_black -= spawn_black[pool_select_black].t;	
					spawn_black[pool_select_black].pos = gs->p2;
					gs->black_pieces[black_num++] = spawn_black[pool_select_black];
					pool_select_black = 0;
					show_pool_black = 0;
					pool_size_black = 0;
					free(spawn_black);
				}
			}

			if(joypad.b)
			{
				pool_select_black = ++pool_select_black > pool_size_black ? 0 : pool_select_black;
			}
			frame_counter = 0;
		}

		frame_counter++;
		/////////////////////////////////////
		// Drawing.
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);

		// Draw in buffer while it is in VSync.
		draw_sprite(background_sprites__p, SCREEN_RGB333_W, SCREEN_RGB333_H, 0, 0, 0, 0, background_sprites__w);

		draw_sprite(green_0__p, 10, 20, 120, 20, 0, 0, GREEN_W);
		draw_sprite(green_0__p, 10, 20, 135, 20, 0, 0, GREEN_W);

		draw_chessboard(gs->color);
#if TWO_PLAYER
		draw_player_cursor(gs->p1, 0x000f);
		if(show_pool_white)
		{
			draw_sprite(chess_sprites__p, 10, 10, spawn_white[pool_select_white].pos.x, spawn_white[pool_select_white].pos.y, spawn_white[pool_select_white].atlas.x, spawn_white[pool_select_white].atlas.y, chess_sprites__w);
		}
#endif
		draw_player_cursor(gs->p2, 0x00f0);

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

	free(gs);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
