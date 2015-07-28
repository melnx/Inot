#include "engine.h"
#include "util.h"
#include "data.h"
#include <vector>
#include <windows.h>
#include <iostream>
#include <time.h>

using namespace std;

int MoveOrderInfo[ MAX_MOVES ];

int engine::Quiescent(int Alpha, int Beta)
{
	++NodesSearched;
	int Val = Position.Evaluate();

	if (Val >= Beta)
		return Beta;

	if (Val > Alpha)
		Alpha = Val;

	//generate all legal moves
	vector<move> Moves( MAX_MOVES );
	int MoveCount;
	GenMoves( Moves, MoveCount, CAPTURES_ONLY );

	//go through moves and make and take back moves
	for(int i=0; i<MoveCount; ++i) {

		Position.MakeMove( Moves[i] );
		Val = -Quiescent(-Beta, -Alpha);
		Position.TakeBack();

		if (Val >= Beta)
			return Beta;
		if (Val > Alpha)
			Alpha = Val;
	}
	return Alpha;
}

int engine::AlphaBeta( int Alpha, int Beta, int RemDepth, vector<move>& Line )
{
	//break out of search if no time left
	static int NodesSearchedInInterval;
	if( NodesSearchedInInterval >= 1024 ){
		if( clock() - SearchStarted > SearchTime ) SearchTerminated = true;
		if( GetAsyncKeyState(VK_F9) ){ cout << "MANUALLY BROKEN OUT OF SEARCH" << endl; SearchTerminated=true; }
		NodesSearchedInInterval = 0;
	}
	if( SearchTerminated ) return 0;

	++NodesSearchedInInterval;
	++NodesSearched;

	int Val;

	if( RemDepth <= 0 ){
		//return Position.Evaluate();
		return Quiescent( Alpha, Beta );
	}

	//see if the hash is found in the table
	if((Val = ProbeHash(RemDepth, Alpha, Beta, Line)) != VAL_UNKNOWN){
		return Val;
	}

	//defs
	int OwnColor = Position.Turn;
	int EnemyColor = OtherColor( OwnColor );
    
	//futility pruning
	/*GenAllAttackBitboards( OwnColor );
	bool EnemyIsInCheck = Position.IsInCheck( OwnColor );
	GenAllAttackBitboards( EnemyColor );
	bool IamInCheck = Position.IsInCheck( OwnColor );

	if( !IamInCheck && !EnemyIsInCheck ){
		if( RemDepth == 3 ){
			Val = Position.Evaluate();
			if( Val + 900 < Alpha )
				RemDepth -= 1;
		}

		if( RemDepth == 2 ){
			Val = Position.Evaluate();
			if( Val + 500 <= Alpha )
				return Val;
		}

		if( RemDepth == 1 ){
			Val = Position.Evaluate();
			if( Val + 300 <= Alpha )
				return Val;
		}
	}*/

	//null move pruning
	Position.MakeNullMove();
	Val = -AlphaBeta( -Beta, -Beta + 1, RemDepth - 1 - 2, Line );
	Position.TakeBackNullMove();
	if (Val >= Beta){
		return Beta;
	}

	//vars
	int HashFlag = HashFlagALPHA;
	bool FoundPv = false;
	bool FoundLegalMove = false;
	int NewRemDepth = RemDepth;
	int Margin = (RemDepth >= 9) ? 1500 : Margins[ RemDepth ];
	int BestMoveIndex=UNDEFINED;

	//move generation
	int MoveCount;
	vector<move> Moves( MAX_MOVES );
	GenMoves( Moves, MoveCount );
	bitboard Before90 = Position.PiecesRotate90;
	GenAllAttackBitboards( OwnColor );

	for( int i=0; i<MoveCount; ++i )
	{
		Position.MakeMove( Moves[i] );

		    GenAllAttackBitboards( EnemyColor );

			//pruning
			if( RemDepth > 2
				&& !Position.IsInCheck( OwnColor )    // own_king_was_not_in_check_before_make_move
				&& Position.Moves.top().Capture == UNDEFINED  //move_is_no_capture 
				&& !Position.IsInCheck( EnemyColor ) //move_does_not_check_the_opponent_king)    // such as Bf1-b5+ 
				)
			{ 
				Val = Position.Evaluate();
				//if( Alpha < Val - Margin ){ NewRemDepth = RemDepth + 1; }
				if( Alpha > Val + Margin || i > 5 ){ NewRemDepth = RemDepth - 1; }
				else{ NewRemDepth = RemDepth; }
			}else{
				NewRemDepth = RemDepth;
			}

			if( !( Position.AttackBitboards[ EnemyColor ][ ALL ] & Position.Pieces[ OwnColor ][ KING ] ) ){
				FoundLegalMove = true;
				if( FoundPv ){
					Val = -AlphaBeta( -Alpha - 1, -Alpha, NewRemDepth - 1, Line );

					if((Val > Alpha) && (Val < Beta)){ // Check for failure.
						Val = -AlphaBeta( -Beta, -Alpha, NewRemDepth - 1, Line );
					}
				}else{
					Val = -AlphaBeta( -Beta, -Alpha, NewRemDepth - 1, Line);
				}
			}else{
				Position.TakeBack(); 
				continue;
			}

		Position.TakeBack();

		if( Before90 != Position.PiecesRotate90 ){ 
			Showbitboard( Before90 );
			Showbitboard( Position.PiecesRotate90 );
			cout << "ERROR2"; system("pause"); 
		}

		//record
		if( Val > Alpha ){

			if( BestMoveIndex > -1 ) MoveOrderInfo[ BestMoveIndex ] --;
			MoveOrderInfo[ i ]++;
			BestMoveIndex = i;

			Line[ HorizonDepth - RemDepth ] = Moves[ i ];

			if( RemDepth == HorizonDepth && Output ){
				cout << HorizonDepth << "->" << i << "\t" << Val << "\t";
				for( int j=0; j<RemDepth; j++ ){
					cout << NotateMove( Line[ j ], Position ) << " ";
				}
				cout << endl;
			}
		}

		//beta cutoff
		if( Val >= Beta ){
			RecordHash(RemDepth, Beta, HashFlagBETA, Moves[ i ] );
			return Beta;
		}

		//alpha cutoff
		if( Val > Alpha ){
			HashFlag = HashFlagEXACT;
			Alpha = Val;
			FoundPv = true;
		}

	}

	if( !FoundLegalMove ){
		if( Position.IsInCheck( OwnColor ) ){
			//cout << " MATE ";
			return -MATE;
		}else{
			//cout << " STALEMATE ";
			return -STALEMATE;
		}
	}

	RecordHash(RemDepth, Alpha, HashFlag, Line[ HorizonDepth - RemDepth ] );
	return Alpha;
}

