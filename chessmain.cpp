//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "chessmain.h"
#include <stdio.h>

#include "wcdefs.h"
#include "externs.h"



//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TColor WhiteSquareColor =clWhite;
TColor BlackSquareColor= clGreen;      // ��
TForm1 *Form1;
FILE *logfile;
void Talk();

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


BOOL         ShowBestLine = true;




//int          ComputerColor;
short        LINESIZE;
short        CHARSIZE;
int          CAPTIONY;
BOOL         Editing;
int          BORDERSIZE;
const        BORDERYEXTRA = 4; // �S�s�N�Z���̌���
short        INFOXSIZE, INFOYSIZE;
BOOL         SoundOn;
 int basex,basey,sz;
 int teban,step;
//----------samll.cpp
MOVETYPE  ZeroMove = { 8, 8, 0, empty, empty };
MOVETYPE  KeyMove;
BOOL      Running;
COLORTYPE RunColor;
BOOL      Analysis, Opan;
double    WantedTime;
BOOL      GameOver = FALSE;
char      EndGameMessage[80]  ;
COLORTYPE    ComputerColor;

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
Graphics::TBitmap *ttemp,*ttemp2;
Graphics::TBitmap *bitmaphd[6][3];
 PIECE * boardmap[8][8] ;

 int dirmap[15][15]={
    {7,0,0,0,0,0,0,1,0,0,0,0,0,0,5},
    {0,7,0,0,0,0,0,1,0,0,0,0,0,5,0},
    {0,0,7,0,0,0,0,1,0,0,0,0,5,0,0},
    {0,0,0,7,0,0,0,1,0,0,0,5,0,0,0},
    {0,0,0,0,7,0,0,1,0,0,5,0,0,0,0},
    {0,0,0,0,0,7,0,1,0,5,0,0,0,0,0},
    {0,0,0,0,0,0,7,1,5,0,0,0,0,0,0},
    {4,4,4,4,4,4,4,0,3,3,3,3,3,3,3},
    {0,0,0,0,0,0,8,2,6,0,0,0,0,0,0},
    {0,0,0,0,0,8,0,2,0,6,0,0,0,0,0},
    {0,0,0,0,8,0,0,2,0,0,6,0,0,0,0},
    {0,0,0,8,0,0,0,2,0,0,0,6,0,0,0},
    {0,0,8,0,0,0,0,2,0,0,0,0,6,0,0},
    {0,8,0,0,0,0,0,2,0,0,0,0,0,6,0},
    {8,0,0,0,0,0,0,2,0,0,0,0,0,0,6},
 };
 bool movestart;
//---------------------------------------------------------------------------
 PIECE piece[33];
 MOVEPACK moveTable[100];
 __int64 hashrandom[8][8][12];

 int currentP;
 bool Play(MOVEPACK *);
 void Back();
 bool validMove(MOVEPACK *);
 bool convertBoard(PIECE *);
 PIECE org[2];
void clearBoard();
void  ResetGame();
 void addPiece(int x,int y,char c);
int  moveListPlayer(int);
int getNextMoves(int ,MOVEPACK *);
 bool  checkStatus(int color) ;
 __int64 hashcode;
 void initHash(){

 int t,x,y,k;
       __int64 z;
       for (x=0;x<8;x++) for (y=0;y<8;y++) for (k=0;k<6;k++){
       z= rand();z<<16;z=z+rand(); z<<=16; z=z+rand();
        hashrandom[x][y][k]=z;

       }
       hashcode=0;
        PIECE *p,*q;
        for (t=0;t<2;t++) {
           q=org+t;
           for (p=q;p=p->next,p!=q;)
           hashcode+= hashrandom[p->x][p->y][p->type-1+t*6];

        }
 }
