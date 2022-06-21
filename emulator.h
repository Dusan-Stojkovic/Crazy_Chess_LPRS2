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
#define joy_bits (*((volatile uint8_t*)LPRS2_JOYPAD_BASE))

typedef struct {
	uint32_t m[SCREEN_IDX1_H][SCREEN_IDX1_W];
} bf_unpack_idx1;
#define unpack_idx1 (*((volatile bf_unpack_idx1*)unpack_idx1_p32))

uint8_t keyboard_input()
{
	static uint8_t shadow = 0;

	auto int update_key_state(int, int);
	int update_key_state(int key, int shadow_key)
	{
		//Start
		if(key == 0 && shadow_key == 0)
		{
			return 0;
		}
		//faling edge
		else if(key == 0 && shadow_key == 1)
		{
			return 1;
		}
		//rising edge
		else if(key == 1 && shadow_key == 0)
		{
			return 2;
		}
		//keep 
		return 3;
	}

	uint8_t toggle = 0;
	uint8_t update = 0;
	uint8_t mask = 0x01;
	for(int i = 0; i < 4; i++)
	{
		update = update_key_state((joy_bits & mask) >> i, (shadow & mask) >> i);
		if(update == 2)
		{
			shadow |= mask;	
		}
		else if(update == 1)
		{
			toggle |= mask;
			shadow &= ~mask;	
		}
		mask = mask << 1;
	}
	return toggle;
}

void draw_background()
{
	// Blue background.
	for(uint16_t r = 0; r < SCREEN_RGB333_H; r++){
		for(uint16_t c = 0; c < SCREEN_RGB333_W; c++){
			unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = 0xf00;
		}
	}
}

//old code
void draw_chessboard(uint16_t color)
{
	// Draw black/white checkboard
	for(uint16_t r = CHESSBOARD_OFFSET_Y; r < CHESSBOARD + CHESSBOARD_OFFSET_Y; r++)
	{
		if((r/10) % 2 == 0)
			color = 0xfff;
		else
			color = 0x0;
		for(uint16_t c = CHESSBOARD_OFFSET_X; c < CHESSBOARD + CHESSBOARD_OFFSET_X; c++)
		{
			if((c / 10) % 2 == 0)
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = color;
			else
				unpack_rgb333_p32[r*SCREEN_RGB333_W + c] = ~color;
		}
	}
}

void draw_player_cursor(point_t p, uint16_t color)
{
	// Draw cursor
	for(uint16_t r = p.y; r < p.y + SQ_A; r++){
		for(uint16_t c = p.x; c < p.x + SQ_A; c++){
			if((r - p.y) < 1 || (c - p.x) < 1 || (r - p.y) > 8 || (c - p.x) > 8)
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
	uint16_t atlas_y,
	uint16_t atlas_w
) {
	//printf("START of print \n");
	for(uint16_t i = atlas_y; i < atlas_y + src_h; i++){
		for(uint16_t j = atlas_x; j < atlas_x + src_w; j++){
			uint16_t pixels = src_p[i*atlas_w + j];
			//printf("%04x ", pixels);
			if(pixels != 0x39)
				unpack_rgb333_p32[(i + dst_y - atlas_y)*SCREEN_RGB333_W + j + dst_x - atlas_x] = pixels;
		}
		//printf("\n");
	}
}

#endif
