#pragma once

#include "enum.h"
#include "position.h"
#include "move.h"
#include <string>
#include <fstream>
#include <vector>

class engine
{
    public:

	//position
	position Position;

	//timing info
	int SearchStarted;
	int NodesSearched;
	int SearchTime;
	int GameTime;
	int CompColor;
	int HorizonDepth;
	bool Output;

	//transposition table
	pos_info TranspositionTable[ TRANSPOSITION_TABLE_SIZE ];
	int ProbeHash(int Depth, int Alpha, int Beta, std::vector<move>& Line);
	void RecordHash(int Depth, int Val, int HashF, move& Move);
	move GetTableMove();
	void DeleteBoardFromTable();
	bitboard ProbeMoveStorage( int Color, int Type, int From, bitboard Board );
	void RecordMoveStorage( int Color, int Type, int From, bitboard Board, bitboard MoveBoard );


	//xboard command handler
	void ProcessCommand( std::string Command );
	bool Post;

	//make a move
	bool MakeMove( int From, int To, int Turn, int Promotion );

	//search for a move
	move MakeBestMove( color Color, int Time, bool Output );
	void Ponder( color Color );
	int SEE( move& Move, int KillType );
	int AlphaBeta( int Alpha, int Beta, int RemDepth, std::vector<move>& Line );
	int Quiescent(int Alpha, int Beta);

	//generate moves
	void GenMoves( std::vector<move>& Moves, int& Count, bool CapturesOnly = false );
	
	bitboard GenAllAttackBitboards( int Color );
	bitboard GenMoveBitboard( int Color, int Type, int Location );
	bitboard GenAttackBitoard( int Color, int Type, int Location );
	
	void GenWhitePawnMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves );
	void GenBlackPawnMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves );
	void GenKingMoveBitboard( int Color, int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves, bitboard Attacks );
	void GenRookMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves );
	void GenBishopMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves );
	void GenWhitePawnAttackBitboard( int Location, bitboard& Moves );
	void GenBlackPawnAttackBitboard( int Location, bitboard& Moves );


	//notation handler
	std::string NotateMove( move& Move, position& Position );
	std::string NotateMove( move& Move );

	//handle searching and pondering
	void StartPonder();
	void TerminatePonder();
	void StartSearch();
	void TerminateSearch();
	bool SearchTerminated;
	bool Searching;

	//test suite
	void TestSuite( std::string Path, int Time );

	//logging
	std::ofstream lout;

	//constructor
	engine( std::string FEN );
	engine();

	//util
	std::string ShowPieceIds();
};