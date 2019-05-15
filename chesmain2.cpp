// ---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "chesmain2.h"
#include <stdio.h>
#include<assert.h>
#include"chessrule.h"
#include"board.h"

#include"joseki.h"

// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
TColor WhiteSquareColor = clWhite;
TColor BlackSquareColor = clGreen; // 緑

FILE *logfile;
// mychess *kernel;
chessrule *newkernel;
bool ShowBestLine = true;
// int          ComputerColor;
short LINESIZE;
short CHARSIZE;
int CAPTIONY;
bool Editing;
int BORDERSIZE;
const BORDERYEXTRA = 4; // ４ピクセルの隙間
short INFOXSIZE, INFOYSIZE;
bool SoundOn;
bool doLoadFile(AnsiString FileName);
#include <Registry.hpp>
void WriteManager(AnsiString Key, AnsiString V) {
	AnsiString S, KeyS;
	int i;
	TRegistry *Reg;
	AnsiString KeyName = "\\Software\\monte";
	try {

		Reg = new TRegistry(KEY_WRITE);
		Reg->RootKey = HKEY_CURRENT_USER;
		Reg->OpenKey(KeyName, true);

		Reg->WriteString(Key, V);
	}

	__finally {
		delete Reg;
	}
}

AnsiString ReadManager(AnsiString Key) {

	AnsiString V="none";
	TRegistry *Reg;
	AnsiString KeyName = "\\Software\\monte";
	try {
		Reg = new TRegistry(KEY_READ);
		Reg->RootKey = HKEY_CURRENT_USER;
		Reg->OpenKey(KeyName, false);

		V = Reg->ReadString(Key);
	}

	__finally {
		delete Reg;
	}
	return V;
}

// ----------samll.cpp----------------------------
// VETYPE  ZeroMove = { 8, 8, 0, empty, empty };
// MOVETYPE KeyMove;
//int RunColor;
bool Analysis, Opan;
double WantedTime;
//BOOL GameOver = FALSE;
char EndGameMessage[80];
//int ComputerColor;
extern DEPTHTYPE Depth;
bool OnSolving;
extern int MoveNo;
long MyTime,MyTime1,RemainTime[2];
// -------------------------------------------

Graphics::TBitmap *BPawn = new Graphics::TBitmap;
Graphics::TBitmap *WPawn = new Graphics::TBitmap;
Graphics::TBitmap *BRook = new Graphics::TBitmap;
Graphics::TBitmap *WRook = new Graphics::TBitmap;
Graphics::TBitmap *BKnight = new Graphics::TBitmap;
Graphics::TBitmap *WKnight = new Graphics::TBitmap;
Graphics::TBitmap *BBishop = new Graphics::TBitmap;
Graphics::TBitmap *WBishop = new Graphics::TBitmap;
Graphics::TBitmap *BQueen = new Graphics::TBitmap;
Graphics::TBitmap *WQueen = new Graphics::TBitmap;
Graphics::TBitmap *BKing = new Graphics::TBitmap;
Graphics::TBitmap *WKing = new Graphics::TBitmap;
Graphics::TBitmap *MKing = new Graphics::TBitmap;
Graphics::TBitmap *MQueen = new Graphics::TBitmap;
Graphics::TBitmap *MBishop = new Graphics::TBitmap;
Graphics::TBitmap *MKnight = new Graphics::TBitmap;
Graphics::TBitmap *MPawn = new Graphics::TBitmap;
Graphics::TBitmap *MRook = new Graphics::TBitmap;
Graphics::TBitmap *ttemp, *ttemp2;
Graphics::TBitmap *bitmaphd[6][3]; ;

// ttemp=ttemp2= MRook;
int upsidedown(int z) {
	 if(V3) return z;
	return(z & 7) + (0x70 - (z & 0x70));
}
bool blink=0;
void ShowTime(int msec){
	RemainTime[newkernel->teban-1]-=msec;
	blink^=1;
	for(int i=0;i<2;i++){
   char w[18];
   int sec= RemainTime[i]/1000;
	bool blink1=(i==newkernel->teban-1)?blink:true;
   sprintf(w,"[%02d%c%02d]",sec/60,(blink1)?':':' ',sec%60);
   Form1->StatusBar1->Panels->Items[3+i]->Text=w;
   }

}
// ---------------------------------------------------------------------------
void FindMove(int maxlevel);
bool movestart;