void inializeBoard(){
     int x,y,i,ii;

       piece[32].type=0;
       piece[32].color=0;
       clearBoard();
        addPiece(4,1,'K');
        addPiece(4,8,'k');
     for (i=0;i<8;i++) {
       addPiece(i+1,2,'P');
       addPiece(i+1,7,'p');

     }

        addPiece(1,1,'R');
        addPiece(2,1,'N');
         addPiece(3,1,'B');


        addPiece(5,1,'Q');
        addPiece(6,1,'B');
         addPiece(7,1,'N');
        addPiece(8,1,'R');
        //
         addPiece(1,8,'r');
        addPiece(2,8,'n');
         addPiece(3,8,'b');


        addPiece(5,8,'q');
        addPiece(6,8,'b');
         addPiece(7,8,'n');
        addPiece(8,8,'r');
      ResetGame();

}
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
  BPawn->LoadFromFile("BPawn.bmp");   bitmaphd[0][0]=BPawn;
  WPawn->LoadFromFile("WPawn.bmp");   bitmaphd[0][1]=WPawn;
  MPawn->LoadFromFile("PMask.bmp");   bitmaphd[0][2]=MPawn;

 BRook->LoadFromFile("BRook.bmp");    bitmaphd[1][0]=BRook;
 WRook->LoadFromFile("WRook.bmp");    bitmaphd[1][1]=WRook;
  MRook->LoadFromFile("RMask.bmp");   bitmaphd[1][2]=MRook;

 BKnight->LoadFromFile("BKngt.bmp"); bitmaphd[2][0]=BKnight;
 WKnight->LoadFromFile("WKngt.bmp");  bitmaphd[2][1]=WKnight;
  MKnight->LoadFromFile("KTMask.bmp"); bitmaphd[2][2]=MKnight;

 BBishop->LoadFromFile("BBishop.bmp"); bitmaphd[3][0]=BBishop;
 WBishop->LoadFromFile("WBishop.bmp"); bitmaphd[3][1]=WBishop;
 MBishop->LoadFromFile("BMask.bmp");   bitmaphd[3][2]=MBishop;
 BQueen->LoadFromFile("BQueen.bmp");   bitmaphd[4][0]=BQueen;
 WQueen->LoadFromFile("WQueen.bmp");   bitmaphd[4][1]=WQueen;
 MQueen->LoadFromFile("QMask.bmp");    bitmaphd[4][2]=MQueen;

 BKing->LoadFromFile("BKing.bmp");     bitmaphd[5][0]=BKing;
 WKing->LoadFromFile("WKing.bmp");      bitmaphd[5][1]=WKing;
 MKing->LoadFromFile("KMask.bmp");       bitmaphd[5][2]=MKing;

   basex=basey=72;
     sz=36;
    inializeBoard();
    teban=2;
    movestart=false;
    step=1;
    DoubleBuffered=true;
    StatusBar1->Panels->Items[0]->Text="gggg";
    Talk();
}

 void drawPiece(int x,int y,int pieceid ,int color){
  int xx,xx2,yy,yy2;

    ttemp=  bitmaphd[pieceid-1][color-1];
    ttemp2= bitmaphd[pieceid-1][2];
     TRect r=Rect(x,y,x+sz,y+sz);
     TRect r2=Rect(0,0,sz,sz);
  Form1->Canvas->Brush->Bitmap=ttemp2;
  Form1->Canvas->CopyMode= cmSrcAnd;
  Form1->Canvas->CopyRect(r,ttemp2->Canvas,r2);
;
  Form1->Canvas->Brush->Bitmap=ttemp;
  Form1->Canvas->CopyMode= cmSrcPaint;
  Form1->Canvas->CopyRect(r,ttemp->Canvas,r2);


}
void PrintBoard(){
    int i,x,y;
    TColor c;


    for (x=0;x<8;x++) for (y=0;y<8;y++) {
        c= !((x^y)&1)?clWhite:clGreen;
        Form1->Canvas->Brush->Color=c;
        Form1->Canvas->Rectangle( basex+ x*sz,basey+sz*y,
        basex+sz*(x+1),basey+sz*(y+1));
    }

    for (i=0;i<32;i++) {
     x=piece[i].showx;
     y=piece[i].showy;

     if (piece[i].color<1) continue;

     drawPiece(x,y,piece[i].type,piece[i].color);
    }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::paint(TObject *Sender)
{

 //  DrawFrame(dc, MainWndRect);
 // DrawFrame(dc, InfoAreaRect);
  PrintBoard();

}
//---------------------------------------------------------------------------
void __fastcall TForm1::dragdrop(TObject *Sender, TObject *Source, int X,
      int Y)
{
    DoPrintf("drop %d %d",X,Y);
}
int GetValidSquare(TPoint p, COLORTYPE player, BOOL CheckPiece) ;

//---------------------------------------------------------------------------
void __fastcall TForm1::mousedown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
     int color,type;
     if (movestart) return ;
     PIECE *p;
     TPoint point= TPoint(X,Y);
    int k,n=  GetValidSquare(point,teban,true);
     DoPrintf("mousedwn %d %d n=%d",X,Y,n);
     if (n<0) return ;
     MOVEPACK *m=moveTable+step;
     p=boardmap[n%8][n/8];
     color= p->color;
     type= p->type;
  //   if (k<0) DoPrintf("empty");
     if (color==0) return ;
 //    if (color!=teban) DoPrintf("illegal teban");
     if (color!=teban) return ;

  //  DoPrintf("k=%d type=%d color=%d",k,type,color);
    if (type<1) return ;;
    movestart=true;
    m->oldx=n%8;
    m->oldy=n/8;
    m->type=type;
    m->color=color;
    currentP=k;


}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button1Click(TObject *Sender)
{

Graphics::TBitmap *BrushBmp = new Graphics::TBitmap;

try
{
  BrushBmp->LoadFromFile("WPawn.bmp");
  Form1->Canvas->Brush->Bitmap = BrushBmp;
  Form1->Canvas->FillRect(Rect(0,0,32,32));

}
__finally
{
  Form1->Canvas->Brush->Bitmap = NULL;
  delete BrushBmp;
}
}
//---------------------------------------------------------------------------

