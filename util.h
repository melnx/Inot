#pragma once
#include <windows.h>
#include "enum.h"

void Showbitboard( bitboard Position );
void ShowBitRow( row Row );
int CountBits( bitboard Position );
int CountBits32( unsigned int v );
int CountBits64( bitboard v );
bitboard RotateBlackR45( bitboard Position );
bitboard RotateBlackL45( bitboard Position );
bitboard RotateWhiteR45( bitboard Position );
bitboard RotateWhiteL45( bitboard Position );
bitboard Rotate90( bitboard Position );
int Population( bitboard& b );
int LSBi( bitboard& a );
std::string SquareToString( int Location );
DWORD WINAPI SearchThreadFunction( LPVOID Param );
DWORD WINAPI PonderThreadFunction( LPVOID Param );
DWORD WINAPI WinMainThreadFunction( LPVOID PonderArgs );
bitboard rand64();
void ShowPieceIds( int* Board );

#define Bitmask(Shift) (bitboard(0x1)<<Shift)
#define OtherColor(Color) (Color==WHITE ? BLACK : WHITE)
#define File( Square ) (Square%8)
#define Rank( Square ) (Square/8)
#define Square( x, y) ( x*8 + y )
#define ABS( a ) ((a < 0) ? -a : a )
#define Distance( a, b ) ( ABS((File(a)-File(b))) + ABS((Rank(a)-Rank(b))) )