// bool convertBoard(PIECE*);
// PIECE org[2];

// nt PN = 0;

void DoPrintf(char *szFormat, ...) {
	va_list vaArgs;
	static char szBuf[256];
	if (!logfile)
		logfile = fopen("debug.txt", "w");
	va_start(vaArgs, szFormat);
	vsprintf(szBuf, szFormat, vaArgs);
	va_end(vaArgs);
	Form1->Memo1->Lines->Add(szBuf);
	fprintf(logfile, "%s", szBuf);
	if (szBuf[0] != ' ') {
		fprintf(logfile, "\n");
		fflush(logfile);
	}

}
joseki *js;
 int converttype[] = {
	  0,6, 3, 5, 4, 2, 1
	};
 int convertSpe[]={0,1,1,2,4,5,6};
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void showStatus(int t, int s) {
	char w[10];
	sprintf(w, "step=%d", s);
	Form1->StatusBar1->Panels->Items[0]->Text = (t == 2) ? "White" : "Black";
	Form1->StatusBar1->Panels->Items[1]->Text = w;
}
extern MOVETYPE Next;
 //---------------------------------------------------------------------------
 //constructer
// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {

	if(!FileExists("BPawn.bmp") )	SetCurrentDir("../");
	BPawn->LoadFromFile("BPawn.bmp");
	bitmaphd[0][0] = BPawn;
	WPawn->LoadFromFile("WPawn.bmp");
	bitmaphd[0][1] = WPawn;
	MPawn->LoadFromFile("PMask.bmp");
	bitmaphd[0][2] = MPawn;

	BRook->LoadFromFile("BRook.bmp");
	bitmaphd[1][0] = BRook;
	WRook->LoadFromFile("WRook.bmp");
	bitmaphd[1][1] = WRook;
	MRook->LoadFromFile("RMask.bmp");
	bitmaphd[1][2] = MRook;

	BKnight->LoadFromFile("BKngt.bmp");
	bitmaphd[2][0] = BKnight;
	WKnight->LoadFromFile("WKngt.bmp");
	bitmaphd[2][1] = WKnight;
	MKnight->LoadFromFile("KTMask.bmp");
	bitmaphd[2][2] = MKnight;

	BBishop->LoadFromFile("BBishop.bmp");
	bitmaphd[3][0] = BBishop;
	WBishop->LoadFromFile("WBishop.bmp");
	bitmaphd[3][1] = WBishop;
	MBishop->LoadFromFile("BMask.bmp");
	bitmaphd[3][2] = MBishop;
	BQueen->LoadFromFile("BQueen.bmp");
	bitmaphd[4][0] = BQueen;
	WQueen->LoadFromFile("WQueen.bmp");
	bitmaphd[4][1] = WQueen;
	MQueen->LoadFromFile("QMask.bmp");
	bitmaphd[4][2] = MQueen;

	BKing->LoadFromFile("BKing.bmp");
	bitmaphd[5][0] = BKing;
	WKing->LoadFromFile("WKing.bmp");
	bitmaphd[5][1] = WKing;
	MKing->LoadFromFile("KMask.bmp");
	bitmaphd[5][2] = MKing;

	newkernel = new chessrule();
	ttemp = 0;
	ttemp2 = 0;
	movestart = false;

	DoubleBuffered = true;
	showStatus(newkernel->teban, newkernel->step);
	assert(Depth<2000);
	ResetGame(); // board.cpp
	RemainTime[0]=RemainTime[1]=30*60*1000;
		 //	assert(Depth<1000);
	js = new joseki();

;
	 char w[30];
	 strcpy( w,"VERSION ");
	 strcat(w,VERSION);
	Caption=w;
	StringGrid1->ColWidths[0]=32;
	StringGrid1->	Cells[0][0]="n";
	StringGrid1->	Cells[1][0]="white";
	StringGrid1->	Cells[2][0]="black";
   AnsiString A=  ReadManager( "chessfn");

   if(A!="none")   {  DoPrintf("Reg=%s",A.c_str());
	doLoadFile(A) ;}
  // StringGrid1->Options  = goDrawFocusSelected;
}

