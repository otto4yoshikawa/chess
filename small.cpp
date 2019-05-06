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
//  �O���[�o���ϐ�
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
//  �`�F�X���v��������
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
//  �v�����Ă��鎞�v���~�߂āAcolor ���̎��v���v���J�n
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
//  MovTab �����Z�b�g
//
void
ResetMoves()
{
  Depth = -1;
  MovTab[-1] = ZeroMove;
}

//
//  HintLine ���N���A
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
//  �n���ꂽ�ʒu�� ProgramColor == Player �Ƃ��Ĉړ���������������
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
//  ProgramColor = Player �Ƃ��Ĉړ����ĕϐ����X�V
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
//  �Q�[���̃R�����g��\���i�`�F�b�N�A���C�g�A���������A�����j
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
  check = Attacks(Opponent, PieceTab[Player][0].isquare); // �`�F�b�N���v�Z
  //  �ړ��̓`�F�b�N���C�g�ł��X�e�B�����C�g�ł��Ȃ�
  if (!possiblemove) {
    if (check) {
      checkmate = 1;
      EndMessage("����Ҳ�");
    } else
      EndMessage("�è�Ҳ�!");

  } else
    if (HintEvalu >= MATEVALUE - DEPTHFACTOR * 16) {
      nummoves = (MATEVALUE - HintEvalu + 0x40) / (DEPTHFACTOR * 2);
      sprintf(buf, "%d ��!", nummoves);
      Message(buf);
    }
  if (check && !checkmate)
    Message("����!");
  else { //  50 �胋�[���ƌJ��Ԃ��ړ��̌���
    if (FiftyMoveCnt() >= 100) {
      EndMessage("50 ��ٰ�");
    } else
     if (Repetition(0) >= 3) {
       EndMessage("3 ��̌J��Ԃ�");
     } else        //  �����̂Ȃ������͕���
      if (-25500 < HintEvalu && HintEvalu < -0x880) {
        switch (Opponent) {
          case white:
            EndMessage(" ���̕���");
            break;
          case black:
            EndMessage(" ���̕���");
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
//  ���[�U������͂��ꂽ�ړ������s
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
//  MovTab �� Depth = -1 �܂ňړ�
//
void
AdjustMoves()
{
  for (int i = Depth; i >= BACK; i--)
    MovTab[i-(Depth+1)] = MovTab[i];
  Depth = -1;
}

//
//  MovTab �̈ړ����P�߂�
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
//  ���͂��ꂽ�ړ��͐�����������
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
    Warning("�s���Ȉړ��ł�");
    Depth--;
    return FALSE;
  }
  if (IllegalMove(&KeyMove)) {
    Warning("�s���Ȉړ��ł�. ����!");
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
//  �v�����ꂽ�������Ԃ��v�Z
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
      //  �c�莞�Ԃ��c��̈ړ��̊Ԃ̎��Ԑ���Ŕz���B
      //  �S�蕪�̃}�[�W����t���āA�v���O�������m����
      //  �^�C�����X���Ȃ��悤�ɂ���
      timecontrol = (((MoveNo >> 1) + 20) / 20) * 20;
      if (timecontrol <= 40)
        timecontrol = 40;
      WantedTime = (AverageTime * timecontrol - ChessTime[ProgramColor].totaltime) /
            (timecontrol + 4 - (MoveNo >> 1));

      //  �Q�[���̎n�߂͂Q�{�̎��Ԃ������čl���A�ʏ�A
      //  ���Ղ̎n�߂��炪�Q�[���̍ł�����I�ȕ���
      if ((MoveNo >> 1) <= 40)
        WantedTime = 5.0 + (WantedTime - 5.0) * ((80 - (MoveNo >> 1)) / 40);
      break;

    case fullgametime:
      //  �Q�[�����Ō� 40 ��Âƍl���A�c�莞�Ԃ�z��
      WantedTime = (AverageTime * 60.0 - ChessTime[ProgramColor].totaltime) / 44;

      //  �Q�[���̎n�߂͂Q�{�̎��Ԃ������čl���A�ʏ�A
      //  ���Ղ̎n�߂��炪�Q�[���̍ł�����I�ȕ���
      if ((MoveNo >> 1) <= 40)
        WantedTime = 5.0 + (WantedTime - 5.0) * ((80 - (MoveNo >> 1)) /40);
      break;

    case matching:
      //  ����Ɠ������炢�̎��Ԃ�������
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
//  �ړ������ɖ߂��A�ϐ����X�V����
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

  // 1000�e�B�b�N���P�b�Ȃ̂�, 100�e�B�b�N == 1/10�b
  NumTicksToWait = tenths*100 + clock();
  while (NumTicksToWait > clock())
    ;
}

//
//  ��ʏ�𖾖ł��Ĉړ�����
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
