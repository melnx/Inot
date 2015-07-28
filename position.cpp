#include "position.h"
#include "util.h"
#include "move.h"
#include "data.h"
#include <string>
#include <iostream>

using namespace std;

position::position()
{
	EnPassant = 0x0;
}

void position::MakeMove( move& Move )
{
	color OwnColor = Move.Color;
	color EnemyColor = OtherColor( OwnColor );

	//move own piece
	Move.PiecesRotate90 = PiecesRotate90;
	Pieces[ OwnColor ][ Move.Type ] ^= Bitmask( Move.From );
	Pieces[ OwnColor ][ Move.Type ] |= Bitmask( Move.To );
	AllPieces[ OwnColor ] ^= Bitmask( Move.From );
	AllPieces[ OwnColor ] |= Bitmask( Move.To );
	PieceIds[ OwnColor ][ Move.From ] = UNDEFINED;
	PieceIds[ OwnColor ][ Move.To ] = Move.Type;
	int CaptureLocation = Move.To;
	
	//see if there is a promotion
	if( Move.Promotion != UNDEFINED ){
		Pieces[ OwnColor ][ PAWN ] ^= Bitmask( Move.To );
		Pieces[ OwnColor ][ Move.Promotion ] |= Bitmask( Move.To );
		PieceIds[ OwnColor ][ Move.To ] = Move.Promotion;
	}

	//take en passant
	if( Move.Type == PAWN ){
		if( abs(Move.To - Move.From)%8 && (Bitmask( Move.To ) & EnPassant) ){
			if( Move.To - Move.From > 0 ){
				CaptureLocation = Move.To-8;
			}else{
				CaptureLocation = Move.To+8;
			}
		}
	}

	//kill enemy piece if there is a capture
	int CaptureType;
	if( (CaptureType = PieceIds[ EnemyColor ][ CaptureLocation ] ) != UNDEFINED ){
		Pieces[ EnemyColor ][ CaptureType ] ^= Bitmask( CaptureLocation );
		AllPieces[ EnemyColor ] ^= Bitmask( CaptureLocation );
		PieceIds[ EnemyColor ][ CaptureLocation ] = UNDEFINED;
		PiecesRotate90 ^= Bitmask( Rotate90Map[ CaptureLocation ] );
		Move.Capture = CaptureType;
		--PieceCount[ EnemyColor ];
	}
	Move.CaptureLocation = CaptureLocation;

	//change the rotate90 bitboard
	PiecesRotate90 ^= Bitmask( Rotate90Map[ Move.From ] );
	PiecesRotate90 |= Bitmask( Rotate90Map[ Move.To ] );

	//memorize castling
	memcpy( Move.CanCastle, CanCastle, 4*sizeof(bool) );

	//memorize en passant
	Move.EnPassant = EnPassant;

	//see if there is en passant
	EnPassant ^= EnPassant;
	if( Move.Type == PAWN ){
		if( Move.To - Move.From == 16 ){
			EnPassant |= Bitmask( Move.To - 8 );
		}else if( Move.To - Move.From == -16 ){
			EnPassant |= Bitmask( Move.To + 8 );
		}
	}

	//see if there is castling
	if( Move.Type == KING ){
		if( Move.To - Move.From  == 2 ){
			Pieces[ OwnColor ][ ROOK ] ^= Bitmask( Move.To+1 );
			Pieces[ OwnColor ][ ROOK ] |= Bitmask( Move.To-1 );
			AllPieces[ OwnColor ] ^= Bitmask( Move.To+1 );
			AllPieces[ OwnColor ] |= Bitmask( Move.To-1 );
			PieceIds[ OwnColor ][ Move.To+1 ] = UNDEFINED;
			PieceIds[ OwnColor ][ Move.To-1 ] = ROOK;
			PiecesRotate90 ^= Bitmask( Rotate90Map[ Move.To+1 ] );
			PiecesRotate90 |= Bitmask( Rotate90Map[ Move.To-1 ] );
		}
		else if( Move.To - Move.From == -2 ){
			Pieces[ OwnColor ][ ROOK ] ^= Bitmask( Move.To-2 );
			Pieces[ OwnColor ][ ROOK ] |= Bitmask( Move.To+1 );
			AllPieces[ OwnColor ] ^= Bitmask( Move.To-2 );
			AllPieces[ OwnColor ] |= Bitmask( Move.To+1 );
			PieceIds[ OwnColor ][ Move.To-2 ] = UNDEFINED;
			PieceIds[ OwnColor ][ Move.To+1 ] = ROOK;
			PiecesRotate90 ^= Bitmask( Rotate90Map[ Move.To-2 ] );
			PiecesRotate90 |= Bitmask( Rotate90Map[ Move.To+1 ] );
		}
		CanCastle[ OwnColor ][ KING_SIDE ] = false;
		CanCastle[ OwnColor ][ QUEEN_SIDE ] = false;
	}
	if( Move.Type == ROOK ){
		if( Move.From % 8 == 0 ){
			CanCastle[ OwnColor ][ QUEEN_SIDE ] = false;
		}else if( Move.From % 8 == 7 ){
			CanCastle[ OwnColor ][ KING_SIDE ] = false;
		}
	}

	Moves.push( Move );
	Turn = OtherColor( Turn );
}

