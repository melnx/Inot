#include "engine.h"
#include "data.h"
#include "util.h"

bitboard engine::GenAllAttackBitboards( int Color )
{
	memset( Position.Attacks[ Color ], 0x0, 7*64*sizeof(int) );
	memset( Position.AttackBitboards[ Color ], 0x0, 7*sizeof(bitboard) );

	for(int Location=0; Location<64; Location++){
		int Type = Position.PieceIds[ Color ][ Location ];
		if( Type != UNDEFINED ){
			bitboard Attacks = GenAttackBitoard( Color, Type, Location );
			Position.AttackBitboards[ Color ][ Type ] |= Attacks;
			Position.AttackBitboards[ Color ][ ALL ] |= Attacks;
			Position.Attacks[ Color ][ Type ][ Location ]++;
			Position.Attacks[ Color ][ ALL ][ Location ]++;
		}
	}
	return Position.AttackBitboards[ Color ][ ALL ];
}

bitboard engine::GenAttackBitoard( int Color, int Type, int Location )
{
	bitboard EnemyPieces = Position.AllPieces[ OtherColor( Color ) ];
	bitboard OwnPieces = Position.AllPieces[ Color ];
	bitboard AllPieces = OwnPieces | EnemyPieces;
	bitboard Moves = 0x0;

	switch( Type ){

		case PAWN:
			if( Color == WHITE ){
				GenWhitePawnAttackBitboard( Location, Moves );
			}
			else{
				GenBlackPawnAttackBitboard( Location, Moves );
			}
			break;

		case KNIGHT:
			Moves = KnightMoveBitboards[ Location ];
			break;

		case BISHOP:
			GenBishopMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case ROOK:
			GenRookMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case QUEEN:
			GenRookMoveBitboard( Location, EnemyPieces, AllPieces, Moves ); 
			GenBishopMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case KING:
			Moves = KingMoveBitboards[ Location ];
			break;

	}

	return Moves;
}

bitboard engine::GenMoveBitboard( int Color, int Type, int Location )
{

	bitboard Moves = 0x0;
	int OwnColor = Color;
	int EnemyColor = OtherColor( Color );
	bitboard EnemyPieces = Position.AllPieces[ EnemyColor ];
	bitboard OwnPieces = Position.AllPieces[ OwnColor ];
	bitboard AllPieces = EnemyPieces | OwnPieces;
	bitboard Attacks = Position.AttackBitboards[ EnemyColor ][ ALL ];

	switch( Type ){

		case PAWN:
			if( Color == WHITE ){
				GenWhitePawnMoveBitboard(Location, EnemyPieces, AllPieces, Moves);
			}
			else{
				GenBlackPawnMoveBitboard(Location, EnemyPieces, AllPieces, Moves);
			}
			break;

		case KNIGHT:
			Moves = KnightMoveBitboards[ Location ];
			break;

		case BISHOP:
			GenBishopMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case ROOK:
			GenRookMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case QUEEN:
			GenRookMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			GenBishopMoveBitboard( Location, EnemyPieces, AllPieces, Moves );
			break;

		case KING:
			GenKingMoveBitboard( Color, Location, EnemyPieces, AllPieces, Moves, Attacks );
			break;

	}

	Moves &= (~OwnPieces);
	return Moves;
}

void engine::GenBishopMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves )
{
	int x = Location%8;
	int y = Location/8;

	//calculate the up->down diagonal
	for(int y1=y+1, x1=x+1; y1<8 && x1<8; ++y1, ++x1){
		Moves |= Bitmask( y1*8 + x1 );
		if( AllPieces & Bitmask( y1*8 + x1 ) )
			break;
	}
	for(int y1=y-1, x1=x-1; y1>=0 && x1>=0; --y1, --x1){
		Moves |= Bitmask( y1*8 + x1 );
		if( AllPieces & Bitmask( y1*8 + x1 ) )
			break;
	}

	//calculate the down->up diagonal
	for(int y1=y-1, x1=x+1; y1>=0 && x1<8; --y1, ++x1){
		Moves |= Bitmask( y1*8 + x1 );
		if( AllPieces & Bitmask( y1*8 + x1 ) )
			break;
	}
	for(int y1=y+1, x1=x-1; y1<8 && x1>=0; ++y1, --x1){
		Moves |= Bitmask( y1*8 + x1 );
		if( AllPieces & Bitmask( y1*8 + x1 ) )
			break;
	}
}

