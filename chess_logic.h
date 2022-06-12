#ifndef __CHESS_LOGIC_H__
#define __CHESS_LOGIC_H__

#include<stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// Game config.

#define SQ_A 10
#define CHESSBOARD 80
#define STEP 10
#define PIECE_NUM 32
#define CHESSBOARD_OFFSET_X 10
#define CHESSBOARD_OFFSET_Y 30

///////////////////////////////////////////////////////////////////////////////
// Game data structures.

typedef enum {
	KING,
	QUEEN,
	ROOK,
	BISHOP,
	KNIGHT,
	PAWN
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
	piece_type p;
	point_t pos;
	color_type c;
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

int overlap_piece(int select, chess_piece_t* chesspieces, int piece_index)
{
	if(select == 0 && piece_index > -1)
	{
		for(uint8_t i = 0; i < PIECE_NUM / 2; i++)
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

int pickup_piece(int select, chess_piece_t* chesspieces, point_t* p, int *piece_index)
{
	if(select && *piece_index == -1)
	{
		for(uint8_t i = 0; i < PIECE_NUM / 2; i++)
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

///////////////////////////////////////////////////////////////////////////////
// Game validation.

int validate_king(game_state_t* gs, chess_piece_t piece)
{
	return 0;
}

int validate_pawn(game_state_t* gs, chess_piece_t piece, point_t pos_prev)
{
	int overlap_flag = 0;
	if(piece.c == WHITE)
	{
		//Test for moving up!
		if(piece.pos.y == CHESSBOARD_OFFSET_Y + 10)
		{
			//Pawn can go two squares up now
			//if(piece->pos.x == piece->pos
		}

		//Test for captures
	}
	else
	{

	}
	return 0;
}

int validate_line(game_state_t* gs, chess_piece_t piece)
{
	return 0;
}

int validate_diagonals(game_state_t* gs, chess_piece_t piece)
{
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Init game_state.

void init_chess_piece(chess_piece_t* piece, color_type c, piece_type t, uint8_t atlas_x, uint8_t atlas_y, uint8_t pos_x, uint8_t pos_y)
{
	piece->p = t;
	piece->pos.x = pos_x;
	piece->pos.y = pos_y;
	piece->atlas.x = atlas_x;
	piece->atlas.y = atlas_y;
	piece->c = c;
}

game_state_t* setup_game()
{
	chess_piece_t init_pieces[PIECE_NUM];
	game_state_t* gs;
	gs = (game_state_t*) calloc(1, sizeof(game_state_t));
	gs->white_pieces = (chess_piece_t*) calloc(16, sizeof(chess_piece_t));
	gs->black_pieces = (chess_piece_t*) calloc(16, sizeof(chess_piece_t));
	gs->color = 0xfff; // for white

	gs->p1.x = 80;
	gs->p1.y = 100;

	gs->p2.x = 80;
	gs->p2.y = 10;


	//Init chesspieces
	//White pieces
	init_chess_piece(&(gs->black_pieces[0]), BLACK, KING, 0, 0, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[1]), BLACK, QUEEN, 10, 0, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[2]), BLACK, ROOK, 20, 0, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[3]), BLACK, ROOK, 20, 0, CHESSBOARD_OFFSET_X + CHESSBOARD - 10, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[4]), BLACK, BISHOP, 30, 0, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[5]), BLACK, BISHOP, 30, 0, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[6]), BLACK, KNIGHT, 40, 0, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[7]), BLACK, KNIGHT, 40, 0, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y);
	init_chess_piece(&(gs->black_pieces[8]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[9]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[10]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[11]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[12]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[13]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[14]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + 10);
	init_chess_piece(&(gs->black_pieces[15]), BLACK, PAWN, 50, 0, CHESSBOARD_OFFSET_X + 70, CHESSBOARD_OFFSET_Y + 10);
	//Black pieces
	init_chess_piece(&(gs->white_pieces[0]), WHITE, KING, 0, 10, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[1]), WHITE, QUEEN, 10, 10, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[2]), WHITE, ROOK, 20, 10, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[3]), WHITE, ROOK, 20, 10, CHESSBOARD_OFFSET_X + CHESSBOARD - 10, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[4]), WHITE, BISHOP, 30, 10, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[5]), WHITE, BISHOP, 30, 10, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[6]), WHITE, KNIGHT, 40, 10, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[7]), WHITE, KNIGHT, 40, 10, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + CHESSBOARD - 10);
	init_chess_piece(&(gs->white_pieces[8]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[9]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 10, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[10]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 20, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[11]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 30, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[12]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 40, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[13]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 50, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[14]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 60, CHESSBOARD_OFFSET_Y + 60);
	init_chess_piece(&(gs->white_pieces[15]), WHITE, PAWN, 50, 10, CHESSBOARD_OFFSET_X + 70, CHESSBOARD_OFFSET_Y + 60);

	return gs;
}

#endif