void drawPiece(int x, int y, int pieceid, int color) {
	int xx, xx2, yy, yy2;
	// if(x==basex) return;
	TColor c = clWhite;
	Graphics::TBitmap * ttemp = bitmaphd[pieceid - 1][color - 1];
	Graphics::TBitmap * ttemp2 = bitmaphd[pieceid - 1][2];

	TRect r = Rect(x, y, x + sz, y + sz);
	// DoPrintf(" [%d %d] %d %d sz=%d",x,y,pieceid,color,sz);
	// Form1->Canvas->Brush->Color=clRed;
	// Form1->Canvas->Rectangle( 0,0,sz,sz);

	Form1->Canvas->Brush->Bitmap = ttemp2;
	TRect r2 = Rect(0, 0, sz / 2, sz / 2);
	Form1->Canvas->CopyMode = cmSrcAnd;
	Form1->Canvas->CopyRect(r, ttemp2->Canvas, r2); // maskout

	Form1->Canvas->Brush->Bitmap = ttemp;
	Form1->Canvas->CopyMode = cmSrcPaint;
	Form1->Canvas->CopyRect(r, ttemp->Canvas, r2); // overwrite

}
#define GRAYSIZE 25

void __fastcall TForm1::paint(TObject *Sender) {
	int i, x, y,x3,y3,z3,x4,y4,z4;
	int showx, showy, type, col;
	TColor c;
	xPIECE *p;
	Canvas->Brush->Color = clGray;
		Canvas->Rectangle(basex - GRAYSIZE, basey - GRAYSIZE,
			basex + sz * 8 + GRAYSIZE, basey + sz * 8 + GRAYSIZE);
	// Form1->Canvas->Brush->Color = clWhite;
	Canvas->Font->Color = clBlack;
	   //	if(newkernel->step>10) return;
	if(newkernel->step>1){
	  z3=newkernel->record[newkernel->step-2].newz;
	  x3=z3&7 ;y3=7-z3/16;
	   z4=newkernel->record[newkernel->step-2].oldz;
	  x4=z4&7 ;y4=7-z4/16;

	  }
	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++) {

			c = !((x ^ y) & 1) ? clWhite : clGreen;
			 if(x==x3 && y==y3) c=clYellow;
			 if(x==x4 && y==y4) c=clYellow;
			// if (x == 0 && y == 0) 		c = clRed;
			Canvas->Brush->Color = c;
			Canvas->Rectangle(basex + x * sz, basey + sz * y, basex + sz *
				(x + 1), basey + sz * (y + 1));
		}

	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++) {
			// if(x==0 && y==0) continue;
			if (!newkernel->getxyrt(x, y, &showx, &showy, &col, &type))
				continue;
			 if(col==0||type==0) continue;
			// DoPrintf("%d %d &d %d",showx,showy,type,col);
			drawPiece(showx, showy, type, col);
		}
	Canvas->Font->Color = clYellow;//clBlack; // clNavy;
	Canvas->Font->Size = 12;
	Canvas->Brush->Color = clGray;

	char w[5];
	int ddx1 = sz * 0.6;
	int ddx2 = sz * 0.2;
	int ddy2 = sz * 0.3;
	AnsiString Textx, Texty;
	for (int i = 0; i < 8; i++) {
		w[0] = '8' - i;
		w[1] = 0;

		Textx = w;
		w[0] = 'a' + i;
		Texty = w;
		Canvas->TextOut(basex + i * sz + ddx1, basey + 8 * sz, Texty);
		Canvas->TextOut(basex - ddx2, basey + i * sz + ddy2, Textx);

	}
}

int GetValidSquare2(TPoint p, int player, bool CheckPiece);

