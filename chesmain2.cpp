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
TColor BlackSquareColor = clGreen; // ��

FILE *logfile;
//mychess *kernel;
chessrule *newkernel;
bool ShowBestLine = true;
// int          ComputerColor;
short LINESIZE;
short CHARSIZE;
int CAPTIONY;
bool Editing;
int BORDERSIZE;
const BORDERYEXTRA = 4; // �S�s�N�Z���̌���
short INFOXSIZE, INFOYSIZE;
bool SoundOn;

// ----------samll.cpp----------------------------
// VETYPE  ZeroMove = { 8, 8, 0, empty, empty };
//MOVETYPE KeyMove;
int  RunColor;
BOOL Analysis, Opan;
double WantedTime;
BOOL GameOver = FALSE;
char EndGameMessage[80];
int ComputerColor;
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
//ttemp=ttemp2= MRook;
int upsidedown(int z){
	return (z&7) +(0x70-(z&0x70) );
}
// ---------------------------------------------------------------------------
 void FindMove(int maxlevel)  ;
bool movestart;

//bool convertBoard(PIECE*);
//PIECE org[2];


//nt PN = 0;


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
 //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 void showStatus(int t,int s){
		char w[10];
		sprintf(w,"step=%d",s);
 Form1->StatusBar1->Panels->Items[0]->Text = (t==2)?"White":"Blck";
 Form1->StatusBar1->Panels->Items[1]->Text = w;
 }
extern MOVETYPE Next;
// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
	SetCurrentDir("../");
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
	ttemp=0;
	ttemp2=0;
	movestart = false;

	DoubleBuffered = true;
	showStatus(newkernel->teban,newkernel->step);

	 js=new joseki();
	 assert( js->Openings);
	 ResetGame();// board.cpp
	 ListBox1->Clear() ;
	  ListBox1->Items->Add("1 d4 e5");
	   ListBox1->Items->Add("2 d4 e5");
		ListBox1->Items->Add("3 d4 e5");

}

void drawPiece(int x, int y, int pieceid, int color) {
	int xx, xx2, yy, yy2;
   //	if(x==basex) return;
	TColor c = clWhite;
  Graphics::TBitmap *	ttemp = bitmaphd[pieceid - 1][color - 1];
  Graphics::TBitmap *	ttemp2 = bitmaphd[pieceid - 1][2];

	TRect r = Rect(x, y, x + sz, y + sz);
   //	 DoPrintf(" [%d %d] %d %d sz=%d",x,y,pieceid,color,sz);
	// Form1->Canvas->Brush->Color=clRed;
	// Form1->Canvas->Rectangle( 0,0,sz,sz);

	Form1->Canvas->Brush->Bitmap = ttemp2;
	TRect r2 = Rect(0, 0, sz/2 ,sz/2);
	Form1->Canvas->CopyMode = cmSrcAnd;
 	Form1->Canvas->CopyRect(r, ttemp2->Canvas, r2); // maskout

	Form1->Canvas->Brush->Bitmap = ttemp;
	Form1->Canvas->CopyMode = cmSrcPaint;
	Form1->Canvas->CopyRect(r, ttemp->Canvas, r2); // overwrite

}
#define GRAYSIZE 25
void __fastcall TForm1::paint(TObject *Sender) {
	int i, x, y;
	int showx, showy, type, col;
	TColor c;
	if(1) {
		Canvas->Brush->Color = clGray;
		Canvas->Rectangle(basex -GRAYSIZE,basey -GRAYSIZE  ,
				basex + sz * 8 +GRAYSIZE ,basey + sz * 8+ GRAYSIZE);
	}
	   //	Form1->Canvas->Brush->Color = clWhite;

	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++) {

			c = !((x ^ y) & 1) ? clWhite : clGreen  ;
		  //	 if (x == 0 && y == 0) 		c = clRed;
		Canvas->Brush->Color = c;
		Canvas->Rectangle(basex + x * sz, basey + sz * y,
				basex + sz * (x + 1), basey + sz * (y + 1));
		}


	for (x = 0; x < 8; x++)
		for (y = 0; y < 8; y++) {
	   //	if(x==0 && y==0) continue;
			if (!newkernel->getxyrt(x, y, &showx, &showy, &col, &type))
				continue;
		   //	if(col==0) continue;
		  //	  DoPrintf("%d %d &d %d",showx,showy,type,col);
			drawPiece(showx, showy, type, col);
		}
		  Canvas->Font->Color=clBlack;//clNavy;
		  Canvas->Font->Size= 12  ;
			  Canvas->Brush->Color= clGray;

		   char w[5];
		   int ddx1=sz*0.6;
		   int ddx2=sz*0.2;
		   int ddy2=sz*0.3;
	 AnsiString Textx,Texty;
		 for (int i=0;i<8;i++) {
		 w[0]='8'-i;w[1]=0;

		  Textx=w;w[0]='a'+i;
		  Texty=w;
		  Canvas-> TextOut(basex+i*sz +ddx1,basey+8*sz,Texty);
		  Canvas-> TextOut(basex-ddx2,basey+i*sz+ddy2,Textx);

	   }
}


