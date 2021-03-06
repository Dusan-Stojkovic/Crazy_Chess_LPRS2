#ifndef __CHESS_LOGIC_H__
#define __CHESS_LOGIC_H__

#include<stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// Game config.

#define SQ_A 10
#define CHESSBOARD 80
#define STEP 10
#define PIECE_NUM_MAX 92
#define PIECE_NUM 32
#define CHESSBOARD_OFFSET_X 10
#define CHESSBOARD_OFFSET_Y 30
#define SPAWN_POOL 100
#define POOL_SIZE 5

///////////////////////////////////////////////////////////////////////////////
// Game data structures.

typedef enum {
	KING = 10,
	QUEEN = 5,
	ROOK = 4,
	BISHOP = 3,
	KNIGHT = 2,
	PAWN = 1
} piece_type;

typedef enum {
	WHITE = 1,
	BLACK = -1
} color_type;

typedef struct {
	uint16_t x;
	uint16_t y;
} point_t;

typedef struct {
	point_t atlas;
	piece_type t;
	point_t pos;
	color_type c;
	int h;
} chess_piece_t;

typedef struct {
	point_t p1;
	point_t p2;
	chess_piece_t* white_pieces;
	chess_piece_t* black_pieces;
	int color;
} game_state_t;

///////////////////////////////////////////////////////////////////////////////
// State update

