//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "wcdefs.h"
#include "wchess.h"
#include "externs.h"

//
//  グローバル変数
//
MOVETYPE  ZeroMove = { 8, 8, 0, empty, empty };
CLOCKTYPE ChessTime[2];
MOVETYPE  KeyMove;
BOOL      Running;
COLORTYPE RunColor;
BOOL      Analysis, Opan;
double    WantedTime;
BOOL      GameOver = FALSE;
char      EndGameMessage[80];

extern double AverageTime;


void EndMessage(char *);

//
//  チェス時計を初期化
//
void
InitChessTime()
{
  InitTime(&ChessTime[white]);
  InitTime(&ChessTime[black]);
}

void
StopChessTime()
{
  if (Running) {
    StopTime(&ChessTime[RunColor]);
    KillTimer(hWndMain, TIMEID);
    Running = FALSE;
  }
}

//
//  計測している時計を止めて、color 側の時計を計測開始
//
void
StartChessTime(COLORTYPE color)
{
  RunColor = color;
  Running = TRUE;
  StartTime(&ChessTime[RunColor]);
  SetTimer(hWndMain, TIMEID, 1000, 0);
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

//
//  HintLine をクリア
//
void
ClearHint()
{
  HintLine[0] = ZeroMove;
  HintEvalu = 0;
}

void
InitNode(NODEVAL* nodes)
{
  nodes->nodebase = 0;
  nodes->nodeoffset = 0;
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

//
//  ゲームのコメントを表示（チェック、メイト、引き分け、放棄）
//
void
PrintComment()
{
  short check, possiblemove, checkmate;
  int nummoves;

  Message("");
  checkmate = 0;
  Depth++;
  possiblemove = 0;
  InitMovGen();
  do {
    MovGen();
    if (Next.movpiece != empty)
      if (!IllegalMove(&Next))
        possiblemove = 1;
  } while (Next.movpiece != empty && !possiblemove);

  Depth--;
  check = Attacks(Opponent, PieceTab[Player][0].isquare); // チェックを計算
  //  移動はチェックメイトでもスティルメイトでもない
  if (!possiblemove) {
    if (check) {
      checkmate = 1;
      EndMessage("ﾁｪｯｸﾒｲﾄ");
    } else
      EndMessage("ｽﾃｨﾙﾒｲﾄ!");

  } else
    if (HintEvalu >= MATEVALUE - DEPTHFACTOR * 16) {
      nummoves = (MATEVALUE - HintEvalu + 0x40) / (DEPTHFACTOR * 2);
      sprintf(buf, "%d 手!", nummoves);
      Message(buf);
    }
  if (check && !checkmate)
    Message("ﾁｪｯｸ!");
  else { //  50 手ルールと繰り返し移動の検査
    if (FiftyMoveCnt() >= 100) {
      EndMessage("50 手ﾙｰﾙ");
    } else
     if (Repetition(0) >= 3) {
       EndMessage("3 回の繰り返し");
     } else        //  見込のない試合は放棄
      if (-25500 < HintEvalu && HintEvalu < -0x880) {
        switch (Opponent) {
          case white:
            EndMessage(" 白の放棄");
            break;
          case black:
            EndMessage(" 黒の放棄");
        }
      }
  }
}

void
EnterMove(MOVETYPE* move)
{
   StopChessTime();
   PrintMove(MoveNo, ProgramColor, move, ChessTime[RunColor].totaltime);
   MakeMove(move);
   UpdateBoard();
   PrintComment();
   StartChessTime(ProgramColor);
}

void
RemoveMove(MOVETYPE* move)
{
   StopChessTime();
   PrintMove(MoveNo, ProgramColor, move, ChessTime[RunColor].totaltime);
   TakeBackMove(move);
   UpdateBoard();
   PrintComment();
   StartChessTime(ProgramColor);
}

//
//  ユーザから入力された移動を実行
//
void
EnterKeyMove()
{
  MovTab[Depth+1] = KeyMove;
  PlayerMove = KeyMove;
  ClearHint();
  DragEnd(TRUE);
  EnterMove(&MovTab[Depth+1]);
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
  
//
//  入力された移動は正しいか検査
//
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
    Warning("不正な移動です");
    Depth--;
    return FALSE;
  }
  if (IllegalMove(&KeyMove)) {
    Warning("不正な移動です. ﾁｪｯｸ!");
    Depth--;
    return FALSE;
  }
  Depth--;
  if (!ComputerThinking) {
    AdjustMoves();
    EnterKeyMove();
    StoreMoves();
  }
  return TRUE;
}

//
//  要求された応答時間を計算
//
void
StartAnalysis()
{
  int            timecontrol;
  extern HWND    hWndMain;
  extern HCURSOR hWaitCursor;

  Analysis = 1;
  Opan = 0;
  SetClassWindowCursor(hWndMain, hWaitCursor);

  switch (Level) {
    case easygame:
    case normal:
      //  残り時間を残りの移動の間の時間制御で配分。
      //  ４手分のマージンを付けて、プログラムが確実に
      //  タイムロスしないようにする
      timecontrol = (((MoveNo >> 1) + 20) / 20) * 20;
      if (timecontrol <= 40)
        timecontrol = 40;
      WantedTime = (AverageTime * timecontrol - ChessTime[ProgramColor].totaltime) /
            (timecontrol + 4 - (MoveNo >> 1));

      //  ゲームの始めは２倍の時間をかけて考え、通常、
      //  中盤の始めからがゲームの最も決定的な部分
      if ((MoveNo >> 1) <= 40)
        WantedTime = 5.0 + (WantedTime - 5.0) * ((80 - (MoveNo >> 1)) / 40);
      break;

    case fullgametime:
      //  ゲームが最後 40 手づつと考え、残り時間を配分
      WantedTime = (AverageTime * 60.0 - ChessTime[ProgramColor].totaltime) / 44;

      //  ゲームの始めは２倍の時間をかけて考え、通常、
      //  中盤の始めからがゲームの最も決定的な部分
      if ((MoveNo >> 1) <= 40)
        WantedTime = 5.0 + (WantedTime - 5.0) * ((80 - (MoveNo >> 1)) /40);
      break;

    case matching:
      //  相手と同じぐらいの時間をかける
      if (MoveNo >= 2)
        WantedTime = ChessTime[Opponent].totaltime / (MoveNo >> 1);
      else
        WantedTime = 5.0;
      WantedTime += (ChessTime[Opponent].totaltime - 
                     ChessTime[ProgramColor].totaltime) * 0.25;

      //break; //wasn't here???

    default:
      WantedTime = 1000000.0;
  }
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

void
IncNode(NODEVAL *nodes)
{
  if (nodes->nodeoffset >= MAXINT) {
    nodes->nodebase++;
    nodes->nodeoffset = 0;

  } else
    nodes->nodeoffset++;
}


void
Wait(int tenths)
{
  clock_t NumTicksToWait;

  // 1000ティックが１秒なので, 100ティック == 1/10秒
  NumTicksToWait = tenths*100 + clock();
  while (NumTicksToWait > clock())
    ;
}

//
//  画面上を明滅して移動する
//
void
FlashMove(MOVETYPE* move)
{
  MakeMove(move);
  UpdateBoard();
  Wait(4);
  TakeBackMove(move);
  UpdateBoard();
  Wait(4);
}

void
DoSlideMove(MOVETYPE& move)
{
  SQUARETYPE castsquare, cornersquare;
  SlidePiece(move.new1, move.old);
  if (move.spe) {
    if (move.movpiece == king) {
      GenCastSquare(move.new1, &castsquare, &cornersquare);
      SlidePiece(castsquare, cornersquare);
    }
  }
}

void
EndMessage(char* message)
{
  strcpy(EndGameMessage, message);
  GameOver = TRUE;
}

void
ShowHint()
{
  DEPTHTYPE dep = 0;
  *buf = 0;
  Message(buf);
  while (HintLine[dep].movpiece != empty) {
    strcat(buf, MoveStr(&HintLine[dep]));
    strcat(buf, " ");
    Message(buf);
    MakeMove(&HintLine[dep]);
    UpdateBoard();
    Wait(6);
    dep++;
  }
  while (dep > 0) {
    dep--;
    TakeBackMove(&HintLine[dep]);
  }
  UpdateBoard();
}
