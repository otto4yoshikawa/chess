//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <vcl.h>

#include <fstream.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <time.h>
#include <ctype.h>
#include "wchess.h"
#include "wcdefs.h"
#include "externs.h"
 void DoPrintf(char *szFormat,...);
//
//  グローバル宣言
//
char *INIFile = "wchess.ini";
BOOL InLibrary;      // プログラムがオープニングライブラリ中の時は True
BOOL MultiMove,      // 複数移動の時は True
     AutoPlay,       // 自動実行モードの時は True
     SingleStep;     // デバッグの目的のために使われるシングルステップモード
LEVELTYPE Level;
double AverageTime = 5.0;
PIECETYPE Pieces[8]  = { rook, knight, bishop, queen,
                         king, bishop, knight, rook};
BOOL MaxLevel;
BOOL Turned;
BOOL UseLib;
LIBTYPE Openings;
COLORTYPE ProgramColor;
int MoveNo;
int PVTable[2][7][0x78];
MOVETYPE PlayerMove;
BOOL Logging;
NODEVAL Nodes;             // 解析したノード数
CLOCKTYPE ChessClock;
extern int LegalMoves;
LINETYPE HintLine;            //  提案したヒント
MAXTYPE HintEvalu;            //  ヒントの評価値
enum ANALYSISCONTROL { Start, Return, Continue };

int OpCount, LibNo;

static DEPTHTYPE LibDepth;
static BOOL Found;
static DEPTHTYPE dep;
BOOL
IllegalMove(MOVETYPE* move);

ofstream *OutputFile;

enum CONTROLVAR { readmove, checkmove, gamemove};
static CONTROLVAR Control;


void
InsertPiece(PIECETYPE p, COLORTYPE c, SQUARETYPE sq)
{
  Board[sq].piece = p;
  Board[sq].color = c;
}


void
ClearPVTable()
{
  int  color;
  int  piece;
  SQUARETYPE square;

  for (color = white; color <= black; color++)
    for (piece = king; piece <= pawn; piece++)
      for (square = 0; square <= 0x77; square++)
        PVTable[color][piece][square] = 0;
}

void
ResetGame()
{
  ClearBoard();
 
  for (int i = 0; i < 8; i++) {
    InsertPiece(Pieces[i], white, i);
    InsertPiece(pawn, white, i + 0x10);
    InsertPiece(pawn, black, i + 0x60);
    InsertPiece(Pieces[i], black, i + 0x70);
  }
  CalcPieceTab();
  Player = white;
  ClearDisplay();
  InitDisplay();
  ColorToPlay(Player);
  Opponent = black;
}


void
NewGame()
{
  SingleStep = InLibrary = FALSE;
  GameOver = FALSE;
  ResetGame();
  PrintCurLevel();
  ResetMoves();
  if (!*Openings)    UseLib = 0;
  else     UseLib = 200;
  MovTab[-1].content = king;
//  InitChessTime();
  ProgramColor = white;
  MoveNo = 0;
 // ClearHint();
  ClearPVTable();
  PlayerMove = ZeroMove;
  if (Logging && !AutoPlay)
    *OutputFile << "\n No  Player Program    Hint    Value Level     Nodes  Time\n";
  InitNode(&Nodes);
  ChessClock.totaltime = 0.0;
  Control = (AutoPlay) ? gamemove : readmove;
}

void
ResetNewPos()
{
  ResetMoves();
  CalcPieceTab();
  UseLib = FALSE;
  
//  ClearHint();
}

void
ResetOpening()
{
  const char *libfilename = "opening.mvs";
  ifstream fin(libfilename, ios::in | ios::binary);

  if (!fin) {
    MessageBox(0,"ｵｰﾌﾟﾆﾝｸﾞﾌｧｲﾙ（OPENING.LIB）が見つかりません", "ｴﾗｰ", MB_ICONHAND | MB_OK);
    Openings = new unsigned char;
    *Openings = 0;
    return;
  }
  Openings = new unsigned char[32000];
  fin.read(Openings, 32000);
  fin.close();
  *Openings = 0xFF;
}