void engine::GenRookMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves )
{
	int x = Location%8;
	int y = Location/8;

	//calculate the bytes for row and column
	byte Row = (byte)( AllPieces >> (y*8) );
	byte Col = (byte)( Position.PiecesRotate90 >> (x*8) );

	//calculate horizontal and vertical moves and or them together
	Moves = RookMoves[ x ][ Row ];
	Moves <<= (y*8);
	Moves |= (VerticalRookMoves[ y ][ Col ] << x);
}

void engine::GenWhitePawnMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves )
{
	EnemyPieces |= Position.EnPassant;
	//if( Position.EnPassant ) Showbitboard( Position.EnPassant );
	int x = Location%8;
	int y = Location/8;
	bool L = (x > 0 && y > 0 && ( Bitmask( (x-1) + (y-1)*8 ) & EnemyPieces ) );
	bool M = (y > 0 && ( Bitmask( x + (y-1)*8 ) & AllPieces ) );
	bool R = (x < 7 && y > 0 && ( Bitmask( (x+1) + (y-1)*8 ) & EnemyPieces ) );

	if( !M )
		Moves |= Bitmask( (y-1)*8 + x ) ;
	if( L )
		Moves |= Bitmask( (y-1)*8 + (x-1) );
	if( R )
		Moves |=  Bitmask( (y-1)*8 + (x+1) );
	if( y == 6 && !( AllPieces &  Bitmask( (y-1)*8 + x ) ) && !(AllPieces & Bitmask( (y-2)*8 + x )) )
		Moves |= Bitmask( (y-2)*8 + x );
}

void engine::GenBlackPawnMoveBitboard( int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves )
{
	EnemyPieces |= Position.EnPassant;
	//if( Position.EnPassant ) Showbitboard( Position.EnPassant );
	int x = Location%8;
	int y = Location/8;
	bool L = (x > 0 && y < 7 && ( Bitmask( (x-1) + (y+1)*8 ) & EnemyPieces ) );
	bool M = (y > 0 && ( Bitmask( x + (y+1)*8 ) & AllPieces ) );
	bool R = (x < 7 && y < 7 && ( Bitmask( (x+1) + (y+1)*8 ) & EnemyPieces ) );

	if( !M )
		Moves |= Bitmask( (y+1)*8 + x ) ;
	if( L )
		Moves |= Bitmask( (y+1)*8 + (x-1) );
	if( R )
		Moves |= Bitmask( (y+1)*8 + (x+1) );
	if( y == 1 && !( AllPieces & Bitmask( (y+1)*8 + x ) ) && !(AllPieces & Bitmask( (y+2)*8 + x )) )
		Moves |= Bitmask( (y+2)*8 + x );
}

void engine::GenKingMoveBitboard( int Color, int Location, bitboard EnemyPieces, bitboard AllPieces, bitboard& Moves, bitboard Attacks )
{
	row Row = 
	((Color==BLACK) ? (row)( AllPieces ) : (row)( AllPieces >> 56 )) | 
	((Color==BLACK) ? (row)( Attacks ) : (row)( Attacks >> 56 ));

	if( !( Attacks & Position.Pieces[ Color ][ KING ] ) ){

		if( Position.CanCastle[ Color ][ LEFT ] ){
			if( Castling[ LEFT ][ (int)Row ] ){
				Moves |= Bitmask( Location - 2 );
			}
		}
		if( Position.CanCastle[ Color ][ RIGHT ] ){
			if( Castling[ RIGHT ][ (int)Row ] ){
				Moves |= Bitmask( Location + 2 );
			}
		}
	}

	Moves = ( KingMoveBitboards[ Location ] | Moves ) & (~Attacks);
}

void engine::GenBlackPawnAttackBitboard( int Location, bitboard& Moves )
{
	int x = Location%8;
	int y = Location/8;
	if( y < 7 ){
		if( x>0 ) Moves |= Bitmask( (y+1)*8 + x-1 );
		if( x<7 ) Moves |= Bitmask( (y+1)*8 + x+1 );
	}
}

void engine::GenWhitePawnAttackBitboard( int Location, bitboard& Moves )
{
	int x = Location%8;
	int y = Location/8;
	if( y > 0 ){
		if( x>0 ) Moves |= Bitmask( (y-1)*8 + x-1 );
		if( x<7 ) Moves |= Bitmask( (y-1)*8 + x+1 );
	}
}