move engine::MakeBestMove( color Color, int Time, bool ShowOutput )
{
	SearchTime = Time;
	move BestMove;
	vector<move> Line;
	memset( MoveOrderInfo, 0x0, MAX_MOVES*sizeof(int) );

	int Alpha = -INFINITY;
	int Beta = INFINITY;
	int Window = 40;
	int Val;
	SearchStarted = clock();
	Output = ShowOutput;

	Searching = true;
	for(int Depth=1;;)
	{
		HorizonDepth = Depth;
		NodesSearched = 0;
		SearchTerminated = false;
		vector<move> DeepestLine( Depth+1 );
		int SearchStartedAtDepth = clock();
		HorizonDepth = Depth;

		Val = AlphaBeta( Alpha, Beta, Depth, DeepestLine );

		if( clock() - SearchStarted > SearchTime ){
			break;
		}

		if (Val <= Alpha) {      // WRONG!
			lout << "failed low" << endl;
			Alpha = -INFINITY;
			Beta = INFINITY;
			continue;
		}

		if (Val >= Beta) {       // WRONG!
			lout << "failed high" << endl;
			Beta = INFINITY;
			Alpha = -INFINITY;
			continue;
		}

		int TimeTaken = clock() - SearchStartedAtDepth;
		float Knps = float( NodesSearched ) / float( TimeTaken );
		if( Output )
		cout << "Depth:" << Depth << "  Time:" << TimeTaken 
			 << "  Nodes:"<< NodesSearched << "  Knps:" << Knps << endl << endl;

		if( Post ){
			cout.setf(ios_base::unitbuf);
			cout << Depth << "\t" << Val << "\t" << TimeTaken << "\t" << NodesSearched << "\t" << NotateMove( DeepestLine[0], Position ) << "\n";
			cout.flush();
		}
		
		Line = DeepestLine;
		Alpha = Val - Window;
		Beta = Val + Window;
		Depth++;

		if( Val == MATE ) break;
	}
	Searching = false;

	BestMove = Line[ 0 ];
	Position.MakeMove( BestMove );

	cout.setf(ios_base::unitbuf);
	if( BestMove.IsValid() ){
	    cout << "move " << NotateMove( BestMove ) << "\n";
	}else{
		cout << "result " << ( CompColor == WHITE ? "0-1 {black mates}" : "1-0 {white mates}" ) << "\n";
	}
	cout.flush();

	int Sum = 0;
	if( Output ){
		for(int i=0; i<sizeof(MoveOrderInfo)/sizeof(int); i++){ Sum += MoveOrderInfo[i]; }
	    cout << "#Move Ordering Info:" << endl;
	    for(int i=0; i<=5; i++){ 
		    cout << "#" << i << ":\t" << (float(MoveOrderInfo[i]) / float(Sum)) * 100 << '%' << endl;
		}
	}

	return BestMove;
}

void engine::Ponder( color Color )
{
	int TempSearchTime = SearchTime;
	SearchTime = 999999999;
	move BestMove;
	vector<move> Line;
	memset( MoveOrderInfo, 0x0, MAX_MOVES*sizeof(int) );
	SearchTerminated = false;

	int Alpha = -INFINITY;
	int Beta = INFINITY;
	int Window = 40;
	int Val;
	Output = false;
	SearchStarted = clock();

	//cout << "pondering";
	Searching = true;
	for(int Depth=1;;Depth++){
		HorizonDepth = Depth;
		NodesSearched = 0;
		vector<move> DeepestLine( Depth+1 );
		HorizonDepth = Depth;

		Val = AlphaBeta( Alpha, Beta, Depth, DeepestLine );

		if( SearchTerminated ){
			//cout << "terminated pondering" << endl;
			break;
		}

		if (Val <= Alpha) {      // WRONG!
			lout << "failed low" << endl;
			Alpha = -INFINITY;
			Beta = INFINITY;
			continue;
		}

		if (Val >= Beta) {       // WRONG!
			lout << "failed high" << endl;
			Beta = INFINITY;
			Alpha = -INFINITY;
			continue;
		}

		Alpha = Val - Window;
		Beta = Val + Window;
		Depth++;
	}

	Searching = false;
	SearchTime = TempSearchTime;
}

void engine::TerminatePonder()
{
	SearchTerminated = true;
	while( Searching ){
		Sleep(0);
	}
}

void engine::StartPonder()
{
	//DWORD SearchThreadId;
	//CreateThread( NULL, 0x0, PonderThreadFunction, 0x0, 0x0, &SearchThreadId );
}

void engine::StartSearch()
{
	DWORD SearchThreadId;
	CreateThread( NULL, 0x0, SearchThreadFunction, 0x0, 0x0, &SearchThreadId );
}

void engine::TerminateSearch()
{
	TerminatePonder();
}