static void
StartUp()
{
  randomize();

  Level = (LEVELTYPE)GetPrivateProfileInt("WCHESS", "Level", (int)easygame, INIFile);

  if (GetPrivateProfileString("WCHESS", "AverageTime", "5.0", buf, 80, INIFile))
    sscanf(buf, "%lf", &AverageTime);

  if (GetPrivateProfileString("WCHESS", "WhiteSquare", "", buf, 80, INIFile))
    sscanf(buf, "%ld", &WhiteSquareColor);

  if (GetPrivateProfileString("WCHESS", "BlackSquare", "", buf, 80, INIFile))
    sscanf(buf, "%ld", &BlackSquareColor);

  MaxLevel = (BYTE)GetPrivateProfileInt("WCHESS", "MaxLevel", MAXPLY, INIFile);
  SoundOn = (BOOL)GetPrivateProfileInt("WCHESS", "SoundOn", 1, INIFile);

 
  CalcAttackTab();
  MultiMove = FALSE;
  AutoPlay = FALSE;
  Turned = FALSE;
  ResetOpening();

  OutputFile = new ofstream("Chess.log");
  if (!OutputFile) {
    MessageBox(0, "記録ﾌｧｲﾙが開けません.\n記録は行いません", "OWL Chess", MB_OK | MB_ICONEXCLAMATION);
    Logging = FALSE;

  } else {
    *OutputFile << endl;
    *OutputFile << "      OWL CHESS by Borland International\n";
    *OutputFile << "      ==================================\n" << endl;
  }
}

const UNPLAYMARK = 0x3f;

//
//  ブロック内の直前の移動を LibNo に設定
//
void
PreviousLibNo()
{
  int n = 0;
  do {
   LibNo--;
   if (Openings[LibNo] >= 128)
     n++;
   if (Openings[LibNo] & 64)
     n--;
  } while (n);
}

//
//  ブロック内の最初の移動を LibNo に設定
//
void
FirstLibNo()
{
  while (!(Openings[LibNo - 1] & 64))
    PreviousLibNo();
}

//
//  ブロック内の次の移動を LibNo に設定。
//  skip がセットされていれば、動けない移動はスキップ
//
void
NextLibNo(short skip)
{
  if (Openings[LibNo] >= 128)
    FirstLibNo();
  else {
    int n = 0;
    do {
      if (Openings[LibNo] & 64)
        n++;
      if (Openings[LibNo] >= 128)
        n--;
      LibNo++;
    } while (n);
    if (skip && (Openings[LibNo] == UNPLAYMARK))
      FirstLibNo();
  }
}

//
//  正しいブロックに一致するノードを見つける
//
static void
FindNode()
{
  LibNo++;
  if (Depth > LibDepth) {
    Found = TRUE;
    return;
  }
  OpCount = -1;
  InitMovGen();
  do {
    OpCount++;
    MovGen();
  } while (Next.movpiece != empty && !EqMove(&Next, &MovTab[Depth]));
  if (Next.movpiece != empty) {
    while ((Openings[LibNo] & 63) != OpCount && Openings[LibNo] < 128)
      NextLibNo(0);
    if ((Openings[LibNo] & 127) == 64 + OpCount) {
      MakeMove(&MovTab[Depth]);
      FindNode();
      TakeBackMove(&MovTab[Depth-1]);
    }
  }
}

//
//  位置に一致するブロックを LibNo に設定
//
void
CalcLibNo()
{
  LibNo = 0;
  if (MoveNo < UseLib) {
    LibDepth = Depth;
    while (MovTab[Depth].movpiece != empty)
      TakeBackMove(&MovTab[Depth]);
    Found = FALSE;
    if (MovTab[Depth].content == king) {
      Depth++;
      FindNode();
      Depth--;
    }
    while(Depth < LibDepth)
      MakeMove(&MovTab[Depth + 1]);
    if (Found)
      UseLib = 200;
    else {
      UseLib = MoveNo;
      LibNo = 0;
    }
  }
}

//
//  ライブラリから定石の移動を見つける
//
static void
FindOpeningMove()
{
  const unsigned char weight[7] = {7, 10, 12, 13, 14, 15, 16};

  unsigned char r = (unsigned char)random(16);   //  0..16 の乱数から重みを計算
  unsigned char p = 0;
  while (r >= weight[p])
    p++;
  for (unsigned char countp = 1; countp <= p; countp++)  // 一致するノードを探す
    NextLibNo(1);
  OpCount = Openings[LibNo] & 63;  //  移動を生成
  InitMovGen();
  for (unsigned char cnt = 0; cnt <= OpCount; cnt++)
    MovGen();

  // MainLine に移動を格納
  MainLine[0] = Next;
  MainLine[1] = ZeroMove;
  MainEvalu = 0;
  MaxDepth = 0;
  LegalMoves = 0;
  InitNode(&Nodes);
}

