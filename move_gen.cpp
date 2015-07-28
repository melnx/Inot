#include "engine.h"
#include "data.h"
#include "util.h"
#include "move.h"
#include <vector>
#include <algorithm>

using namespace std;

void engine::GenMoves( vector<move>& Moves, int& C, bool CapturesOnly )
{
	int OwnColor = Position.Turn;
	int EnemyColor = OtherColor( OwnColor );
	C = 0;
	
	//GenAllAttackBitboards( OwnColor );
	GenAllAttackBitboards( EnemyColor );
	move HashTableMove = GetTableMove();

	for( square Location = 0; Location<64; ++Location ){
		int Type;
		if( (Type = Position.PieceIds[ OwnColor ][ Location ]) != UNDEFINED ){
			bitboard MovesBitboard = GenMoveBitboard( OwnColor, Type, Location );
			if( CapturesOnly ) MovesBitboard &= Position.AllPieces[ EnemyColor ];

			for( int MoveTo=0; MoveTo<64; ++MoveTo ){
				if( MovesBitboard & Bitmask(MoveTo) ){

					int Value = 0;

					int Capture = Position.PieceIds[ EnemyColor ][ MoveTo ];
					if( Capture != UNDEFINED ){
						Value += (PieceValues[ Capture ] * 900 - PieceValues[ Type ] * 100) + 2000;
					}
					
					if( Position.Turn == WHITE ){
					    Value += PieceSquareTable[ Type ][ MoveTo ] - PieceSquareTable[ Type ][ Location ];
					}else{
						Value += PieceSquareTable[ Type ][ FlipMap[ MoveTo ] ] - PieceSquareTable[ Type ][ FlipMap[ Location ] ];
					}

					Value += ( 
						Position.Attacks[ OwnColor ][ PAWN ][ MoveTo ]*4 + Position.Attacks[ OwnColor ][ KNIGHT ][ MoveTo ]*3 +
						Position.Attacks[ OwnColor ][ BISHOP ][ MoveTo ]*3 + Position.Attacks[ OwnColor ][ ROOK ][ MoveTo ]*2 +
						Position.Attacks[ OwnColor ][ KNIGHT ][ MoveTo ]*2 + Position.Attacks[ OwnColor ][ KNIGHT ][ MoveTo ]
					);

					Value -= ( 
						Position.Attacks[ EnemyColor ][ PAWN ][ MoveTo ]*4 + Position.Attacks[ EnemyColor ][ KNIGHT ][ MoveTo ]*3 +
						Position.Attacks[ EnemyColor ][ BISHOP ][ MoveTo ]*3 + Position.Attacks[ EnemyColor ][ ROOK ][ MoveTo ]*2 +
						Position.Attacks[ EnemyColor ][ KNIGHT ][ MoveTo ]*2 + Position.Attacks[ EnemyColor ][ KNIGHT ][ MoveTo ]
					);

					//Value += ((7-abs(MoveTo%8-KingCoords[ NmeColor ][ 0 ]))+
					//	(7-abs(MoveTo/8-KingCoords[ NmeColor ][ 1 ])));

					Moves[ C ] = move( OwnColor, Type, Location, MoveTo, Value );

					//Moves[ C ].Value += SEE( Moves[ C ], Capture );

					//if it's a move found in the hash table
					if( Moves[ C ] == HashTableMove ) Moves[ C ].Value += HASH_TABLE_MOVE_BONUS;

					//promotion
					if( Type == PAWN && (MoveTo > 56 || MoveTo < 8) ){
						Moves[ C ].Promotion = BISHOP;
						Moves[ C ].Value += 10000;
						C++;
						Moves[ C ] = Moves[ C-1 ];
						Moves[ C ].Promotion = KNIGHT;
						C++;
						Moves[ C ] = Moves[ C-1 ];
						Moves[ C ].Promotion = ROOK;
						Moves[ C ].Value += 10000;
						C++;
						Moves[ C ] = Moves[ C-1 ];
						Moves[ C ].Promotion = QUEEN;
						Moves[ C ].Value += 10000;
					}

					C++;
				}
			}

		}
	}

	//sort the moves
	sort( Moves.begin(), Moves.end() );
}