void position::TakeBack()
{
	if( !Moves.size() ) return;

	Turn = OtherColor( Turn );
	color OwnColor = Turn;
	color EnemyColor = OtherColor( Turn );
	move& Move = Moves.top();

	//move back your own piece
	Pieces[ OwnColor ][ (Move.Promotion==UNDEFINED) ? Move.Type : Move.Promotion ] ^= Bitmask( Move.To );
	Pieces[ OwnColor ][ Move.Type ] |= Bitmask( Move.From );
	AllPieces[ OwnColor ] ^= Bitmask( Move.To );
	AllPieces[ OwnColor ] |= Bitmask( Move.From );
	PieceIds[ OwnColor ][ Move.To ] = UNDEFINED;
	PieceIds[ OwnColor ][ Move.From ] = Move.Type;
	PiecesRotate90 = Move.PiecesRotate90;

	//un-capture the piece if there is one captured
	if( Move.Capture != UNDEFINED ){
		Pieces[ EnemyColor ][ Move.Capture ] |= Bitmask( Move.CaptureLocation );
		AllPieces[ EnemyColor ] |= Bitmask( Move.CaptureLocation );
		PieceIds[ EnemyColor ][ Move.CaptureLocation ] = Move.Capture;
	}

	//remember the castling privelages
	memcpy( CanCastle, Move.CanCastle, 4*sizeof(bool) );
	
	//remember en passant
	EnPassant = Move.EnPassant;

	//undo castling
	if( Move.Type == KING ){
		if( Move.To - Move.From  == 2 ){
			Pieces[ OwnColor ][ ROOK ] |= Bitmask( Move.To+1 );
			Pieces[ OwnColor ][ ROOK ] ^= Bitmask( Move.To-1 );
			AllPieces[ OwnColor ] |= Bitmask( Move.To+1 );
			AllPieces[ OwnColor ] ^= Bitmask( Move.To-1 );
			PieceIds[ OwnColor ][ Move.To+1 ] = ROOK;
			PieceIds[ OwnColor ][ Move.To-1 ] = UNDEFINED;
		}
		else if( Move.To - Move.From == -2 ){
			Pieces[ OwnColor ][ ROOK ] |= Bitmask( Move.To-2 );
			Pieces[ OwnColor ][ ROOK ] ^= Bitmask( Move.To+1 );
			AllPieces[ OwnColor ] |= Bitmask( Move.To-2 );
			AllPieces[ OwnColor ] ^= Bitmask( Move.To+1 );
			PieceIds[ OwnColor ][ Move.To-2 ] = ROOK;
			PieceIds[ OwnColor ][ Move.To+1 ] = UNDEFINED;
		}
	}

	Moves.pop();
}

void position::MakeNullMove()
{
	Turn = OtherColor( Turn );
}

void position::TakeBackNullMove()
{
	Turn = OtherColor( Turn );
}

bitboard position::Hash()
{
	bitboard Result = 0x0;

	for( int Color=BLACK; Color<=WHITE; ++Color ){
		for( location Location=0; Location<64; ++Location ){
			if( PieceIds[ Color ][ Location ] != UNDEFINED ){
				Result ^= PieceSquareHashes[ Color ][ PieceIds[ Color ][ Location ] ][ Location ]; 
			}
		}	
	}

	if( Turn == WHITE ){
		Result ^= 0x1020304050607080;
	}

	return Result;
}

bool position::IsInCheck( color Color )
{
	return ( AttackBitboards[ OtherColor(Color) ][ ALL ] & Pieces[ Color ][ KING ] );
}

