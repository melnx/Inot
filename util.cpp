#include "engine.h"
#include "data.h"
#include "util.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>

using namespace std;

void Showbitboard(bitboard Position)
{
    bitboard One = 0x1; 
    for(int y=0; y<8; y++){
        for(int x=0; x<8; x++){
            cout << ((bool)(Position & (One << (y*8+x))) ? "#" : "`") << " ";
        }        
        cout << endl;
    }     
    cout << endl;
}

void ShowBitRow( row Row ){
    row One = 0x1;

	for(int i=0; i<8; ++i){
		cout << ((bool)(Row & (One<<i))) ? "1" : "0";
	}

	cout << endl;
}

bitboard rand64()
{    
	bitboard Ret=0;
    
	Ret |= ( bitboard(rand()) << 0*sizeof(int) );
	Ret |= ( bitboard(rand()) << 2*sizeof(int) );
    Ret |= ( bitboard(rand()) << 4*sizeof(int) );
	Ret |= ( bitboard(rand()) << 8*sizeof(int) );
	Ret |= ( bitboard(rand()) << 10*sizeof(int) );
	Ret |= ( bitboard(rand()) << 12*sizeof(int) );

    //Showbitboard(Ret);

	return Ret;
}

int LSBi( bitboard& a )
{
	__asm
	{
			mov ebx, [ a ]
			mov ebx, [ ebx ]

			bsf eax, ebx
			jnz l1

			mov ebx, [ a ]
			mov ebx, [ ebx + 4 ]

			bsf eax, ebx
			add eax, 32
			jnz l1

			mov eax, 64
			l1:
	}
}

int Population( bitboard& b )
{
	bitboard a = b;
	__asm
	{
			mov ecx, dword ptr a
			xor eax, eax
			test ecx, ecx
			jz l1
		l0:     
			lea edx, [ecx - 1]
			inc eax
			and ecx, edx
			jnz l0
		l1:
			mov ecx, dword ptr a + 4
			test ecx, ecx
			jz l3
		l2:
			lea edx,[ecx - 1]
			inc eax
			and ecx, edx
			jnz l2
		l3:
	}
}