// -----mose down----------------------------------------------------------------------
void __fastcall TForm1::onmouse(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y) {

	if (movestart)
		return;
		DoPrintf("Moise down MoveNo=%d Depth=%d",MoveNo,Depth);
	TPoint point = TPoint(X, Y);
	int k, n = GetValidSquare2(point, newkernel->teban, true);
	DoPrintf("mousedwn board  [%d %d] n=%02x", X, Y, n);
	if (n < 0)
		return;

	 movestart = newkernel->setCurrentP(n);

}

void __fastcall TForm1::onmoudemove(TObject *Sender, TShiftState Shift, int X,
	int Y)

{

	if (!movestart)
		return;
	newkernel->moveCurrent(X - sz / 2, Y - sz / 2);

	Repaint();

}
void
EnterKeyMove(int,MOVETYPE m) ;

 void convertMove(MOVE Move,MOVETYPE *move){

		int pp = newkernel->board[Move.newz]->type;
		move->old = Move.oldz;
		move->new1 = Move.newz;
		move->spe = convertSpe[Move.spe];
		 PIECETYPE ppp=  converttype[pp];

		move->movpiece = ppp;
		if (Move.cap) {
			move->content = converttype[Move.cap->type];
		}
		else  	move->content = 0;
 }
void __fastcall TForm1::onmpouseup(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y) {

		DoPrintf("MoveNo=%d Depth=%d",MoveNo,Depth);
	if (!movestart)
		return;
	movestart = false;
	TPoint point = TPoint(X, Y);
	int k, n = GetValidSquare2(point, newkernel->teban, true);
	if (n < 0)
		return;
	DoPrintf("mouse up %02x %02x ", n, newkernel->currentP->z);

	if (!newkernel->CheckAndPlay(n))
		DoPrintf("invalid");
	else {
	DoPrintf("MoveNo=%d Depth=%d",MoveNo,Depth);
		MOVE Move = newkernel->record[newkernel->step - 2];
			MOVETYPE move;
	 convertMove(Move,&move);




		EnterKeyMove(Depth,move )	;
		MakeMove(&move);
	  DoPrintf("afterMoveNo=%d Depth=%d",MoveNo,Depth);
	}
	showStatus(newkernel->teban, newkernel->step);
	Repaint();
	MyTime1=MyTime;
	return;

}

// ---------------------------------------------------------------------------

int GetValidSquare2(TPoint p, int player, bool CheckPiece) {
	int x, y;
	x = (p.x - basex) / sz;
	y = (p.y - basey) / sz;
	if (x < 0 || x > 7 || y < 0 || y > 7)
		return-1;
	if(V2) y=7-y;
	return 16 * y + x;
}

// ---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender) {
	DoPrintf("computer play moveno=%d ",MoveNo);
	MOVETYPE move;
	MOVE Move;

	js->CalcLibNo();
	DoPrintf("MN=%d",MoveNo);
//	Depth=0;
 //	if(LibNo>=0){
	move = js->FindOpeningMove();

	if (move.new1 < 0x88) {

		MakeMove(&move);
		Move.newz = upsidedown(move.new1);
		Move.oldz = upsidedown(move.old);
		Move.spe = move.spe;
		Move.p = newkernel->board[Move.oldz];
		Move.cap = 0;
	}
	else {
		Move.newz = 8;
	}
	if (newkernel->board[move.new1])
		Move.cap = newkernel->board[Move.newz];
	newkernel->Play(Move);
	showStatus(newkernel->teban, newkernel->step);

	Repaint();
	FindMove(12);

}
int index, moveindexstack[40];

static void cxyp(char c, int *cx, int *cy, char *cp) {
	int i;
	for (i = 0; i < 8; i++)
		if (c == *("12345678" + i)) {
			*cy = i;
			return;
		}
	for (i = 0; i < 8; i++)
		if (c == *("abcdefgh" + i)) {
			*cx = i;
			return;
		}
	for (i = 0; i < 6; i++)
		if (c == *("KQRBNP" + i)) {
			*cp = c;
			return;
		}
}