int GetValidSquare2(TPoint p, int player, bool CheckPiece);

// -----mose down----------------------------------------------------------------------
void __fastcall TForm1::onmouse(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y) {
	int color, type;
	if (movestart)
		return;

	TPoint point = TPoint(X, Y);
	int k, n = GetValidSquare2(point, newkernel->teban, true);
	DoPrintf("mousedwn board  [%d %d] n=%02x", X, Y, n);
	if (n < 0)
		return;

	// k=p-piece;

   //	DoPrintf("k=%d type=%d color=%d", k, type, color);
	if (type < 1)  		return; ;
	movestart = newkernel->setCurrentP(n);

}




void __fastcall TForm1::onmoudemove(TObject *Sender, TShiftState Shift, int X,
	int Y)

{

	if (!movestart)
		return;
	newkernel->moveCurrent(X-sz/2, Y-sz/2);

	Repaint();

}

void __fastcall TForm1::onmpouseup(TObject *Sender, TMouseButton Button,
	TShiftState Shift, int X, int Y) {
	int converttype[]={6,3,5,4,2,1};

	if (!movestart)
		return;
	movestart = false;
	TPoint point = TPoint(X, Y);
	int k, n = GetValidSquare2(point, newkernel->teban, true);
	if (n < 0)		return;
	DoPrintf("mouse up %02x %02x ", n,	newkernel->currentP->z);


	if (!newkernel->CheckAndPlay(n))  		DoPrintf("invalid");
	 else{
	 MOVE Move=newkernel->record[newkernel->step-2];
		MOVETYPE move;
		int pp=newkernel->board[n]->type;
move.old=Move.oldz;
move.new1=Move.newz;
move.spe=Move.spe;

move.movpiece= pp;
  if(Move.cap){move.content=Move.cap->type;}
  else move.content=0;
MakeMove(&move );


	 }
	showStatus(newkernel->teban,newkernel->step);
	Repaint();
	return;


}

// ---------------------------------------------------------------------------

int GetValidSquare2(TPoint p, int player, bool CheckPiece) {
	int x, y;
	x = (p.x - basex) / sz;
	y = (p.y - basey) / sz;
	if (x < 0 || x > 7 || y < 0 || y > 7)
		return -1;
	return 16 * y + x;
}