TPoint
GetSquareXY(SQUARETYPE square)
{
  if (Turned)
    square ^= 0x77;
  return TPoint((square % 8) * SQUARE_SIZE + BORDERSIZE + MYFRAMESIZE,
                (7 - square / 16) * SQUARE_SIZE + BORDERSIZE + MYFRAMESIZE);
}

 int
GetValidSquare(TPoint p, COLORTYPE player, BOOL CheckPiece)
{
   int x,y;
   x=(p.x-basex)/36;
   y=(p.y-basey)/36;
   if (x<0 || x>7|| y<0 ||y>7)  return -1;
   return 8*y+x;
}


void __fastcall TForm1::mouseup(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
if (!movestart) return;
     movestart=false;
    TPoint point= TPoint(X,Y);
    int k,n=  GetValidSquare(point,teban,true);
    MOVEPACK *m=moveTable+step;
     if (n<0) return ;
     PIECE*p;
     p=boardmap[n%8][n/8];

     if (p->color==teban) return;

     m->newx=n%8;
     m->newy=n/8;
     m->cappiece=0;
     if (p->type)  m->cappiece=p;
     SQUARETYPE s1,s2;
     s1=(7-m->oldy)*16+m->oldx;
     s2=(7-m->newy)*16+m->newx;
   
     bool mc=MoveCheck(s1,s2);
     if (mc) DoPrintf("mc valid");
     if (!validMove(m)) {
     piece[currentP].showx=basex+sz*m->oldx;
      piece[currentP].showy=basex+sz*m->oldy;
      Repaint();
     return ;}
     DoPrintf("valid");

     Play(m);
     Repaint();
}

