#pragma once
#include "enum.h"
#include <string.h>

class move
{
    public:

	int Color; 
	int Type; 
	int From;
	int To;
	int Value;
	int Capture;
	int Promotion;
	int CaptureLocation;
	bool CanCastle[ 2 ][ 2 ];
	bitboard EnPassant;
	bitboard PiecesRotate90;

	move()
	{ 
		Color = UNDEFINED;
		Type = UNDEFINED;
		From = UNDEFINED;
		To = UNDEFINED;
		Value = -INFINITY; 
		Promotion = UNDEFINED; 
		Capture = UNDEFINED;
		CaptureLocation = UNDEFINED;
		CanCastle[ 0 ][ 0 ] = false;
		CanCastle[ 0 ][ 1 ] = false;
		CanCastle[ 1 ][ 0 ] = false;
		CanCastle[ 1 ][ 1 ] = false;
		EnPassant = 0x0;
		PiecesRotate90 = 0x0;
	}

	move( int _Color, int _Type, int _From, int _To, int _Value, int _Promotion=UNDEFINED, bool* _CanCastle=0x0){
		Color = _Color;
		Type = _Type;
		From = _From;
		To = _To;
		Value = _Value;
		Promotion = _Promotion;
		Capture = UNDEFINED;
		CaptureLocation = UNDEFINED;
		EnPassant = 0x0;
		if( _CanCastle ) memcpy( CanCastle, _CanCastle, 4*sizeof(bool) );
	}

	bool operator <(const move &rhs) const{ 
		return Value > rhs.Value;
	}  
	
	bool IsValid(){
		return (Type > -1);
	}
	
	bool operator==( move& R ) const{
		return ( Color == R.Color && Type == R.Type && From == R.From && To == R.To && Promotion == R.Promotion );
	}
};