static void xypsearch(int newza, int oldz, char cp, SPE esp) {
	 int i,indexa=index;

	char newz ,newz1, oldz1;
	newz=newza;
	bool capture=false;
	char pp;
	 int match=0;
	 int pcount=0;
	 for (i = 0; i < newkernel->movelist.size(); i++) {
		newz1 = newkernel->movelist[i].newz;
		oldz1 = newkernel->movelist[i].oldz;
		if(newz1==newza ) {
		pp = newkernel->movelist[i].p->type;
		pp = *(".PRNBQK" + pp);
		match++;
		 if(pp=='P') pcount++;
		 }
		}
		if (esp==normalspe && (match>1) )
		if (esp==normalspe && (match>1) && cp<0 && pcount>0 )  {
		  cp='P';		
		}
		if (esp==normalspe && (match>1) && cp<0 && pcount==0 )  {
		  index+=2; return;		
		}

	for (i = 0; i < newkernel->movelist.size(); i++) {
		newz1 = newkernel->movelist[i].newz;
		oldz1 = newkernel->movelist[i].oldz;

		pp = newkernel->movelist[i].p->type;
		pp = *(".PRNBQK" + pp);
 if(newza==0x171)		DoPrintf("oldz1=(%02x %02x %c) IN=(sp=%d oldz=%02x newz=%02x %c) ",
		oldz1,	newz1,cp, 	newkernel->movelist[i].spe, oldz, newz, pp,cp);

			if (esp >0 && newkernel->movelist[i].cap == 0)
			continue;
  if(take && cp<0)cp='P';
  if(oldz>=0) {
		 if((oldz &7) !=(oldz1&7)) continue;
  }
			if(esp==take && newkernel->movelist[i].cap==0) continue;

		if( esp==kingsidecas||esp==queensidecas )  {
			if ( newkernel->movelist[i].spe == esp)
				{moveindexstack[index++] = i;		 return; }
				}
		if (oldz < 0 && cp < 0 && newz1 == newz) {
    if(index>34)
		DoPrintf("Aoldz1=(%02x %02x %c) IN=(sp=%d oldz=%02x newz=%02x %c) ",
		oldz1,	newz1,cp, 	newkernel->movelist[i].spe, oldz, newz, pp,cp);

			moveindexstack[index++] = i;  }

		if (oldz < 0 && cp > 0 && cp == pp && newz1 == newz)
		   {

			moveindexstack[index++] = i;  }
		if (oldz < 0) 		continue;
		if (((oldz1 & 0x0f) == oldz || (oldz1 & 0xf0) == oldz)
			&& cp == pp && newz1 == newz) {

			moveindexstack[index++] = i;
			 if(newkernel->movelist[i].spe==empassant) return;
		}
	}
}