// ---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
	DoPrintf("computer play");
	MOVETYPE move;
	MOVE Move;
	move=js->FindOpeningMove();
	if(move.new1<0x88){
   //	  DoPrintf(" FT= %02x %03x", move.old, move.new1);
   MakeMove(&move);
	Move.newz=upsidedown(move.new1);
	Move.oldz=upsidedown(move.old);
	Move.spe=move.spe;
	Move.p=newkernel->board[Move.oldz];
	Move.cap=0;
	}
	else  {
	Move.newz=8;
	}
	if(newkernel->board[ move.new1]) Move.cap=newkernel->board[Move.newz];
	newkernel->Play(Move) ;
	showStatus(newkernel->teban,newkernel->step);

	Repaint();

}
int index,moveindexstack[40];
static  void cxyp(char c,int *cx,int *cy, char *cp){
	int i;
	for(i=0;i<8;i++) if (c== *("12345678"+i)) {*cy=i;return;}
	for(i=0;i<8;i++) if (c== *("abcdefgh"+i)) {*cx=i;return;}
	for(i=0;i<6;i++) if (c== *("KQRBNP"+i)) {*cp=c;return;}
}
static  void xypsearch(int newz,int oldz,char cp,int esp)  {
	newkernel->genmove(newkernel->teban);
	 int newz1,oldz1; char pp;
	 newz=(newz&7)+ 16*(7-newz/16);    //chessruuke invert
	 if (oldz >=16) oldz=16*(7-oldz/16);
	for(int i=0;i<newkernel->movelist.size();i++){
	newz1= newkernel->movelist[i].newz;
	oldz1=newkernel->movelist[i].oldz ;
	 if(esp==100 && newkernel->movelist[i].cap==0) continue;
	DoPrintf("oldz1=%02x newz1=%02x oldz=%02x newz=%02x ",
	oldz1,newz1,oldz,newz);
	 pp=   newkernel->movelist[i].p->type;
	  pp =*(".PRNBQK"+pp);

	  if(esp>0 && newkernel->movelist[i].spe==esp)
	  moveindexstack[index++]=i;
	  if(oldz<0 && cp<0 && newz1==newz)
	   moveindexstack[index++]=i;
	  if(oldz<0 && cp>0 && cp==pp && newz1==newz)
	   moveindexstack[index++]=i;
	   if(oldz<0) continue;
	   if( ((oldz1&0x0f)==oldz  ) || ((oldz1&0xf0)==oldz) && cp==pp &&
	   newz1==newz)  {
			  DoPrintf("$$$$$$$$$$$i=%d",i);
			moveindexstack[index++]=i;   }
	   }
}
bool pushStack(int n,char *str){
	   if(n!=index) return false;
	   DoPrintf("istr=%s",str);
	   int csp,cx,cy,dummy,cz1;
	   char cp,cap,dummyc;
	   cx=cy=cp=cz1=-1;
  //castling
	   if(memcmp(str ,"0-0",3)==0)  {
		xypsearch(-1,-1,-1,1);return (n+1==index);
	   }
	   if(memcmp(str ,"0-0-0",5)==0)  {
		xypsearch(-1,-1,-1,2);return (n+1==index);
	   }
	   cx=cy=cp=cz1=-1;   csp=0;
  //
		cxyp(*str++,&cx,&cy,&cp);
		if(cx>=0){  //1�����ڂ������W
		cxyp(*str++,&dummy,&cy,&cp);
		if(cy>=0) {  //�Q�����ڂ�Y���W�@�m�[�}��
		 DoPrintf("to=%02x",cy*16+cx);
		 xypsearch(cy*16+cx,-1,-1,0);
		 return (index==(n+1));
		 }
		else if(cp>0) //1�����ڂ�x �Q�����ڂ���
			 cz1=cx;cx=-1;
			  xypsearch(cx+16*cy, cz1,cp,0);
			   return (index==n+1);
		}
		else if(cp>0) {   //1�����ڂ���
		  if(*str=='x'){   //2�����ڂ� take X
			 str++;cap=1;
			cxyp(*str++,&cx,&cy,&dummyc);
				if(cx>=0) {  //3�����ڂ�x���W�@
				cxyp(*str++,&dummy,&cy,&cp);
				if(cy>=0) {  //3�����ڂ�Y���W�@
				 xypsearch(cy*16+cx,-1,-1,100);
				  return (index==n+1);
				 }}


			 }
		 cxyp(*str++,&cx,&cy,&cp);
		if(cx>=0){  //2�����ڂ������W
		int cxsave=cx;  cx=-1;  cy=-1;
		cxyp(*str++,&cx,&cy,&cp);

		if(cy>=0) {  //3�����ڂ�Y���W
		 xypsearch(cy*16+cxsave,-1,cp,0);   //Kc5
		   return (index==n+1);

		  }
		 else  if(cx>=0){//3�����ڂ� x���W                 //Xcc
				  cxyp(*str++,&cx,&cy,&cp);
				  if(cy>=0)  //4�����ڂ� y���W
				  DoPrintf("cxsave =%02x z=%02x ",cxsave,cy*16+cx);
				  xypsearch(cy*16+cx,cxsave,-1,0);
		  }
		}
		if(cy>=0){  //2�����ڂ�y���W
			  cz1=cy*16;
		cxyp(*str++,&cx,&cy,&cp);

		  if(cx>=0){  //3�����ڂ�x���W     K7c5
			cxyp(*str++,&dummy,&cy,&dummyc);
			if(cy>=0) {xypsearch(cy*16+cx,cz1,cp,0);}
			}

		 }
	 }
	 return (index==n+1);

  }


