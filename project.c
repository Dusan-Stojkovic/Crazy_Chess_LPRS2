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
#include <pthread.h>

//Change this to 0 if no Nokia 5110 shield is available
#define TWO_PLAYER 0

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
#endif

	gpu_p32[0] = 3; // RGB333 mode.
	gpu_p32[0x800] = 0x00ff00ff; // Magenta for HUD.

	// Game state.
	game_state_t* gs = setup_game();

	int pickup_piece_black = -1;
	point_t pos_prev_black;

	int frame_counter = 0;
	int mov_x;
	int mov_y;

	while(1){
		//TODO instead of polling register an interrupt
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
			if(pickup_piece(a, gs->white_pieces, &(gs->p1), &pickup_piece_white))
			{
				pos_prev_white = gs->p1;
			}

			if(overlap_piece(a, gs->white_pieces, pickup_piece_white))
			{
				printf("Invalid move, pieces can't overlap.\n");
				gs->white_pieces[pickup_piece_white].pos= pos_prev_white;
			}
			else if(b && pickup_piece_white > -1)
			{
				pos_prev_white = (point_t){ 0, 0 };
				pickup_piece_white = -1;
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

			/////////////////////////////////////
			// Gameplay.

			if(pickup_piece(joypad.a, gs->black_pieces, &(gs->p2), &pickup_piece_black))
			{
				pos_prev_black = gs->p2;
			}


			if(overlap_piece(joypad.a, gs->black_pieces, pickup_piece_black))
			{
				printf("Invalid move, pieces can't overlap.\n");
				gs->black_pieces[pickup_piece_black].pos= pos_prev_black;
			}
			else if(joypad.z && pickup_piece_black > -1)
			{
				pos_prev_black = (point_t){ 0, 0 };
				pickup_piece_black = -1;
			}


			// tests for valid moves goes here!
			// test 1 no overlap of pieces!
			// TODO expand this test to allow for captures!

			//switch(chesspieces[pickup_piece].p)
			//{
			//	case KING:
			//		break;
			//	case QUEEN:
			//		break;
			//	case ROOK:
			//		break;
			//	case BISHOP:
			//		break;
			//	case KNIGHT:
			//		break;
			//	case PAWN:
			//		if(validate_pawn(gs, chesspieces[pickup_piece], pos_prev) != 0)
			//		{
			//			chesspieces[pickup_piece].pos.x = pos_prev.x;
			//			chesspieces[pickup_piece].pos.y = pos_prev.y;
			//		}
			//		break;
			//}
			frame_counter = 0;
		}

		frame_counter++;
		/////////////////////////////////////
		// Drawing.
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync.
		
		//Draw background
		uint16_t step_x = 32;
		uint16_t step_y = 32;
		uint16_t startdraw_x = 0;
		uint16_t startdraw_y;

		//draw_sprite(background_sprites__p, step_x, step_y, startdraw_x, startdraw_y, 0, 0, background_sprites__w);
		
		do
		{
			startdraw_y = 0;
			do
			{
				draw_sprite(background_sprites__p, step_x, step_y, startdraw_x, startdraw_y, 0, 0, background_sprites__w);
				startdraw_y += step_y;
			}
			while(startdraw_y < SCREEN_RGB333_H - step_y);
			draw_sprite(background_sprites__p, startdraw_x, SCREEN_RGB333_H - startdraw_y, startdraw_x, startdraw_y, 0, 0, background_sprites__w);
			startdraw_x += step_x;
		}
		while(startdraw_x < SCREEN_RGB333_W - step_x);
		startdraw_y = 0;
		do
		{
			//draw_sprite(background_sprites__p, step_x, step_y, startdraw_x, startdraw_y, 0, 0);
			draw_sprite(background_sprites__p, startdraw_x, SCREEN_RGB333_H - startdraw_y, startdraw_x, startdraw_y, 0, 0, background_sprites__w);
			startdraw_y += step_y;
		}
		while(startdraw_y < SCREEN_RGB333_H - step_y);
		draw_sprite(background_sprites__p, SCREEN_RGB333_W - startdraw_x, SCREEN_RGB333_H - startdraw_y, startdraw_x, startdraw_y, 0, 0, background_sprites__w);

		draw_chessboard(gs->color);
#if TWO_PLAYER
		draw_player_cursor(gs->p1, 0x000f);
#endif
		draw_player_cursor(gs->p2, 0x00f0);

		for(uint16_t i = 0; i < PIECE_NUM / 2; i++)
		{
			// Draw chesspiece
			draw_sprite(chess_sprites__p, 10, 10, gs->white_pieces[i].pos.x, gs->white_pieces[i].pos.y, gs->white_pieces[i].atlas.x, gs->white_pieces[i].atlas.y, chess_sprites__w);
		}
		for(uint16_t i = 0; i < PIECE_NUM / 2; i++)
		{
			// Draw chesspiece
			draw_sprite(chess_sprites__p, 10, 10, gs->black_pieces[i].pos.x, gs->black_pieces[i].pos.y, gs->black_pieces[i].atlas.x, gs->black_pieces[i].atlas.y, chess_sprites__w);
		}
	}

	free(gs);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