//---------------------------------------------------------------------------
int direction(int oldx,int oldy , int newx, int newy){
      int difx,dify;
      if (oldx==newx && newy==oldy) return 0;
//horizontal vertical
      if (oldx==newx ) {
          if (oldy>newy) return 1; return 2;

      }
      if (oldy==newy ) {
          if (oldx<newx) return 3; return 4;
      }
      difx=abs(oldx-newx) ;
      dify=abs(oldy-newy);
    //knight move
      if (difx==1 && dify==2) return 10;
      if (difx==2 && dify==1) return 10;
      if (difx!=dify) return 0;
    //diagonal
       if( oldx<newx && oldy>newy) return 5;
       if( oldx<newx && oldy<newy) return 6;
       if( oldx>newx && oldy>newy) return 7;
       if( oldx>newx && oldy<newy) return 8;
      return 0;}

 int diftablex[]={0,0,0,1,-1,1,1,-1,-1};
 int diftabley[]={0,-1,1,0,0,-1,1,-1,1};

 bool empTest(MOVEPACK *movelast, MOVEPACK *move){
       if (movelast->type!=1)return false;
       if (abs(movelast->oldy-movelast->newy)!=2) return false;
       if (move->newy==(movelast->oldy+movelast->newy)/2) return true;
       return false;
 }

 bool validMove(MOVEPACK *move){
        int oldx =move->oldx;

        int oldy=move->oldy;

        int type=move->type;

        int color=move->color;

        int newx=move->newx;
        int newy=move->newy;
        int type2= (move->cappiece)?0:move->cappiece->type;
  int dir,posx,posy,dif;
  bool empsn;
        dir=direction(oldx,oldy,newx,newy);

        if (type==3 ) return (dir==10);
        if (dir==0|| dir>8) return false;
   //pawn
         if (type==1) {
         dif=abs(oldy-newy);
              empsn=empTest(&moveTable[step-1] ,&moveTable[step]);
              if (empsn) moveTable[step].esp=1;
               if (type2>=0 || empsn) {
                 if (dif!=1) return false;
                 if (color==2 && (dir==5 || dir==7)) return true;
                 if (color==1 && (dir==6 || dir==8)) return true;
                 return false;
               }
               if (dif>2 || type2>0 ) return false;
               if (color==1 && dir!=2) return false;
               if (color==2 && dir!=1) return false;

         }

        if (type==2 && dir>4) return false;
        if (type==4 && dir<=4) return false;
        posx=oldx;posy=oldy;
        do {
        posx+=diftablex[dir];
        posy+=diftabley[dir];

        if (posx==newx && posy==newy) return true;
        if (type==6) return false;
         } while(boardmap[posx][posy]<0);
        return false ;


  }
 void basicPlay(MOVEPACK *m){
  PIECE*p= boardmap[m->oldx][m->oldy] ;
  PIECE*p2=boardmap[m->newx][m->newy] ;

     if (p2->type>0) {  DEQ(p2);}

     boardmap[m->oldx][m->oldy]=&piece[32];
     boardmap[m->newx][m->newy]=p;

       p->x=m->newx;
       p->y=m->newy;
       if (m->esp>0) p->type=m->esp;


 }
 void basicBack( ){}
 bool Play(MOVEPACK *m) {
     PIECE *p,*p3;
     int k,k3;

     basicPlay(m);
    if (checkStatus(3-teban)){
     basicBack();
     return false;



     }

     k= p->type;
     if (p->color==2)k+=6;


     hashcode^=hashrandom[m->oldx][m->oldy][k];
     hashcode^=hashrandom[m->newx][m->newy][k];

     if (m->cappiece) {
     k3= m->cappiece->type;
     if ( m->cappiece->color==2) k3+=6;
       hashcode^=hashrandom[m-cappiece>x][m->cappiece->y][k3];
     }
     teban=3-teban;
     step++;
       return true;
  }

  void basicBack(MOVEPACK *m){
    int index,newx,newy,oldx,oldy,capindex;
    newx=m->newx;
    newy=m->newy;
    oldx=m->oldx;
    oldy=m->oldy;

    if (m->cappiece){ENQ(m->prev,m->cappiece);}
    index=  boardmap[newx][newy];
    capindex= moveTable[step-1].capindex;
    boardmap[oldx][oldy]=index;
    piece[index].x=oldx;
    piece[index].y=oldy;
    if ( moveTable[step-1].esp>0) piece[index].type=1;
    boardmap[newx][newy]=capindex;

    teban=3-teban;
    step--;

  }
  void Back(){
  MOVEPACK *m=  &moveTable[step-1];
  basicBack(m);

    k= p->type;
     if (p->color==2)k+=6;


     hashcode^=hashrandom[m->oldx][m->oldy][k];
     hashcode^=hashrandom[m->newx][m->newy][k];

     if (m->cappiece) {
     k3= m->cappiece->type;
     if ( m->cappiece->color==2) k3+=6;
       hashcode^=hashrandom[m-cappiece>x][m->cappiece->y][k3];
     }
   teban=3-teban;
    step--;


  }
