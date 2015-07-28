#include "engine.h"

using namespace std;

int engine::ProbeHash(int Depth, int Alpha, int Beta, vector<move>& Line)
{
	bitboard Key = Position.Hash();
	pos_info* phashe = &TranspositionTable[ Key % zobrist(TRANSPOSITION_TABLE_SIZE) ];

	if( phashe->Key == Key ) {

		//Line[ SearchDepth - Depth  ] = phashe->Best;
		if( !phashe->Best.IsValid() ) return VAL_UNKNOWN;

		if (phashe->Depth >= Depth){

			Line[ HorizonDepth - Depth  ] = phashe->Best;

			if (phashe->Flags == HashFlagEXACT)
				return phashe->Val;

			if ((phashe->Flags == HashFlagALPHA) && (phashe->Val <= Alpha))
				return Alpha;

			if ((phashe->Flags == HashFlagBETA) && (phashe->Val >= Beta))
				return Beta;

		}else{
			return VAL_UNKNOWN;
		}

		//RememberBestMove();
	}
	return VAL_UNKNOWN;
}



void engine::RecordHash(int Depth, int Val, int Flags, move& Move)
{
	if( !Move.IsValid() ) return;
	bitboard Key = Position.Hash();
	pos_info* phashe = &TranspositionTable[ Key % zobrist(TRANSPOSITION_TABLE_SIZE) ];
	phashe->Key = Key;
	phashe->Val = Val;
	phashe->Flags = Flags;
	phashe->Depth = Depth;
	phashe->Best = Move;
}

move engine::GetTableMove(){
	bitboard Key = Position.Hash();
	pos_info* phashe = &TranspositionTable[ Key % zobrist(TRANSPOSITION_TABLE_SIZE) ];
	if( phashe->Key == Key ) return phashe->Best;
	return move();
}


void engine::DeleteBoardFromTable(){
	bitboard Key = Position.Hash();
	TranspositionTable[ Key % zobrist(TRANSPOSITION_TABLE_SIZE) ] = pos_info();
}
