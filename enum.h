#pragma once

typedef unsigned __int64 bitboard;
typedef unsigned __int64 zobrist;
typedef unsigned char row;
typedef unsigned char rank;
typedef unsigned char file;
typedef unsigned char byte;
typedef int turn;
typedef int color;
typedef int square;
typedef int type;
typedef int location;

const int MAX_MOVES = 128;
const bool CAPTURES_ONLY = true;
const bool SIMPLE = true;
const int GOOD_CAPTURE = 2000;
const int KILLER_TABLE_ALPHA = 15000;
const int KILLER_TABLE_BETA = 18000;
const int HASH_TABLE_MOVE_BONUS = 20000;
const int TRANSPOSITION_TABLE_SIZE = 1000000;
const int VAL_UNKNOWN = -666;

enum
{
	UNDEFINED = -1
};

enum colors
{
    BLACK,
	WHITE
};

enum sides
{
	LEFT,
	RIGHT
};

enum catles
{
	QUEEN_SIDE,
	KING_SIDE
};

enum
{
    MATE = 999999,
	STALEMATE = 0
};

enum
{
	INFINITY = 999999999
};

enum pieces
{
    PAWN,
	KNIGHT,
	BISHOP,
	ROOK,
	QUEEN,
	KING,
	ALL
};

enum setup_pieces
{
	EMPTY=0,
    WP,BP,
	WN,BN,
	WB,BB,
	WR,BR,
	WQ,BQ,
	WK,BK
};

enum piece_coords
{
	A8,B8,C8,D8,E8,F8,G8,H8,
	A7,B7,C7,D7,E7,F7,G7,H7,
	A6,B6,C6,D6,E6,F6,G6,H6,
	A5,B5,C5,D5,E5,F5,G5,H5,
	A4,B4,C4,D4,E4,F4,G4,H4,
	A3,B3,C3,D3,E3,F3,G3,H3,
	A2,B2,C2,D2,E2,F2,G2,H2,
	A1,B1,C1,D1,E1,F1,G1,H1
};

enum files
{
	FILEA,
	FILEB,
	FILEC,
	FILED,
	FILEE,
	FILEF,
	FILEG,
	FILEH,
};

enum ranks
{
	RANK1,
	RANK2,
	RANK3,
	RANK4,
	RANK5,
	RANK6,
	RANK7,
	RANK8
};
