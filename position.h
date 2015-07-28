#pragma once
#include "enum.h"
#include "move.h"
#include <string>
#include <stack>

class position
{
    public:

	//piece info
	bitboard Pieces[ 2 ][ 6 ]; //the master bitboard of all pieces
	bitboard AllPieces[ 2 ]; //bitboards for all pieces for each side
	square PieceIds[ 2 ][ 64 ]; //shows which piece is on which square
	bitboard PiecesRotate90; //the bitboard of all pieces rotated by 90 for rook move generation
	
	//attack info
	int PieceCount[ 3 ]; //how many pieces total there are
	int Attacks[ 2 ][ 7 ][ 64 ]; //shows how many of each piece attack the square
	bitboard AttackBitboards[ 2 ][ 7 ]; //shows where different pieces attack
	bitboard MoveBitboards[ 2 ][ 7 ]; //shows where different pieces can go
	bool CanCastle[ 2 ][ 2 ]; //shows whether the two sides can castle
	int Castled[ 2 ]; //shows how the two sides have castled
	bitboard EnPassant; //en passant bitboard
	color Turn; //the current turn
	bool EndGame; //endgame boolean

	//moves
	void MakeMove( move& Move );
	void TakeBack();
	void MakeNullMove();
	void TakeBackNullMove();
	std::stack<move> Moves;

	//check if in check
	bool IsInCheck( color Color );

	//transposition table hash
	zobrist Hash();
	
	//board notation
	void LoadFEN( std::string FEN );
	std::string GetFEN();

	//evaluation
	int Evaluate();
	int EvaluateMaterial();

	//board util
	void GetBoardArray( int* Board );
	bitboard GetAllPieces();
	bitboard GetEnemyPieces( color Color );
	bitboard GetOwnPieces( color Color );
	void GetPieceIds();
	void Show();

	//constructor
	position();
};

#define    HashFlagEXACT   0
#define    HashFlagALPHA   1
#define    HashFlagBETA    2

class pos_info {
    public:
	zobrist Key;
	int Depth;
	int Flags;
	int Val;
	move Best;
};