int CountBits32( unsigned int v )
{
	int c =  ((v & 0xfff) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
	c += (((v & 0xfff000) >> 12) * 0x1001001001001ULL & 0x84210842108421ULL) % 
		 0x1f;
	c += ((v >> 24) * 0x1001001001001ULL & 0x84210842108421ULL) % 0x1f;
	return c;
}

int CountBits64( bitboard v )
{
	return CountBits32( (int)(v) ) + CountBits32( (int)(v>>32) );
}

int CountBits( bitboard Position )
{
	int Count = 0;
	const bitboard One = 0x1;
	for(int i=0; i<64; ++i){
		if( Position & Bitmask( i ) ) ++Count;
	}
	return Count;
}

bitboard RotateBlackR45( bitboard Position )
{
	return 0x0;
}

bitboard RotateBlackL45( bitboard Position )
{
	return 0x0;
}

bitboard RotateWhiteR45( bitboard Position )
{
	return 0x0;
}

bitboard RotateWhiteL45( bitboard Position )
{
	return 0x0;
}

bitboard Rotate90( bitboard Position ){
	bitboard Result = 0x0;

	for(int i=0; i<64; ++i){
		if( (bool)(Position & Bitmask( i ) ) ){
			Result |= Bitmask( Rotate90Map[ i ] );
		}
	}
	return Result;
}

string SquareToString( int Location )
{
	string Ret;
	int x = Location%8;
	int y = Location/8;
	Ret += char('a' + x);
	Ret += char('0' + (8-y));
	return Ret;
}

void FindRookMoves(){

	row One = 0x1;

	for( int i=0; i<8; i++ ){
		for(int j=0; j<256; j++){
			row ValidMoves = 0x0;

			for(int x=i+1; x<=7; ++x){
				ValidMoves |= (One << x);
				if( row(j) & (One << x) ) break;
			}
			for(int x=i-1; x>=0; --x){
				ValidMoves |= (One << x);
				if( row(j) & (One << x) ) break;
			}

			//for(int m=0; m<8; m++){
			//cout << (bool)( ValidMoves & (One<<m) );
			//}cout << ",";

			cout << "0x" << hex << (int)ValidMoves << ",";

		}
		cout << endl;
	}

}

std::string engine::NotateMove( move& Move )
{
	string Promotion = (Move.Promotion == -1) ? "" :  string("=") + PieceChars[Move.Promotion];
	stringstream MoveStream;
	MoveStream << char(Move.From%8+'a') << 8-(Move.From/8)  
			   << char(Move.To%8+'a') << 8-(Move.To/8) << Promotion;
	return MoveStream.str();
}

std::string engine::NotateMove( move& Move, position& Position )
{
	string r = "";

	if( Move.From == -1 && Move.To == -1 )
		return "Pass";

	int Type = Move.Type;
	if( Type == KING )
	{
		if( Move.From - Move.To == 2 ) return "O-O-O";
		else if( Move.To - Move.From == 2 ) return "O-O";
	}

	bool EP = false;
	bool Checks = false;
	bool Capture = (Move.Capture != UNDEFINED);//( Position.GetEnemyPieces( Position.Turn ) & Bitmask( Move.To ) );
	if( Type == PAWN && abs(Move.To-Move.From)%8 ) { Capture = true; EP = true; }
	bool ShowFile = false;
	bool ShowRank = false;

	int MoveCount;
	vector< move > Moves( MAX_MOVES ); 
	GenMoves( Moves, MoveCount );

	for( int i=0; i<MoveCount; i++ ){
		if( Moves[ i ] == Move ) continue;
		if( Moves[ i ].Type == Move.Type ){
			if( Moves[ i ].To == Move.To ){
				if( Moves[ i ].From/8 == Move.From/8 ) ShowFile = true;
				if( Moves[ i ].From%8 == Move.From%8 ) ShowRank = true;
			}
		}
	}

	if( Type != PAWN ) r += PieceChars[ Type ];
	if( ( Type == PAWN && Capture ) || ShowFile ) r += 'a' + Move.From%8;
	if( ShowRank ) r += '1' + Move.From/8;
	if( Capture ) r += 'x';

	r += SquareToString( Move.To );

	if( Type == PAWN && ( Move.To/8 == 0 || Move.To/8 == 7 ) && Move.Promotion == -1 )
		Move.Promotion = QUEEN;

	if( Move.Promotion != -1 )
		r += PieceChars[ Move.Promotion ];

	//Position.MakeMove( Move );
	    //bitboard Attacks = GenAttackBitboard( Move.Color );
		//if( Attacks & Position.Pieces[ OtherColor[ Move.Color ] ][ KING ] ) Checks = true;
	//Position.TakeBack();

	if( Checks )
		r += '+';

	return r;
}

void engine::TestSuite( string Path, int Time )
{
	cout << "========================================================" << endl;
	cout << "SUITE " << Path << endl;
	ifstream in( Path.c_str() );
	if( !in ){ cout << "cannot open file " << Path << endl; return; }

	string Line;
	float Correct = 0;
	float Total = 0;

	while(getline(in,Line)){
		string Fen;
		string Move;
		int Spaces = 0;
		int i;
		for(i=0; i<Line.size(); i++){
			if( Line[i] == ' ' ) Spaces ++;
			if( Spaces == 5 ) break;
			//if( Spaces<2 ) 
			Fen+=Line[i];
		}
		//Fen+=" KQkq - 0 1";
		for(int j=i+1; j<Line.size(); j++){
			if( Line[j] == ';' || Line[j] == '+' ) break;
			Move += Line[j];
		}

		Move += ' ';
		vector<string> Moves;

		int First = -1;
		int Next = 0;
		for(int i=0; i<Move.size(); i++){
			if( Move[i] == ' ' ){
				Next = i;
				string Temp = Move.substr( First+1, Next-First-1 );
				First = Next;
				Moves.push_back( Temp );
			}
		}

		Position.LoadFEN( Fen );
		move BestMove = MakeBestMove( Position.Turn, Time, false );
		string SanNotation = NotateMove( BestMove, Position );
		string CoordNotation = NotateMove( BestMove );
		
		cout << SanNotation << "/" << CoordNotation << "==" << Move << endl;

		for(int i=0; i<Moves.size(); i++){
			if( Moves[i] == SanNotation || Moves[i] == CoordNotation ){
				Correct += 1.0f;
			}
		}
		Total += 1.0f;
		cout << (Correct/Total)*100.0f << "%" << endl;
	}
}

