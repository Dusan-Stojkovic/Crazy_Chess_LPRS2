#ifndef __EMULATOR_H__
#define __EMULATOR_H__

///////////////////////////////////////////////////////////////////////////////
// HW stuff.

#define WAIT_UNITL_0(x) while(x != 0){}
#define WAIT_UNITL_1(x) while(x != 1){}

#define SCREEN_IDX1_W 640
#define SCREEN_IDX1_H 480
#define SCREEN_IDX4_W 320
#define SCREEN_IDX4_H 240
#define SCREEN_RGB333_W 160
#define SCREEN_RGB333_H 120

#define SCREEN_IDX4_W8 (SCREEN_IDX4_W/8)

#define gpu_p32 ((volatile uint32_t*)LPRS2_GPU_BASE)
#define palette_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x1000))
#define unpack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x400000))
#define pack_idx1_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x600000))
#define unpack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0x800000))
#define pack_idx4_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xa00000))
#define unpack_rgb333_p32 ((volatile uint32_t*)(LPRS2_GPU_BASE+0xc00000))
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
	uint32_t m[SCREEN_IDX1_H][SCREEN_IDX1_W];
} bf_unpack_idx1;
#define unpack_idx1 (*((volatile bf_unpack_idx1*)unpack_idx1_p32))

void draw_background()
{
	// Blue background.
	for(uint16_t r = 0; r < SCREEN_RGB333_H; r++){
		for(uint16_t c = 0; c < SCREEN_RGB333_W; c++){
			unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = 0xf00;
		}
	}
}

void draw_chessboard(game_state_t* gs)
{
	// Draw black/white checkboard
	for(uint16_t r = gs->chessboard_offset[1]; r < CHESSBOARD + gs->chessboard_offset[1]; r++)
	{
		if((r/10) % 2 == 0)
			gs->color = 0xfff;
		else
			gs->color = 0x0;
		for(uint16_t c = gs->chessboard_offset[0]; c < CHESSBOARD + gs->chessboard_offset[0]; c++)
		{
			if((c / 10) % 2 == 0)
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = gs->color;
			else
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = ~(gs->color);
		}
	}
}

void draw_player_cursor(game_state_t* gs, uint16_t color)
{
	// Draw cursor
	for(uint16_t r = gs->p1.y; r < gs->p1.y + SQ_A; r++){
		for(uint16_t c = gs->p1.x; c < gs->p1.x + SQ_A; c++){
			if((r-gs->p1.y) < 1 || (c-gs->p1.x) < 1 || (r-gs->p1.y) > 8 || (c-gs->p1.x) > 8)
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = color;
		}
	}

}

void draw_sprite(
	uint16_t* src_p,
	uint16_t src_w,
	uint16_t src_h,
	uint16_t dst_x,
	uint16_t dst_y,
	uint16_t atlas_x,
	uint16_t atlas_y
) {
	//printf("START of print \n");
	for(uint16_t i = atlas_y; i < atlas_y + src_h; i++){
		for(uint16_t j = atlas_x; j < atlas_x + src_w; j++){
			uint16_t pixels = src_p[i*chess_sprites__w + j];
			//printf("%04x ", pixels);
			if(pixels != 0x39)
				unpack_rgb333_p32[(i + dst_y - atlas_y)*SCREEN_RGB333_W + j + dst_x - atlas_x] = pixels;
		}
		//printf("\n");
	}
}

#endif