void __fastcall TForm1::mousemove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
    if (!movestart) return;
     piece[currentP].showx=X-16;
       piece[currentP].showy=Y-16;
       Repaint();

}
//---------------------------------------------------------------------------
 void clearBoard(){
     int x,y;
     for (x=0;x<8;x++) for (y=0;y<8;y++) boardmap[x][y]=&piece[32];
     for (x=0;x<32;x++) piece[x].color=-1;
     INITQ(&org[0]);
     INITQ(&org[1]);
 }
 void addPiece(int x,int y,char c) {
 int i,type,color;
   for (i=0;i<32;i++) if (piece[i].color==-1) break;
     switch(c) {
     case 'P': color=1;type=1;break;
     case 'p': color=2;type=1;break;
     case 'R': color=1;type=2;break;
     case 'r': color=2;type=2;break;
     case 'N': color=1;type=3;break;
     case 'n': color=2;type=3;break;
     case 'B': color=1;type=4;break;
     case 'b': color=2;type=4;break;
     case 'Q': color=1;type=5;break;
     case 'q': color=2;type=5;break;
     case 'K': color=1;type=6;break;
     case 'k': color=2;type=6;break;
   }
   piece[i].x=x-1; piece[i].y=y-1;
   piece[i].type=type; piece[i].color=color;
   piece[i].showx= basex+sz*(x-1);
   piece[i].showy= basex+sz*(y-1);
   ENQ(&org[color-1],&piece[i]);
   boardmap[x-1][y-1]=&piece[i];
 }
void __fastcall TForm1::load1Click(TObject *Sender)
{
    if(!OpenDialog1->Execute()) return ;
    DoPrintf("%s", OpenDialog1->FileName);
   int x,y;
   char c;
   clearBoard();
    TStringList *p=new TStringList;
    p->LoadFromFile( OpenDialog1->FileName);
    teban==p->Strings[0][6]-'0';
    if (teban!=1) teban==2;
    for (y=1;y<=8;y++) {
    for (x=1;x<=8;x++) {
    c=p->Strings[y][x];
        if (c=='K'|| c=='k')   addPiece(x,y,c);


     }
    }
     for (y=1;y<=8;y++) {

     for (x=1;x<=8;x++) {
        c=p->Strings[y][x];
        if (!(c=='K' || c=='k' || c=='.'))   addPiece(x,y,c);


     }
    }
    delete p;
  moveListPlayer(teban);
  if (checkStatus(3-teban)) DoPrintf("gameover");
     Repaint();

}
//---------------------------------------------------------------------------
int MoveCount;
MOVEPACK mpack[64];
int knightdirx[]={1,2,2,1,-1,-2,-2,-1};
int knightdiry[]={-2,-1,1,2,2,1,-1,-2};
int kingdirx[]={-1,-1,-1,0,1,1,1,0};
int kingdiry[]={-1,0,1,1,1,0,-1,-1};

MOVEPACK *moveBuffer;
bool check( MOVEPACK m);
void initGenMove(MOVEPACK *m){ moveBuffer=m;     MoveCount=0;}
int getColor(int x,int y) {
   int index;
       if (x<0||x>7 ||y<0 ||y>7) return -1;
       index= boardmap[x][y] ;
       if (index<0) return 0;
        return piece[index].color;
}