void position::LoadFEN( string Board )
{
	memset( Pieces, 0x0, 2*6*sizeof(bitboard) );
	int x=0;
	int y=0;

	//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

	cout << "LOADED POSITION: " << Board << endl;
	int Phase = 0;

	for( int i=0; i<Board.size(); i++ )
	{
		int Type = -1;
		int Color = 0;

		if( Board[i] == ' ' ){
			Phase++;
			continue;
		}

		switch(Phase){

			case 0:
				switch( Board[ i ] )
				{
					case '/':
						y++;
						x=0;
						break;

					case 'p':
						Type = PAWN;
						Color = BLACK;
						break;
					case 'n':
						Type = KNIGHT;
						Color = BLACK;
						break;
					case 'b':
						Type = BISHOP;
						Color = BLACK;
						break;
					case 'r':
						Type = ROOK;
						Color = BLACK;
						break;
					case 'q':
						Type = QUEEN;
						Color = BLACK;
						break;
					case 'k':
						Type = KING;
						Color = BLACK;
						break;

					case 'P':
						Type = PAWN;
						Color = WHITE;
						break;
					case 'N':
						Type = KNIGHT;
						Color = WHITE;
						break;
					case 'B':
						Type = BISHOP;
						Color = WHITE;
						break;
					case 'R':
						Type = ROOK;
						Color = WHITE;
						break;
					case 'Q':
						Type = QUEEN;
						Color = WHITE;
						break;
					case 'K':
						Type = KING;
						Color = WHITE;
						break;

					default:
						if( Board[i] < '1' || Board[i] > '8' ) break;
						x+= (int(Board[i]-'0'));
						break;
				}

				if( Type >= 0 ){
					Pieces[ Color ][ Type ] |= Bitmask( x + y*8 );
					x++;
				}
				break;

			case 1:
				if( Board[i] == 'w' ){ Turn = WHITE; }else{ Turn = BLACK; }
				break;

			case 2:
				if( Board[i] == 'Q' ) CanCastle[ WHITE ][ QUEEN_SIDE ] = true;
				if( Board[i] == 'K' ) CanCastle[ WHITE ][ KING_SIDE ] = true;
				if( Board[i] == 'q' ) CanCastle[ BLACK ][ QUEEN_SIDE ] = true;
				if( Board[i] == 'k' ) CanCastle[ BLACK ][ KING_SIDE ] = true;
				break;

			case 3:
				//en passant
				break;

			case 4:
				//full move number
				break;


		}
	}

	PiecesRotate90 = Rotate90( GetAllPieces() );
	GetPieceIds();
	AllPieces[ BLACK ] = GetOwnPieces( BLACK );
	AllPieces[ WHITE ] = GetOwnPieces( WHITE );
	PieceCount[ BLACK ] = Population( AllPieces[ BLACK ] );
	PieceCount[ WHITE ] = Population( AllPieces[ WHITE ] );
}

string position::GetFEN()
{
	string FEN;
	int Board[ 64 ];
	GetBoardArray( Board );

	int NumEmpty = 0;
	for(int i=0; i<64; i++){

		if( (Board[i] != EMPTY || (i && !(i%8))) && NumEmpty ){
			FEN += char('0'+NumEmpty);
			NumEmpty = 0;
		}

		if( i && !(i%8) ){
			FEN += '/';
			NumEmpty = 0;
		}

		switch(Board[ i ]){
			case BP: FEN += 'p'; break;
			case BN: FEN += 'n'; break;
			case BB: FEN += 'b'; break;
			case BR: FEN += 'r'; break;
			case BQ: FEN += 'q'; break;
			case BK: FEN += 'k'; break;
			case WP: FEN += 'P'; break;
			case WN: FEN += 'N'; break;
			case WB: FEN += 'B'; break;
			case WR: FEN += 'R'; break;
			case WQ: FEN += 'Q'; break;
			case WK: FEN += 'K'; break;
			default: NumEmpty++; break;
		}

	}

	if( NumEmpty ) FEN += char('0'+NumEmpty);

	FEN += " ";

	FEN += ( Turn== WHITE ) ? 'w' : 'b';

	FEN += " ";

	FEN += CanCastle[ WHITE ][ KING_SIDE ] ? 'K' : '-';
	FEN += CanCastle[ WHITE ][ QUEEN_SIDE ] ? 'Q' : '-';
	FEN += CanCastle[ BLACK ][ KING_SIDE ] ? 'k' : '-';
	FEN += CanCastle[ BLACK ][ QUEEN_SIDE ] ? 'q' : '-';

	FEN += " ";

	FEN += '-';

	FEN += " ";

	FEN += '0';

	return FEN;
}

