#include "position.h"
#include "data.h"
#include "enum.h"
#include "util.h"

int position::EvaluateMaterial()
{
	int Scores[ 2 ] = { 0, 0 };
	int WhiteKingLocation = LSBi( Pieces[ WHITE ][ KING ] );
	int BlackKingLocation = LSBi( Pieces[ BLACK ][ KING ] );

	for( int Location=0; Location<64; ++Location ){
		int Type = PieceIds[ BLACK ][ Location ]; 
		if( Type != UNDEFINED ){
			Scores[ BLACK ] += PieceValues[ Type ] * 100;
			Scores[ BLACK ] += PieceSquareTable[ Type ][ FlipMap[ Location ] ];
			
			Scores[ BLACK ] -= ( 
				Attacks[ WHITE ][ PAWN ]  [ Location ]*4 + Attacks[ WHITE ][ KNIGHT ][ Location ]*3 +
				Attacks[ WHITE ][ BISHOP ][ Location ]*3 + Attacks[ WHITE ][ ROOK ][ Location ]*2 +
				Attacks[ WHITE ][ KNIGHT ][ Location ]*2 + Attacks[ WHITE ][ KNIGHT ][ Location ]
			);

			Scores[ BLACK ] += ( 
				Attacks[ BLACK ][ PAWN ]  [ Location ]*4 + Attacks[ BLACK ][ KNIGHT ][ Location ]*3 +
				Attacks[ BLACK ][ BISHOP ][ Location ]*3 + Attacks[ BLACK ][ ROOK ][ Location ]*2 +
				Attacks[ BLACK ][ KNIGHT ][ Location ]*2 + Attacks[ BLACK ][ KNIGHT ][ Location ]
			);

			Scores[ BLACK ] += (14-Distance( Location, WhiteKingLocation ))/2;
		}
	}

	for( int Location=0; Location<64; ++Location ){
		int Type = PieceIds[ WHITE ][ Location ]; 
		if( Type != UNDEFINED ){
			Scores[ WHITE ] += PieceValues[ Type ] * 100;
			Scores[ WHITE ] += PieceSquareTable[ Type ][ Location ];

			Scores[ WHITE ] += ( 
				Attacks[ WHITE ][ PAWN ]  [ Location ]*4 + Attacks[ WHITE ][ KNIGHT ][ Location ]*3 +
				Attacks[ WHITE ][ BISHOP ][ Location ]*3 + Attacks[ WHITE ][ ROOK ][ Location ]*2 +
				Attacks[ WHITE ][ KNIGHT ][ Location ]*2 + Attacks[ WHITE ][ KNIGHT ][ Location ]
			);

			Scores[ WHITE ] -= ( 
				Attacks[ BLACK ][ PAWN ]  [ Location ]*4 + Attacks[ BLACK ][ KNIGHT ][ Location ]*3 +
				Attacks[ BLACK ][ BISHOP ][ Location ]*3 + Attacks[ BLACK ][ ROOK ][ Location ]*2 +
				Attacks[ BLACK ][ KNIGHT ][ Location ]*2 + Attacks[ BLACK ][ KNIGHT ][ Location ]
			);

			Scores[ WHITE ] += (14-Distance( Location, BlackKingLocation ))/2;
		}
	}

	return ( Turn == WHITE ) ? Scores[ WHITE ] - Scores[ BLACK ] : Scores[ BLACK ] - Scores[ WHITE ];
}

int position::Evaluate()
{
	int Score = 0;

	Score += EvaluateMaterial();

	return Score;
}

int engine::SEE( move& Move, int KillType )
{
	int Outcome = 0;
	int LowestPiece[ 2 ] = { PAWN, PAWN };
	int PiecesSpent[ 2 ] = { 0, 0 };
	int OwnColor = Position.Turn;
	int NmeColor = OtherColor[ Position.Turn ];
	int TypeOnAttackSquare = KillType;
	int Values[ 2 ] = {0 ,0};
	byte TotalAttacks[ 2 ] = { Position.Attacks[ BLACK ][ ALL ][ Move.To ], Position.Attacks[ WHITE ][ ALL ][ Move.To ] };

	int Turn = OwnColor;
	while( Position.Attacks[ OwnColor ][ LowestPiece[ OwnColor ] ][ Move.To ] == 0 ) LowestPiece[ OwnColor ]++;
	while( Position.Attacks[ NmeColor ][ LowestPiece[ NmeColor ] ][ Move.To ] == 0 ) LowestPiece[ NmeColor ]++;

	while( TotalAttacks[ WHITE ] && TotalAttacks[ BLACK ] ){

		Values[ Turn ] += (TypeOnAttackSquare==-1) ? 0 : PieceValues[ TypeOnAttackSquare ] * 100;
		TypeOnAttackSquare = LowestPiece[ Turn ];
		TotalAttacks[ OwnColor ]--;

		PiecesSpent[ Turn ]++;
		if( Position.Attacks[ Turn ][ LowestPiece[ Turn ] ][ Move.To ] == PiecesSpent[ Turn ] ){
			PiecesSpent[ Turn ] = 0;
			LowestPiece[ Turn ]++;
			if( LowestPiece[ Turn ] > KING ) break;
		}

		Turn = OtherColor[ Turn ];
	}

	return OwnColor == WHITE ? Values[ WHITE ] - Values[ BLACK ] : Values[ BLACK ] - Values[ WHITE ];
}