bool addGenMove( MOVEPACK m)  {
        int color2=  getColor(  m.newx ,m.newy);
        if (color2<0 || color2==m.color) return false;

      if ( check(m) ) return false;
           moveBuffer[MoveCount++]=m;
           return color2==0;
           }

 void addMoveDirection(int dir, MOVEPACK m ) {
      m.newx=m.oldx;
      m.newy=m.oldy;
      int index,color2;
      do {
         m.newx+=diftablex[dir] ;
         m.newy+=diftabley[dir] ;
       } while(addGenMove(m));
 }
 void addPawnGenMove(MOVEPACK M){
 if (M.newy==0 || M.newy==7) {
     M.esp=5;addGenMove(M);
      M.esp=3;addGenMove(M);
 }
 else { M.esp=0;
                 addGenMove(M);}

 }
void moveGen(int type,int color,int oldx,int oldy){
 MOVEPACK M;
 int dx,dy,color2;
 M.oldx=oldx;M.oldy=oldy;M.type=type;M.color=color;
 switch(type) {
 int i;
       case 1:
        dy=(color==1)?1:-1;
        if (boardmap[oldx][oldy+dy]<0)  {
         M.newx=oldx;M.newy=oldy+dy;
          color2=getColor( M.newx,M.newy);
          if (color2==0) {
          addPawnGenMove(M);
           color2=getColor( M.newx,M.newy+dy);
           if (color2==0)   addPawnGenMove(M);
          }
        }
       if (getColor(oldx-1,oldy+dy)==3-color){ M.newx=oldx-1;M.newy=oldy+dy;addPawnGenMove(M);}
       if (getColor(oldx+1,oldy+dy)==3-color){ M.newx=oldx+1;M.newy=oldy+dy;addPawnGenMove(M);}
         break;

       case 2:
         for (i=1;i<=4;i++)  addMoveDirection(i,M);  break;
       case 4:
         for (i=5;i<=8;i++)  addMoveDirection(i,M);  break;
       case 5:
         for (i=1;i<=8;i++)  addMoveDirection(i,M);  break;
       case 6:
         for (i=0;i<8;i++)  {
              M.newx=oldx+kingdirx[i];   M.newy=oldy+kingdiry[i];
              addGenMove(M);
         }
         break;

       case 3:
         for (i=0;i<8;i++)  {
              M.newx=oldx+knightdirx[i];   M.newy=oldy+knightdiry[i];
              addGenMove(M);
         }
         break;


 }

}
int moveListPlayer(int color) {
      initGenMove(mpack);
      PIECE *p;
      for ( p=&org[color-1];p=p->next,p!=&org[color-1];)
      {

      moveGen(p->type,color,p->x,p->y);
   //     DoPrintf("[%d %d]==>[%d %d] %d",p->x+1,p->y+1,p->type);
      }

     DoPrintf("movecount=%d",MoveCount);

}
int getNextMoves(int color,MOVEPACK *m) {

}
void __fastcall TForm1::game1Click(TObject *Sender)
{
    DoPrintf("test");
    moveListPlayer(teban);

    int i;
    for (i=0;i<MoveCount;i++) {
     DoPrintf("[%d %d]==>[%d %d]", mpack[i].oldx+1, mpack[i].oldy+1,
     mpack[i].newx+1,mpack[i].newy+1);

    }
    //computerplay
  ProgramMove();  


}
//---------------------------------------------------------------------------
 bool attack(int x,int y,PIECE*p) {
          int dx,dy,dir;
          int pawndir[2]={-1,1};
          dx= x-p->x;dy=y-p->y;
          if (p->type==3) return (abs(dx)==2 && abs(dy)==1)   ||
             abs(dx==1 && abs(dy)==2) ;
          if (p->type==1) return (abs(dx==1) && dy==pawndir[p->color-1]);
          dir= dirmap[dx+7][dy+7];
          if (dir==0) return false;
          if (p->type==2 && dir>4)  return false;
          if (p->type==4 && dir<5)  return false;


        do {
        x+=diftablex[dir];
        y+=diftabley[dir];

        if (x==p->x && y==p->y) return true;
        if (p->type==6) return false;
         } while(boardmap[x][y]<0);
        return false;
 }


bool  checkStatus(int color) {
      PIECE*king= org[color-1]. next;
      PIECE *p,*q;
      q= &org[2-color] ;
      for (p=q;p=p->next,p!=q;){
      if (attack(king->x,king->y,p)) return true;;
      }
      return false;
}