bool pushStack(int n, char *str) {
	if (n != index)
		return false;
	DoPrintf("n=%d istr=%s", n,str);
		newkernel->genmove(newkernel->teban);

	int csp, cx, cy, dummy, cz1,newz;
	char cp, cap, dummyc;
	cx = cy = cp = cz1 = -1;
	// queen side castling
	if (memcmp(str, "O-O-O", 5) == 0) {
		newz=(newkernel->teban==2)?0x03:0x73;
		xypsearch(newz, -1, -1, queensidecas);
		return(n + 1 == index);
	}
	 // king side castling
	else if (memcmp(str, "O-O", 3) == 0) {
			newz=(newkernel->teban==2)?0x06:0x76;
		xypsearch(-1, -1, -1, kingsidecas);
		return(n + 1 == index);
	}

	cx = cy = cp = cz1 = -1;
	csp = 0;
	//
	cxyp(*str++, &cx, &cy, &cp);
	if (cx >= 0) { // 1文字目がｘ座標

		if (*str == 'x') { // 2文字目が take X
             cz1=cx;
			str++;
			cap = 1;
			cxyp(*str++, &cx, &cy, &dummyc);
			if (cx >= 0) { // 3文字目がx座標
				cxyp(*str++, &dummy, &cy, &cp);
				if (cy >= 0) { // 4文字目がY座標

					xypsearch(cy * 16 + cx, cz1, -1, take);
					return(index == n + 1);
				}
			}

		}
		cxyp(*str++, &dummy, &cy, &cp);
		if (cy >= 0) { // ２文字目がY座標　ノーマル
	   //		DoPrintf("to=%02x", cy * 16 + cx);
			xypsearch(cy * 16 + cx, -1, -1, normalspe);
			return(index == (n + 1));
		}


		else if (cp > 0) {  // 1文字目がx ２文字目が駒
			cz1 = cx;
		cx = -1;
		xypsearch(cx + 16 * cy, cz1, cp, normalspe);
		return(index == n + 1);
		}

	}
	else if (cp > 0) { // 1文字目が駒
		if (*str == 'x') { // 2文字目が take X
			str++;

			cxyp(*str++, &cx, &cy, &dummyc);
			if (cx >= 0) { // 3文字目がx座標
				cxyp(*str++, &dummy, &cy, &cp);
				if (cy >= 0) { // 3文字目がY座標
					xypsearch(cy * 16 + cx, -1, cp, take);
					return(index == n + 1);
				}
			}

		}
		cxyp(*str++, &cx, &cy, &cp);
		if (cx >= 0) { // 2文字目がｘ座標
			int cxsave = cx;
			cx = -1;
			cy = -1;
			cxyp(*str++, &cx, &cy, &cp);

			if (cy >= 0) { // 3文字目がY座標
				xypsearch(cy * 16 + cxsave, -1, cp, normalspe); // Kc5
				return(index == n + 1);

			}
			else if (cx >= 0) { // 3文字目が x座標                 //Xcc
				cxyp(*str++, &cx, &cy, &cp);
				if (cy >= 0) // 4文字目が y座標
					DoPrintf("cxsave =%02x z=%02x ", cxsave, cy * 16 + cx);
				xypsearch(cy * 16 + cx, cxsave, cp, normalspe);
					DoPrintf("cxsaveend");
						return(index == n + 1);
			}
		}
		if (cy >= 0) { // 2文字目がy座標
			cz1 = cy * 16;
			cxyp(*str++, &cx, &cy, &cp);

			if (cx >= 0) { // 3文字目がx座標     K7c5
				cxyp(*str++, &dummy, &cy, &dummyc);
				if (cy >= 0) {
					xypsearch(cy * 16 + cx, cz1, cp, normalspe);
				}
			}

		}
	}
	return(index == n + 1);

}

// ---------------------------------------------------------------------------
// チェスの代数方式の棋譜の分解
// n  whiteplay blackplay
int algebdecode(char *str, char *m1, char *m2) {
	int n = atoi(str);
	*m1 = *m2 = 0;
	while (*str != '.') {
		if (*str == 0)
			break;
		str++;
	}
	if (*str == 0)
		return n;
	str++;
	while (*str != ' ') {
		if (*str == 0)
			break;
		*m1++ = *str++;
	}
	*m1 = 0;
	if (*str == 0)
		return n;
	while (*str == ' ') {
		if (*str == 0)
			break;
		str++;
	}
	if (*str == 0)
		return n;
	while (*str != ' ') {
		if (*str == 0)
			break;
		*m2++ = *str++;
	}
	*m2 = 0;
	return n;
}