void
OutputNode(NODEVAL* nodes)
{
  double nodereal;
  if (!Logging)
    return;
  char buf[20];
  nodereal = nodes->nodebase * MAXINT + nodes->nodeoffset;
  sprintf(buf, "%12.1f", nodereal);
  *OutputFile << buf;
}


void StartMove();

void
ReturnAnalysis()
{
  int myx;
  char str[8];
  char buf[40];

  MovTab[0] = MainLine[0];   //  MainLine を HintLine にコピー

  DoPrintf("new1=%02x old=%02x piece=%d", MainLine[0].new1,
  MainLine[0].old,(int)MainLine[0].movpiece);
  for (myx = 1; myx <= MAXPLY; myx++)
    HintLine[myx - 1] = MainLine[myx];
  dep = MAXPLY;
  HintEvalu = MainEvalu;
  if (MovTab[0].movpiece == empty) {
    HintLine[0] = ZeroMove;   // 可能な移動はない
    if (AutoPlay) {
      NewGame();
      PrintBoard();
      StartMove();
    }
    return;
  }

//   FlashMove(&MovTab[Depth+1]);  //  明滅して移動する
  DoSlideMove(MovTab[Depth+1]);
  EnterMove(&MovTab[Depth+1]);
  if (SoundOn)
    MessageBeep(0);
  StoreMoves();
  if (Logging && !AutoPlay) {
    sprintf(buf, "%3d. ",(MoveNo+1) / 2);
    *OutputFile << buf;
    strcpy(str, MoveStr(&MovTab[0]));
    if ((PlayerMove.movpiece == empty) && (Opponent == white))
      sprintf(buf, "%8.8s ", str);
    else
      sprintf(buf, "%s%8.8s",MoveStr(&PlayerMove), str);
    *OutputFile << buf;
    sprintf(buf, "  (%s)%9.2f%3d:%2d", MoveStr(&MainLine[1]),
            MainEvalu / 256.0, MaxDepth, LegalMoves);
    *OutputFile << buf;
    OutputNode(&Nodes);
    sprintf(buf, "%8.1lf\n", ChessClock.totaltime);
    *OutputFile << buf;
  }
  PlayerMove = ZeroMove;
  ColorToPlay(Player);
  if (AutoPlay) {
    if (MoveNo >= 120 || FiftyMoveCnt() >= 100 || Repetition(0) >= 3 || MainEvalu <= -0x880) {
      NewGame();
      PrintBoard();
    }
    StartMove();
    return;
  }

}



void
StartMove()
{
  MSG msg;
// StartAnalysis();
  AdjustMoves();
  CalcLibNo();      //  オープニング移動か検査
  Depth = 0;
  if (LibNo > 0) {
    OpeningLibMsg();
    InLibrary = TRUE;
    FindOpeningMove();
    if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
     if (msg.message == WM_COMMAND && msg.wParam == CM_STOP)
        return;
     TranslateMessage(&msg);
     DispatchMessage(&msg);
    }
  } else {
    if (InLibrary) {
      InLibrary = FALSE;
      ClearMessage();
    }
    FindMove(MaxLevel);
  }
  Depth = -1;
  ReturnAnalysis();
}

void
ProgramMove()
{
  do {
    GotValidMove = FALSE;
    ColorToPlay(Player);
    StartMove();
  } while (GotValidMove);
}

void
Talk()
{
  StartUp();
  NewGame();
}

BOOL
Redo()
{
//   MakeMove(&MovTab[Depth+1]);
  EnterMove(&MovTab[Depth+1]);
 // ClearHint();
//   UpdateBoard();
  ClearBestLine();
  ColorToPlay(Player);
  if (Depth >= -1)
    return FALSE;  // 元に戻せない
  return TRUE;     // 元に戻せる
}

BOOL
Undo()
{
// TakeBackMove(&MovTab[Depth]);
  extern void RemoveMove(MOVETYPE *);
  RemoveMove(&MovTab[Depth]);
//  ClearHint();
//   UpdateBoard();
  ClearBestLine();
  ColorToPlay(Player);
  if (MovTab[Depth].movpiece == empty)
    return FALSE;  // もう取り消すものがない
  return TRUE;     // まだ取り消すことができる
}