//---------------------------------------------------------------------------
//�`�F�X�̑㐔�����̊����̕���
//n  whiteplay blackplay
 int algebdecode(char *str,char *m1,char *m2){
	 int n=atoi(str);
	 *m1=*m2=0;
	 while(*str!='.') {if(*str==0) break;str++;}
	 if(*str==0) return n;
	 str++;
	  while(*str!=' ') {if(*str==0) break;*m1++=*str++;} *m1=0;
	  if(*str==0) return n;
	 while(*str==' ') {if(*str==0) break;str++;}
		if(*str==0) return n;
	  while(*str!=' ') {if(*str==0) break;*m2++=*str++;} *m2=0;
	  return n;
 }
void __fastcall TForm1::load1Click(TObject *Sender)
{
   DoPrintf("load");
	AnsiString A;
   int n; char algeb[2][10];
   //	if (!OpenDialog1->Execute()) 		return;
	ListBox1->Clear();
   //	A = OpenDialog1->FileName;
   A="game1.txt";
	TStringList *T= new TStringList;
	T->LoadFromFile(A) ;
	index=0;
	for (int i=0; i <T->Count; i++) {
		A = T->Strings[i];

		n=algebdecode( A.c_str(),algeb[0],algeb[1]);
		n--;
		n=n*2;
		for(int ii=0;ii<2;ii++,n++){

			DoPrintf("i=%d ii=%d  n=%d %s ",i,ii,n,algeb[ii]);
		if(!pushStack(n, algeb[ii])) {
		  if(n==index) DoPrintf("can not move to");
		  else DoPrintf("�B���ł�");
	 //	DoPrintf("n=%d index=%d step=%d",2*n+1,index,	newkernel->step);
		goto aa;}
			int k=  moveindexstack[index-1];
		DoPrintf("k=%d from=%02x to=%02x", k,
			newkernel-> movelist[k].oldz  ,
			 newkernel-> movelist[k].newz );
		 newkernel->Play( 	newkernel-> movelist[k]);
		 Repaint();
		}

		ListBox1->Items->Add(A);

	}
   aa:
}
//---------------------------------------------------------------------------
  void xx(int n ) {
	  if(n==index) return;
	  if(n<index){
		 while(newkernel->step>n) index--,newkernel->Back();}
	  else {
		  newkernel->genmove(newkernel->teban);
		  int k=  moveindexstack[index++];
		  newkernel->Play( 	newkernel-> movelist[k]);
	   }
  }

void __fastcall TForm1::listbox1click(TObject *Sender)
{
	int k=ListBox1->ItemIndex;

AnsiString A= ListBox1->Items->Strings[k];
	 char *str=A.c_str();
	 int n=atoi(str);
	 if(*str!=' ') str++;  if(*str!=' ') str++;
	 while(*str==' ') {if(*str==0) break;str++;}
	 xx( 2*n-2);
   //	 newkernel->Tranlate(str);

	DoPrintf("single click %s",A.c_str());
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

void __fastcall TForm1::doubleclick(TObject *Sender)
{
		int k=ListBox1->ItemIndex; // DoPrintf("double click %d",k);
		AnsiString A= ListBox1->Items->Strings[k];
		  MMM nnn;
		 char *str=A.c_str();
   char *str1;
	 int n=atoi(str);
	while(*str!=' ') {if(*str==0) break;str++;}
	 while(*str==' ') {if(*str==0) break;  str++;}
	 str1=str;
		while(*str!=' ') {if(*str==0) break;str++;}
	  while(*str==' ') {if(*str==0) break;  str++;}
	xx( 2*n-1);
		 newkernel->Translate(str,&nnn);
		  DoPrintf("B=%s str=%s n=%d",A.c_str(),str,n);


}
//---------------------------------------------------------------------------

void __fastcall TForm1::onkeydown(TObject *Sender, WORD &Key, TShiftState Shift)
{

   AnsiString B;
  if(Key==VK_RETURN){
  B= Memo1->Lines->Strings[Memo1->Lines->Count-1];
    }

  MMM nnn;
 // newkernel->Translate(B.c_str(),&nnn);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::onenter(TObject *Sender)
{
	 DoPrintf("onenter");
}
//---------------------------------------------------------------------------