void update_cursor(point_t* p, int mov_x, int mov_y)
{
	if(p->x + mov_x*STEP < CHESSBOARD_OFFSET_X)
	{
		p->x = CHESSBOARD_OFFSET_X;
	}
	else if(p->x + mov_x*STEP < CHESSBOARD_OFFSET_X + CHESSBOARD - SQ_A)
	{
		p->x += mov_x*STEP;
	}
	else
	{
		p->x = CHESSBOARD_OFFSET_X + CHESSBOARD - SQ_A;
	}
	if(p->y + mov_y*STEP < CHESSBOARD_OFFSET_Y)
	{
		p->y = CHESSBOARD_OFFSET_Y;
	}
	else if(p->y + mov_y*STEP < CHESSBOARD_OFFSET_Y + CHESSBOARD - SQ_A)
	{
		p->y += mov_y*STEP;
	}
	else
	{
		p->y = CHESSBOARD_OFFSET_Y + CHESSBOARD - SQ_A;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Game validation.

int overlap_piece(int select, chess_piece_t* chesspieces, int piece_num, int piece_index)
{
	if(select == 0 && piece_index > -1)
	{
		for(uint8_t i = 0; i < piece_num; i++)
		{
			if(chesspieces[i].pos.x == chesspieces[piece_index].pos.x && 
					chesspieces[i].pos.y == chesspieces[piece_index].pos.y && 
					i != piece_index)
			{
				return 1;
			}
		}
	}
	return 0;
}

int pickup_piece(int select, chess_piece_t* chesspieces, int piece_num, point_t* p, int *piece_index)
{
	if(select && *piece_index == -1)
	{
		for(uint8_t i = 0; i < piece_num; i++)
		{
			if(chesspieces[i].pos.x == p->x && chesspieces[i].pos.y == p->y)
			{
				*piece_index = i;
				chesspieces[i].pos = *p;
				return 1;		//Mode 1: new piece selected
			}
		}
	}
	if(*piece_index > -1)
	{
		chesspieces[*piece_index].pos.x = p->x;
		chesspieces[*piece_index].pos.y = p->y;
	}
	return 0;					//Mode 0: previous selected piece moved
}

//ret value is score
int piece_combat(chess_piece_t* attacker, chess_piece_t* defender, int attack_i, int* attack_force, int* defense_force)
{
	//attack/defense value are set on piece types
	int score = 0;
	for(int i = 0; i < *defense_force; i++)
	{
		if(attacker[attack_i].pos.x == defender[i].pos.x && attacker[attack_i].pos.y == defender[i].pos.y)
		{
			//TODO figure out how to deallocate p and enemy
			if(attacker[attack_i].h == defender[i].h)
			{
				if(attack_i == 0)
				{
					//The king is dead!
					return -10;
				}
				//deallocate attacker, add score to attacker 
				score = defender[i].t;
				defender[i].h = 1;
				attacker[attack_i] = attacker[*attack_force - 1];
				//attacker[*attack_force - 1] = 0;
				(*attack_force)--;
				return score;
			}
			else if(attacker[attack_i].h - defender[i].h < 0)
			{
				//deallocate attacker, add score to defender
				score = -attacker[attack_i].t;
				defender[i].h -= attacker[attack_i].h;
				attacker[attack_i] = attacker[*attack_force - 1];
				(*attack_force)--;
				return score;
			}
			else
			{
				if(i == 0)
				{
					//The king is dead!
					return 10;
				}
				//deallocate defender, add score to attacker
				score = defender[i].t;
				attacker[attack_i].h -= defender[i].h;
				defender[i] = defender[*defense_force - 1];
				(*defense_force)--;
				return score;
			}
		}
	}

	return score;
}

int spawn_pool(chess_piece_t** pool, color_type c, int *cash)
{
	uint8_t offset = 0;
	uint8_t pool_y = CHESSBOARD_OFFSET_Y + 5; 
	if(c == WHITE)
	{
		offset = 10;	
		pool_y = CHESSBOARD_OFFSET_Y + CHESSBOARD - 15;
	}

	*pool = (chess_piece_t*) calloc(POOL_SIZE, sizeof(chess_piece_t));

	init_chess_piece(&((*pool)[0]), c, PAWN, 1, 50, offset, SPAWN_POOL, pool_y);
	init_chess_piece(&((*pool)[1]), c, KNIGHT, 2, 40, offset, SPAWN_POOL, pool_y);
	init_chess_piece(&((*pool)[2]), c, BISHOP, 3, 30, offset, SPAWN_POOL, pool_y); 
	init_chess_piece(&((*pool)[3]), c, ROOK, 4, 20, offset, SPAWN_POOL, pool_y);
	init_chess_piece(&((*pool)[4]), c, QUEEN, 5, 10, offset, SPAWN_POOL, pool_y);

	int i = 0;
	for(i = 1; i <= QUEEN; i++)
	{
		if(i == *cash)
		{
			return i;
		}
	}
	return i;
}

///////////////////////////////////////////////////////////////////////////////
// Init game_state.

void init_chess_piece(chess_piece_t* piece, color_type c, piece_type t, int health, uint8_t atlas_x, uint8_t atlas_y, uint8_t pos_x, uint8_t pos_y)
{
	piece->t = t;
	piece->pos.x = pos_x;
	piece->pos.y = pos_y;
	piece->atlas.x = atlas_x;
	piece->atlas.y = atlas_y;
	piece->c = c;
	piece->h = health;
}

game_state_t* setup_game()
{
	game_state_t* gs;
	gs = (game_state_t*) calloc(1, sizeof(game_state_t));
	gs->white_pieces = (chess_piece_t*) calloc(PIECE_NUM_MAX / 2, sizeof(chess_piece_t));
	gs->black_pieces = (chess_piece_t*) calloc(PIECE_NUM_MAX / 2, sizeof(chess_piece_t));
	gs->color = 0xfff; // for white

	gs->p1.x = 0;
	gs->p1.y = 0;

	gs->p2.x = 0;
	gs->p2.y = 0;

	//Init chesspieces
	//Black pieces
	init_chess_piece(&(gs->black_pieces[0]), BLACK, KING, 10, 0, 0, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[1]), BLACK, QUEEN, 5, 10, 0, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[2]), BLACK, ROOK, 4, 20, 0, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[3]), BLACK, ROOK, 4, 20, 0, CHESSBOARD_OFFSET_X + CHESSBOARD - 10, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[4]), BLACK, BISHOP, 3, 30, 0, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[5]), BLACK, BISHOP, 3, 30, 0, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[6]), BLACK, KNIGHT, 2, 40, 0, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[7]), BLACK, KNIGHT, 2, 40, 0, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[8]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[9]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[10]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[11]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[12]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[13]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[14]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[15]), BLACK, PAWN, 1, 50, 0, CHESSBOARD_OFFSET_X + 70, CHESSBOARD_OFFSET_Y + 10);

	//White pieces
	init_chess_piece(&(gs->white_pieces[0]), WHITE, KING, 10, 0, 10, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[1]), WHITE, QUEEN, 5, 10, 10, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[2]), WHITE, ROOK, 4, 20, 10, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[3]), WHITE, ROOK, 4, 20, 10, CHESSBOARD_OFFSET_X + CHESSBOARD - 10, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[4]), WHITE, BISHOP, 3, 30, 10, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[5]), WHITE, BISHOP, 3, 30, 10, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[6]), WHITE, KNIGHT, 2, 40, 10, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[7]), WHITE, KNIGHT, 2, 40, 10, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[8]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[9]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[10]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[11]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[12]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[13]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[14]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[15]), WHITE, PAWN, 1, 50, 10, CHESSBOARD_OFFSET_X + 70, CHESSBOARD_OFFSET_Y + 60);

	return gs;
}

#endif