void
QuitProgram()
{
  if (Logging)
    OutputFile->close();

  sprintf(buf, "%ld", (DWORD)WhiteSquareColor);
  WritePrivateProfileString("WCHESS", "WhiteSquare", buf, INIFile);

  sprintf(buf, "%ld", (DWORD)BlackSquareColor);
  WritePrivateProfileString("WCHESS", "BlackSquare", buf, INIFile);

  sprintf(buf, "%d", Level);
  WritePrivateProfileString("WCHESS", "Level", buf, INIFile);
  sprintf(buf, "%lf", AverageTime);
  WritePrivateProfileString("WCHESS", "AverageTime", buf, INIFile);
  sprintf(buf, "%d", (int)MaxLevel);
  WritePrivateProfileString("WCHESS", "MaxLevel", buf, INIFile);
  sprintf(buf, "%d", (int)SoundOn);
  WritePrivateProfileString("WCHESS", "SoundOn", buf, INIFile);
}

void
FindHintMove()
{
  //  HintLine が空なら、オープニングライブラリから
  //  移動を取得するか、手を考える
  //
  if (HintLine[0].movpiece == empty) {
    AdjustMoves();
    CalcLibNo();
    Depth = 0;
    if (LibNo > 0)
      FindOpeningMove();
    else {
      Analysis = 1;
      Opan = 0;
      FindMove(1);
    }
    Depth = -1;
    HintLine[0] = MainLine[0];
    HintLine[1] = ZeroMove;
    HintEvalu = -MainEvalu;
  }
}
void
InitNode(NODEVAL* nodes)
{
  nodes->nodebase = 0;
  nodes->nodeoffset = 0;
}
//
//  ProgramColor = Player として移動して変数を更新
//
void
MakeMove(MOVETYPE* move)
{
  Depth++;
  MoveNo++;
  Perform(move, 0);
  ProgramColor = Opponent;
  Opponent = Player;
  Player = ProgramColor;
}
void
IncNode(NODEVAL *nodes)
{
  if (nodes->nodeoffset >= MAXINT) {
    nodes->nodebase++;
    nodes->nodeoffset = 0;

  } else
    nodes->nodeoffset++;
}
//
//  MovTab を Depth = -1 まで移動
//
void
AdjustMoves()
{
  for (int i = Depth; i >= BACK; i--)
    MovTab[i-(Depth+1)] = MovTab[i];
  Depth = -1;
}

//
//  MovTab の移動を１つ戻す
//
void
StoreMoves()
{
  Depth--;
  for (int i = BACK; i <= Depth; i++)
    MovTab[i] = MovTab[i+1];
  MovTab[BACK] = ZeroMove;
}
void
EnterMove(MOVETYPE* move)
{

 //  PrintMove(MoveNo, ProgramColor, move, ChessTime[RunColor].totaltime);
   MakeMove(move);
   UpdateBoard();

}

//
//  移動を元に戻し、変数を更新する
//
void
TakeBackMove(MOVETYPE* move)
{
  ProgramColor = Opponent;
  Opponent = Player;
  Player = ProgramColor;
  Perform(move, 1);
  MoveNo--;
  Depth--;
}

//
//  ユーザから入力された移動を実行
//
void
EnterKeyMove()
{
  MovTab[Depth+1] = KeyMove;
  PlayerMove = KeyMove;
 
  EnterMove(&MovTab[Depth+1]);
}
void
DoSlideMove(MOVETYPE& move)
{
  SQUARETYPE castsquare, cornersquare;
 // SlidePiece(move.new1, move.old);
  if (move.spe) {
    if (move.movpiece == king) {
      GenCastSquare(move.new1, &castsquare, &cornersquare);
    //  SlidePiece(castsquare, cornersquare);
    }
  }
}
//
//  MovTab をリセット
//
void
ResetMoves()
{
  Depth = -1;
  MovTab[-1] = ZeroMove;
}





BOOL
MoveCheck(SQUARETYPE startsq, SQUARETYPE endsq)
{
  Depth++;
  KeyMove = ZeroMove;
  InitMovGen();
  do {
    MovGen();
    if (Next.new1 == endsq && Next.old == startsq) {
      KeyMove = Next;
      break;
    }
  } while (Next.movpiece != empty);

  if (KeyMove.movpiece == empty) {

    Depth--;
    return FALSE;
  }
  if (IllegalMove(&KeyMove)) {

    Depth--;
    return FALSE;
  }
  Depth--;

    AdjustMoves();
    EnterKeyMove();
    StoreMoves();

  return TRUE;
}
//
//  渡された位置を ProgramColor == Player として移動が正しいか検査
//
BOOL
IllegalMove(MOVETYPE* move)
{
  Perform(move, 0);
  BOOL illegal = Attacks(Opponent, PieceTab[Player][0].isquare);
  Perform(move, 1);
  return illegal;
}