void __fastcall TForm1::load1Click(TObject *Sender) {
	DoPrintf("load");
	AnsiString A;

		A = "game1.txt"; //  StringGrid1->Col=12;


if(1){
	if (!OpenDialog1->Execute()) 		return;
	A = OpenDialog1->FileName;
	  WriteManager("chessfn",A);
   //  WritePrivateProfileInt("aaaaa",123);
  // Level = (LEVELTYPE)GetPrivateProfileInt("WCHESS", "Level",
  // (int)easygame, INIFile);

	}
	doLoadFile(A);

}
bool doLoadFile(AnsiString FileName){
	int n,nn;
	char algeb[2][10];
	char w[10];
	AnsiString A;
if(!FileExists (FileName)) return false;
	TStringList *T = new TStringList;
	T->LoadFromFile(FileName);
	index = 0;
  Form1->	 StringGrid1->RowCount= T->Count+1;
	for (int i = 0; i < T->Count; i++) {
		A = T->Strings[i];

		nn=n = algebdecode(A.c_str(), algeb[0], algeb[1]);

	Form1->	StringGrid1->	Cells[0][i+1]=IntToStr(n);
		n--;
		n = n * 2;
		for (int ii = 0; ii < 2; ii++, n++) {

			 strcpy(w, algeb[ii]);
		  Form1->	  StringGrid1->	Cells[ii+1][i+1]=w;
 //select
			 if(memcmp(w,"1-0",3)==0) { DoPrintf("White won");goto aa;}
			  if(memcmp(w,"0-1",3)==0) { DoPrintf("Black won");goto aa;}
			  if(memcmp(w,"1/2-1/2",7)==0) { DoPrintf("Draw ");goto aa;}


			if (!pushStack(n, algeb[ii])) {
				if (n == index)
					DoPrintf("n=%d can not move to %s",nn,w);
				else
					DoPrintf("n=%d 曖昧です %s",nn,w);
					nn=50;
					return false;
			}
			int k = moveindexstack[index - 1];

			newkernel->Play(newkernel->movelist[k]);
			 Form1->Repaint();
			 newkernel->record2.push_back(newkernel->movelist[k]);
				MOVETYPE MM;
			convertMove(newkernel->movelist[k],&MM);

			MakeMove(&MM);
			 Form1->		Repaint();
			showStatus( newkernel->teban,newkernel->step);

		}
	aa:	//if(nn>15) return ;

	}
	int k= newkernel->step;
	DoPrintf("size=%d",  newkernel->record.size());



	while(k>2) { newkernel->Back();
				TakeBackMove(&MovTab[Depth]);
	//  TGridRect AA=  StringGrid1->Selection  ;
	 // DoPrintf("x=%d",AA.
		Form1->	Repaint();
				showStatus( newkernel->teban,newkernel->step);
			  k--;

			}
	 return true;
}

// ---------------------------------------------------------------------------
void xx(int n) {
	MOVETYPE MM;
	if (n == newkernel->step)
		return;
	else if (n < newkernel->step) {
		while (newkernel->step > n)
		   {
			 newkernel->Back();
			TakeBackMove(&MovTab[Depth]);}
	}
	else {
		 while (newkernel->step <n)   {
	   //	newkernel->genmove(newkernel->teban);

		int k =newkernel->step;
		 //moveindexstack[newkernel->step-1];
			newkernel->Play(newkernel->record2[k-1]);

			convertMove(newkernel->record2[k-1],&MM);

			MakeMove( &MM);
			 }
	}
	Form1->Repaint();


		}


// ---------------------------------------------------------------------------


void __fastcall TForm1::onkeydown(TObject *Sender, WORD &Key,
	TShiftState Shift) {

	AnsiString B;
	if (Key == VK_RETURN) {
		B = Memo1->Lines->Strings[Memo1->Lines->Count - 1];
	}

	MMM nnn;
	// newkernel->Translate(B.c_str(),&nnn);

}
// ---------------------------------------------------------------------------

void __fastcall TForm1::onenter(TObject *Sender) {
	DoPrintf("onenter");
}
// ---------------------------------------------------------------------------

 extern  MOVETYPE Next;
 extern int MoveNo;

void __fastcall TForm1::undo1Click(TObject *Sender)
{
   DoPrintf("undo");
	newkernel->Back();
		showStatus( newkernel->teban,newkernel->step);

	Repaint();

	TakeBackMove(&MovTab[Depth]);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
	  if(OnSolving){
MyTime=  GetTickCount();
 ShowTime(MyTime-MyTime1); MyTime1=MyTime;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::newgame1Click(TObject *Sender)
{
	 OnSolving=true;
	 MyTime1=GetTickCount(  );
   //	 StringGrid1->Objects[1][1]->ToString();

}
//---------------------------------------------------------------------------


void __fastcall TForm1::cellselected(TObject *Sender, int ACol, int ARow, bool &CanSelect)

{
int	pstep= newkernel->step;
int newstep=(ARow-1)*2+(ACol)+1;
DoPrintf("%d %d pstep%d newstep=%d",ACol,ARow,pstep,newstep);
xx(newstep);
//StringGrid1->Cells[ACol][ARow]  ="KKKK";
}
//---------------------------------------------------------------------------

