
///////////////////////////////////////////////////////////////////////////////
// Headers.

#include <stdint.h>
#include "system.h"
#include <stdio.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// HW stuff.

#define WAIT_UNITL_0(x) while(x != 0){}
#define WAIT_UNITL_1(x) while(x != 1){}

#define SCREEN_W 640
#define SCREEN_H 480

#define gpu_p32 ((volatile uint32_t*)LPRS2_GPU_BASE)
#define palette_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x1000))
#define unpack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x400000))
#define pack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x600000))
#define joypad_p32 ((volatile uint32_t*)LPRS2_JOYPAD_BASE)

typedef struct {
	unsigned a      : 1;
	unsigned b      : 1;
	unsigned z      : 1;
	unsigned start  : 1;
	unsigned up     : 1;
	unsigned down   : 1;
	unsigned left   : 1;
	unsigned right  : 1;
} bf_joypad;
#define joypad (*((volatile bf_joypad*)LPRS2_JOYPAD_BASE))

typedef struct {
	uint32_t m[SCREEN_H][SCREEN_W];
} bf_unpack_idx1;
#define unpack_idx1 (*((volatile bf_unpack_idx1*)unpack_idx1_p32))

#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Game config.

#define STEP 32
#define RECT_H 64
#define RECT_W 128
#define SQ_A 256

#define UNPACKED_0_PACKED_1 1



///////////////////////////////////////////////////////////////////////////////
// Game data structures.

typedef struct {
	uint16_t x;
	uint16_t y;
} point_t;

typedef enum {RECT, SQ} player_t;

typedef struct {
	// Upper left corners.
	point_t rect;
	point_t sq;
	
	player_t active;
} game_state_t;



///////////////////////////////////////////////////////////////////////////////
// Game code.

int main(void) {
	
	// Setup.
	gpu_p32[0] = 1; // 1b index mode.
	gpu_p32[1] = UNPACKED_0_PACKED_1;
	palette_p32[0] = 0x00ff0000; // Blue for background.
	palette_p32[1] = 0x000000ff; // Red for players.
	gpu_p32[0x800] = 0x0000ff00; // Green for HUD.


	// Game state.
	game_state_t gs;
	gs.rect.x = 0;
	gs.rect.y = 0;
	
	gs.sq.x = 128;
	gs.sq.y = 128;
	
	gs.active = RECT;

	int key_pressed = 0;	
	
	while(1){
		
		
		/////////////////////////////////////
		// Poll controls.
		int mov_x = 0;
		int mov_y = 0;
		if(joypad.down){
			mov_y = +1;
		}
		if(joypad.up){
			mov_y = -1;
		}
		if(joypad.right){
			mov_x = +1;
		}
		if(joypad.left){
			mov_x = -1;
		}
		//TODO Have bug here. Hold right button and play with A button.
		int toggle_active = joypad.a;
		
		
		
		
		
		
		/////////////////////////////////////
		// Gameplay.
		
		switch(gs.active){
		case RECT:
			//TODO Limit not to go through wall. Same for all players.
			if(gs.rect.x + mov_x*STEP < 0)
				gs.rect.x = 0;
			else if(gs.rect.x + mov_x*STEP < SCREEN_W - RECT_W)
				gs.rect.x += mov_x*STEP;
			else
				gs.rect.x = SCREEN_W - RECT_W;
			if(gs.rect.y + mov_y*STEP < 0)
				gs.rect.y = 0;
			else if(gs.rect.y + mov_y*STEP < SCREEN_H - RECT_H)
				gs.rect.y += mov_y*STEP;
			else
				gs.rect.y = SCREEN_H - RECT_H;

			//printf("%i %i\n", gs.rect.x, gs.rect.y);

			if(toggle_active - key_pressed == 1){
				gs.active = SQ;
				key_pressed = 1;
			}
			else if(toggle_active - key_pressed == -1)
				key_pressed = 0;
			break;
		case SQ:
			if(gs.sq.x + mov_x*STEP < 0)
				gs.sq.x = 0;
			else if(gs.sq.x + mov_x*STEP < SCREEN_W - SQ_A)
				gs.sq.x += mov_x*STEP;
			else
				gs.sq.x = SCREEN_W - SQ_A;
			if(gs.sq.y + mov_y*STEP < 0)
				gs.sq.y = 0;
			else if(gs.sq.y + mov_y*STEP < SCREEN_H - SQ_A)
				gs.sq.y += mov_y*STEP;
			else
				gs.sq.y = SCREEN_H - SQ_A;

			if(toggle_active - key_pressed == 1){
				gs.active = RECT;
				key_pressed = 1;
			}
			else if(toggle_active - key_pressed == -1)
				key_pressed = 0;
			break;
		}
		
		
		
		/////////////////////////////////////
		// Drawing.
		
		
		// Detecting rising edge of VSync.
		WAIT_UNITL_0(gpu_p32[2]);
		WAIT_UNITL_1(gpu_p32[2]);
		// Draw in buffer while it is in VSync.
		
		
		
#if !UNPACKED_0_PACKED_1
		// Unpacked.
		
		// Clear to blue.
		for(int r = 0; r < SCREEN_H; r++){
			for(int c = 0; c < SCREEN_W; c++){
				unpack_idx1_p32[r*SCREEN_W + c] = 0;
			}
		}
		
		
		
		// Red rectangle.
		// Use array with 2D indexing.
		for(int r = gs.rect.y; r < gs.rect.y+RECT_H; r++){
			for(int c = gs.rect.x; c < gs.rect.x+RECT_W; c++){
				unpack_idx1_p32[r*SCREEN_W + c] = 1;
			}
		}
		
		
		
		// Red sqaure.
		// Use struct with 2D matrix.
		//for(int r = gs.sq.y; r < gs.sq.y+SQ_A; r++){
		//	for(int c = gs.sq.x; c < gs.sq.x+SQ_A; c++){
		//		unpack_idx1.m[r][c] = 1;
		//	}
		//}
		
		int center[2] = { gs.sq.x + SQ_A / 2, gs.sq.y + SQ_A / 2 };
		int radius = SQ_A / 2;
		for(int r = gs.sq.y; r < gs.sq.y + SQ_A; r++)
		{
			for(int c = gs.sq.x; c < gs.sq.x + SQ_A; c++)
			{
				float dist = sqrt((center[0] - c)*(center[0] - c) + (center[1] - r)*(center[1] - r));
				if((int)dist < radius) 
					unpack_idx1.m[r][c] = 1;
				else
					unpack_idx1.m[r][c] = 0;
			}
		}
		
		
#else
		// Packed.
		// Clear to blue.
		for(int r = 0; r < SCREEN_H; r++){
			for(int c = 0; c < SCREEN_W/32; c++){
				pack_idx1_p32[r*SCREEN_W/32 + c] = 0;
			}
		}

		int center[2] = { gs.sq.x + SQ_A / 4, gs.sq.y + SQ_A / 4 };
		int radius = SQ_A / 4;


		//A serious bug here!
		//Game design is out of the question.
		//TODO This is just test. Implement same as for unpacked.
		for(int r = gs.sq.y; r < gs.sq.y + SQ_A; r++){
			for(int c = gs.sq.x / 32; c < (gs.sq.x + SQ_A) / 32; c++){
				if(r == gs.sq.y + SQ_A/2 && c > (gs.sq.x + SQ_A / 2) / 32)
					pack_idx1_p32[r*(SCREEN_W/32) + c] = 0;
				else
					pack_idx1_p32[r*(SCREEN_W/32) + c] = 0xfffffff0;
			}
		}
		
		
#endif
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