void position::GetPieceIds()
{
	for( color Color = BLACK; Color <= WHITE; ++Color ){
		for( square Square = 0; Square < 64; ++Square ){
			PieceIds[ Color ][ Square ] = UNDEFINED;
		}
	}
	
	for( color Color = BLACK; Color <= WHITE; ++Color ){
		for( type Type = PAWN; Type <= KING; ++Type ){
			for( square Square = 0; Square < 64; ++Square ){
				if( Pieces[ Color ][ Type ] & Bitmask( Square ) ){
					PieceIds[ Color ][ Square ] = Type;
				}
			}
		}
	}

}

void position::GetBoardArray( int* Info ){

	for(int i=0; i<64; i++) Info[i] = 0x0;

	for( color Color=BLACK; Color<=WHITE; ++Color ){
		for( type Type=PAWN; Type<=KING; ++Type){
			for( square Location=0; Location<64; ++Location ){
				if( Info[Location] != 0x0 ) continue;
				if( Pieces[ Color ][ Type ] & Bitmask( Location ) ){
					if( Color == WHITE ){
						switch( Type ){
							case PAWN:   Info[ Location ] = WP; break;
							case KNIGHT: Info[ Location ] = WN; break;
							case BISHOP: Info[ Location ] = WB; break;
							case ROOK:   Info[ Location ] = WR; break;
							case QUEEN:  Info[ Location ] = WQ; break;
							case KING:   Info[ Location ] = WK; break;
						}
					}else{
						switch( Type ){
							case PAWN:   Info[ Location ] = BP; break;
							case KNIGHT: Info[ Location ] = BN; break;
							case BISHOP: Info[ Location ] = BB; break;
							case ROOK:   Info[ Location ] = BR; break;
							case QUEEN:  Info[ Location ] = BQ; break;
							case KING:   Info[ Location ] = BK; break;
						}
					}
				}else{
					if( Info[Location] != 0 ){
						Info[ Location ] = 0x0;
					}
				}
			}
		}	
	}

}

void position::Show(){

	int Info[64];

	GetBoardArray( Info );

	cout <<  " +---+---+---+---+---+---+---+---+\n ";

	for(int y=0; y<8; y++){
		cout << "| ";
		for(int x=0; x<8; x++){
			int i=y*8+x;
			switch( Info[i] ){

				case BP: cout << "p"; break;
				case BN: cout << "n"; break;
				case BB: cout << "b"; break;
				case BR: cout << "r"; break;
				case BQ: cout << "q"; break;
				case BK: cout << "k"; break;

				case WP: cout << "P"; break;
				case WN: cout << "N"; break;
				case WB: cout << "B"; break;
				case WR: cout << "R"; break;
				case WQ: cout << "Q"; break;
				case WK: cout << "K"; break;

				default: cout << " ";
			}
			cout << " | ";
		}
		cout <<  "\n +---+---+---+---+---+---+---+---+\n ";
	}

	cout << endl;
}

bitboard position::GetAllPieces()
{
	return 
	Pieces[ WHITE ][ PAWN ] | 
	Pieces[ WHITE ][ KNIGHT ] | 
	Pieces[ WHITE ][ BISHOP ] | 
	Pieces[ WHITE ][ ROOK ] | 
	Pieces[ WHITE ][ QUEEN ] |
	Pieces[ WHITE ][ KING ] |
	Pieces[ BLACK ][ PAWN ] | 
	Pieces[ BLACK ][ KNIGHT ] | 
	Pieces[ BLACK ][ BISHOP ] | 
	Pieces[ BLACK ][ ROOK ] | 
	Pieces[ BLACK ][ QUEEN ] |
	Pieces[ BLACK ][ KING ];
}

bitboard position::GetEnemyPieces( color Color )
{
	color EnemyColor = OtherColor( Color );

	return
	Pieces[ EnemyColor ][ PAWN ]   | 
	Pieces[ EnemyColor ][ KNIGHT ] | 
	Pieces[ EnemyColor ][ BISHOP ] | 
	Pieces[ EnemyColor ][ ROOK ]   | 
	Pieces[ EnemyColor ][ QUEEN ]  |
	Pieces[ EnemyColor ][ KING ];
}

bitboard position::GetOwnPieces( color Color )
{
	return
	Pieces[ Color ][ PAWN ]   | 
	Pieces[ Color ][ KNIGHT ] | 
	Pieces[ Color ][ BISHOP ] | 
	Pieces[ Color ][ ROOK ]   | 
	Pieces[ Color ][ QUEEN ]  |
	Pieces[ Color ][ KING ];
}