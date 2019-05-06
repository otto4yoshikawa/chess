//-------------------------------------------------------------------------
#pragma hdrstop
 #include<iostream>

#include <tchar.h>
#include "genmove4.h"
//---------------------------------------------------------------------------

GENMOVE     *Genmove;

int _tmain(int argc, _TCHAR* argv[])
{
	bool game,inputdone;
	char message[256];
	Genmove= new GENMOVE();
	do {
	  inputdone=false;
	  do {
	 std:: cout<<"enter your move";
	   std::cin>>message;
	   inputdone =Genmove->InputMove(message);} while( !inputdone);
	   game=Genmove->ComputerPlay(message);
	 std::	cout<<message;
	} while (game)  ;



	return 0;
}
//---------------------------------------------------------------------------
