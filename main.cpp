#include "engine.h"
#include "util.h"
#include "data.h"
#include <iostream>
#include <string>

using namespace std;

engine Engine;

int main()
{	
	Engine.SearchTime = 15000;

	//Engine.ProcessCommand( "post" ); 
	//Engine.TestSuite( "../Test Suites/BWTC.EPD", 3000 );
	//Engine.TestSuite( "../Test Suites/WAC.EPD", 2800 );
	//Engine.TestSuite( "../Test Suites/WCSAC.EPD", 2800 ); 

	while( 1 ){
		string Command;
		getline( cin, Command );
		Engine.ProcessCommand( Command ); 
	}
}

DWORD WINAPI SearchThreadFunction( LPVOID SearchArgs )
{
	Engine.TerminatePonder();
	Engine.MakeBestMove( Engine.CompColor, Engine.SearchTime, true );
	Engine.StartPonder();
	return 0x0;
}

DWORD WINAPI PonderThreadFunction( LPVOID PonderArgs )
{
	Engine.Ponder( Engine.CompColor );
	return 0x0;
}