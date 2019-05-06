//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "wdhessmain.h"
#include <stdio.h>

#include "wcdefs.h"
#include "externs.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

void DoPrintf(char *szFormat,...){
	va_list vaArgs;
   static char szBuf[256];
   if (!logfile) logfile=fopen("debug.txt","w");
   va_start(vaArgs,szFormat);
   vsprintf(szBuf,szFormat,vaArgs);
   va_end(vaArgs);
   Form1->Memo1->Lines->Add(szBuf);
   fprintf(logfile,"%s",szBuf);
   if (szBuf[0]!=' ') {fprintf(logfile ,"\n"); fflush(logfile);}

}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::mousedown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
	  DoPrintf("A");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::mouseup(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
      DoPrintf("A");
}
//---------------------------------------------------------------------------
