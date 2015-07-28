#include "engine.h"
#include "data.h"
#include "util.h"
#include <string>
#include <vector>
#include <Windows.h>
#include <sstream>

using namespace std;

engine::engine( string FEN  )
{
	Position.LoadFEN( FEN );
	lout.open("Inot_Log.txt");
}

engine::engine()
{
	Position.LoadFEN( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" );
	lout.open("Inot_Log.txt");
}

void engine::ProcessCommand( string Cmd )
{
	static bool Force = false;
	if( Cmd == "" ) return;
	lout << Cmd << endl;

	//tokenize the command into arguments
	Cmd += ' ';
	vector<string> Args;

	int First = -1;
	int Next = 0;
	bool InQuote = false;
	for(int i=0; i<Cmd.size(); i++){
		if( Cmd[i] == '"' ){
			InQuote = !InQuote;
			continue;
		}
		if( Cmd[i] == ' ' && !InQuote ){
			Next = i;
			string Temp = Cmd.substr( First+1, Next-First-1 );
			First = Next;
			Args.push_back( Temp );
		}
	}

	//process a move
	if( Args[ 0 ].size() == 4 || Args[ 0 ].size() == 5 ){
		if( Args[ 0 ][ 0 ] >= 'a' && Args[ 0 ][ 0 ] <= 'h' &&
			Args[ 0 ][ 1 ] >= '1' && Args[ 0 ][ 1 ] <= '8' &&
			Args[ 0 ][ 2 ] >= 'a' && Args[ 0 ][ 2 ] <= 'h' &&
			Args[ 0 ][ 3 ] >= '0' && Args[ 0 ][ 3 ] <= '8' )
		{
			lout << "Enemy Move: " << Args[0] << endl;
			int From = (Args[ 0 ][ 0 ]-'a') + (8-(Args[ 0 ][ 1 ]-'0'))*8;
			int To = (Args[ 0 ][ 2 ]-'a') + (8-(Args[ 0 ][ 3 ]-'0'))*8;

			int Promotion = -1;
			if( Args[ 0 ].size() == 5 ){
				if( Args[ 0 ][ 4 ] == 'q' ) Promotion = QUEEN;
				else if( Args[ 0 ][ 4 ] == 'r' ) Promotion = ROOK;
				else if( Args[ 0 ][ 4 ] == 'n' ) Promotion = KNIGHT;
				else if( Args[ 0 ][ 4 ] == 'b' ) Promotion = BISHOP;
				lout << "Promotion: " << Promotion << endl;
			}

			bool LegalMove = MakeMove( From, To, OtherColor[ CompColor ], Promotion );

			if( !LegalMove ){
				//4kbnr/1P2pppp/3p4/8/8/8/P3PPPP/RNBQKBNR w KQk - 0 1
				lout << "Illegal move" << endl;
				cout << "Illegal move" << endl;
				Showbitboard( Position.GetAllPieces() );
				Position.Show();
				return;
			}

			if( !Force ) StartSearch();
		}
	}

	//try to find the command
	if( Args[ 0 ] == "go" ){
		Force = false;
		StartSearch();
	}

	else if( Args[ 0 ] == "force" ){
		Force = true;
	}

	else if( Args[ 0 ] == "white" ){
		CompColor = WHITE;
	}

	else if( Args[ 0 ] == "black" ){
		CompColor = BLACK;
	}

	else if( Args[ 0 ] == "remove" ){
		TerminatePonder();
		Position.TakeBack();
		Position.TakeBack();
	}

	else if( Args[ 0 ] == "new" ){
		TerminatePonder();
		Position.LoadFEN( StartingPositionFEN );
		lout << "NEW: " << StartingPositionFEN << endl;
	}

	else if( Args[ 0 ] == "sd" ){

	}

	else if( Args[ 0 ] == "st" ){

	}

	else if( Args[ 0 ] == "pause" ){
		TerminatePonder();
	}

	else if( Args[ 0 ] == "level" ){
		int NumMoves = atoi( Args[ 1 ].c_str() );
		if( NumMoves == 0 ) NumMoves = 60;
		SearchTime = ((atoi( Args[ 2 ].c_str() ) * 60) * 1000) / NumMoves;
	} 

	else if( Args[ 0 ] == "setboard" ){
		if( Args.size()>1 )
			Position.LoadFEN( Args[ 1 ] );
	}

	else if( Args[ 0 ] == "xboard" ){
		cout.setf( ios::unitbuf );
		cout << "done=0\n";
		//cout << "feature ping=0\n";
		cout << "feature setboard=1\n";
		//cout << "feature playother=1\n";
		//cout << "feature san=0\n";
		//cout << "feature usermove=0\n";
		cout << "feature time=1\n";
		//cout << "feature draw=0\n";
		//cout << "feature sigint=0\n";
		//cout << "feature sigterm=0\n";
		//cout << "feature reuse=1\n";
		//cout << "feature analyze=0\n";
		cout << "feature myname=\"Inot2\"\n";
		//cout << "feature variants=\"\"\n";
		cout << "feature color=0\n";
		//cout << "feature ics=0\n";
		cout << "feature name=1\n";
		cout << "feature pause=0\n";
		cout << "done=1\n";
		cout.flush();
	}

	else if( Args[ 0 ] == "quit" ){
		TerminatePonder();
		lout << "exiting...";
		exit(0);
	}

	else if( Args[ 0 ] == "post" ){
		Post = true;
	}

	else if( Args[ 0 ] == "nopost" ){
		Post = false;
	}

	else if( Args[ 0 ] == "testsuite" ){
		TestSuite( Args[1], atoi(Args[2].c_str()) );
	}

}

bool engine::MakeMove( int From, int To, int Turn, int Promotion )
{
	int Type = Position.PieceIds[ Turn ][ From ];
	if( Type == UNDEFINED ) return false;

	move Move( Turn, Type, From, To, 0, Promotion );
	if( From == To ) return false;

	bool LegalMove = false;

	TerminatePonder();

	int MoveCount;
	vector<move> Moves( MAX_MOVES );
	GenMoves( Moves, MoveCount );

	//cout << "HERE" << endl 
	//	<< "MOVE COUNT " << MoveCount << endl;

	for(int i=0; i<MoveCount; i++){
		//cout << NotateMove( Moves[ i ] ) << endl;
		if( Moves[ i ] == Move ){
			LegalMove = true;
			cout << "#made " << i << " best move" << endl;
			Position.MakeMove( Move );
		}
	}
	
	return LegalMove;
}

string engine::ShowPieceIds()
{
	stringstream sout;
	for(int Color=BLACK; Color<=WHITE; Color++){
		for(int Location=0; Location<64; Location++){
			if( Location%8 == 0 ) sout << "\n";
			int Type = Position.PieceIds[ Color ][ Location ];
			sout << (Type == UNDEFINED ? '\'' : PieceChars[ Type ]) << " ";
		}
	}
	sout << endl;
	cout << sout.str();
	return sout.str();
}