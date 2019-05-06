//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include "wcdefs.h"
#include "wchess.h"
#include "info.h"
#include "externs.h"                                    
//BOOL Turned;

//
//  �O���[�o���ϐ�
//
BOARDIDTYPE Display[0x78];
char *PieceLetter = " KQRBNP";
char buf[280];   //  �ėp������o�b�t�@�A�e���Ŏg�p

//
//  �ÓI�O���[�o���ϐ�
//
static TRect BoardRect;

//
//  �w�肵����̃r�b�g�}�b�v�n���h��
//
HBITMAP
GetBitmapHandle(PIECETYPE piece, COLORTYPE pcolor)
{
  if (!piece)
    return 0;
  return PieceBmpArray[piece - 1][pcolor];
}


//
//  ���\���E�B���h�E�̂��ׂĂ̏�������
//
void
ClearInfoWindow()
{

}

//
//  �v���C���鑤�̐F��\��
//
void
ColorToPlay(COLORTYPE color)
{

}

void
Message(char* str)
{

}

void
Error(char* str)
{
  if (SoundOn)
    MessageBeep(0);
  strcpy(buf, str);
  SendMessage(hWndMain, WM_COMMAND, EM_ERROR, 0L);
}

void
Warning(char* str)
{
  if (SoundOn)
    MessageBeep(0);
  Message(str);
}

//
//  ��̈ړ��𕶎���ɕϊ�
//
char*
MoveStr(MOVETYPE* move)
{
  static char str[7];

  strcpy(str, "   ");
  if (move->movpiece != empty) {
    if (move->spe && move->movpiece == king) { //  �L���X�����O
      if (move->new1 > move->old)
        strcpy(str, "O-O  ");
      else
        strcpy(str, "O-O-O ");

    } else {
      str[0] = PieceLetter[move->movpiece];
      str[1] = char('a' + move->old % 16);
      str[2] = char('1' + move->old / 16);
      str[3] = move->content == empty ? '-' : 'x';
      str[4] = char('a' + move->new1 % 16);
      str[5] = char('1' + move->new1 / 16);
    }
  }
  return str;
}


void
PrintMove(int moveno, COLORTYPE programcolor, MOVETYPE* move, double time)
{
  int minutes = (int)(time / 60.0);

  sprintf(buf, "%2.2d:%#04.1f %3.3d. %s", minutes, time - minutes * 60.0, moveno / 2 + 1, MoveStr(move));
/*
  if (programcolor == white)
    TInfo->SetWhiteInfoText(buf);
  else
    TInfo->SetBlackInfoText(buf);
    */
}

//
//  �R�����g��`��
//
void
DrawFrame(HDC hDC, TRect& rect, BOOL drawBackground)
{
  int x1 = rect.left;
  int x2 = rect.right;
  int y1 = rect.top;
  int y2 = rect.bottom;

  //
  // �g�̗֊s��`���A�v��������Δw�i����������
  //
  HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,
      GetStockObject(drawBackground ? LTGRAY_BRUSH : NULL_BRUSH));
  HPEN hOldPen = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
  Rectangle(hDC, x1, y1, x2, y2);

  //
  // �g�̍��������ł��郉�C�g�O���[�̒����`��`��
  //
  HPEN hPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
  SelectObject(hDC, hPen);
  Rectangle(hDC, x1+1, y1+1, x2-1, y2-1);
  DeleteObject(SelectObject(hDC, GetStockObject(WHITE_PEN)));

  //
  // �g�̉E�Ɖ����̖��邢������`��
  //
  TPoint points[3];
  points[0].x = x1 + 2;
  points[1].y = points[0].y = y2 - 3;
  points[2].x = points[1].x = x2 - 3;
  points[2].y = y1 + 2;
  Polyline(hDC, points, 3);

  //
  // �g�̉E�Ɖ����̉e��`��
  //
  hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
  SelectObject(hDC, hPen);
  points[0].x = x1;
  points[1].y = points[0].y = y2-1;
  points[2].x = points[1].x = x2-1;
  points[2].y = y1;
  Polyline(hDC, points, 3);
  SetPixel(hDC, x2-1, y2-1, RGB(128, 128, 128));

  //
  // �g�̍��Ə㕔�̉e��`��
  //
  points[1].x = points[0].x = x1 + 2;
  points[0].y = y2 - 3;
  points[2].y = points[1].y = y1 + 2;
  points[2].x = x2 - 3;
  Polyline(hDC, points, 3);
  SelectObject(hDC, hOldBrush);
  DeleteObject(SelectObject(hDC, hOldPen));

  //
  // �e�Ɩ��邢�����������ΐ��̊p��Y��Ă͂����Ȃ�
  //
  SetPixel(hDC, x2-1, y1,   RGB(192, 192, 192));
  SetPixel(hDC, x2-3, y1+2, RGB(192, 192, 192));
  SetPixel(hDC, x1,   y2-1, RGB(192, 192, 192));
  SetPixel(hDC, x1+2, y2-3, RGB(192, 192, 192));
}

//
//  ���݂̃��x����\��
//
void
PrintCurLevel()
{
  extern BOOL MultiMove;

  if (MultiMove)
    strcpy(buf, "�Q�l�p");
  else {
    switch (Level) {
      case normal:
        sprintf(buf, "%1.0f �b / ��", AverageTime);
        break;
      case fullgametime:
        sprintf(buf, "%2.2f �� / ����", AverageTime);
        break;
      case easygame:
        strcpy(buf, "�₳����");
        break;
      case infinite :
        strcpy(buf, "������ǂ�");
        break;
      case plysearch :
        sprintf(buf, "��ǂݐ[�� = %d", MaxLevel);
        break;
      case matesearch:
        strcpy(buf, "�l�߂�ǂ�");
        break;
      case matching :
        strcpy(buf, "�Γ��Ȏ���");
        break;
    }
  }
 // TInfo->SetLevelText(buf);
}

TPoint
GetSquareXY(SQUARETYPE square)
{
  if (Turned)
    square ^= 0x77;
  return TPoint((square % 8) * SQUARE_SIZE + BORDERSIZE + MYFRAMESIZE,
                (7 - square / 16) * SQUARE_SIZE + BORDERSIZE + MYFRAMESIZE);
}

void
ClearSquare(SQUARETYPE square)
{

}

void
ClearDisplay()
{
  ClearInfoWindow();
  for (SQUARETYPE sq = 0; sq <= 0x77; sq++)
    Display[sq].piece = empty;
}

//
//  ��ʂɃ{�[�h��\��
//
void
DrawBoard()
{
  unsigned char no;
  HDC hDC;
  const SQUARETYPE printno[64] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x67,
    0x57, 0x47, 0x37, 0x27, 0x17, 0x07, 0x06, 0x05,
    0x04, 0x03, 0x02, 0x01, 0x11, 0x21, 0x31, 0x41,
    0x51, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x56,
    0x46, 0x36, 0x26, 0x16, 0x15, 0x14, 0x13, 0x12,
    0x22, 0x32, 0x42, 0x52, 0x53, 0x54, 0x55, 0x45,
    0x35, 0x25, 0x24, 0x23, 0x33, 0x43, 0x44, 0x34
  };

  BoardRect.left = BoardRect.top = BORDERSIZE;
  BoardRect.right = BoardRect.bottom = BORDERSIZE + 2*MYFRAMESIZE + 8*SQUARE_SIZE;


  for (no = 0; no < 64; no++)
    ClearSquare(printno[no]);
}


void
PrintPiece(SQUARETYPE square, PIECETYPE piece, COLORTYPE color, DWORD Rop)
{

   return;

 }

void
InitDisplay()
{
  for (SQUARETYPE square = 0; square <= 0x77; square++)
    if (!(square & 0x88))
      if (Board[square].piece != Display[square].piece ||
          Board[square].piece != empty &&
          Board[square].color != Display[square].color) {
        Display[square].piece = Board[square].piece;
        Display[square].color = Board[square].color;
      }
}

//
//  �����؂ɂ����̘g��\���B
//  Defended �� TRUE (��삳��Ă���)�Ȃ獕�A�����łȂ���ΐԁB
//
static void
FrameSquare(SQUARETYPE square, BOOL Defended)
{

}

void
HideAttacks()
{
  for (SQUARETYPE square = 0; square <= 0x77; square++)
    if (!(square & 0x88)) {
      if (Board[square].attacked) {
        Board[square].attacked = FALSE;
        ClearSquare(square);
        PrintPiece(square, Board[square].piece, Board[square].color, SRCINVERT);
      }
    }
}

void
ShowAttacks()
{
  for (SQUARETYPE square = 0; square <= 0x77; square++)
    if (!(square & 0x88)) {
      if (Attacks(ComputerColor, square) && Board[square].color != ComputerColor && Board[square].piece != empty) {
        Board[square].attacked = TRUE;
        if (Attacks((COLORTYPE)!ComputerColor, square))
          FrameSquare(square, TRUE);
        else
          FrameSquare(square, FALSE);

      } else if (Board[square].attacked) {
        Board[square].attacked = FALSE;
        ClearSquare(square);
        PrintPiece(square, Board[square].piece, Board[square].color, SRCINVERT);
      }
    }
}

void
UpdateBoard()
{
  for (SQUARETYPE square = 0; square <= 0x77; square++)
    if (!(square & 0x88))
      if (Board[square].piece != Display[square].piece ||
          Board[square].piece != empty &&
          Board[square].color != Display[square].color) {
        if (Display[square].piece != empty)
          ClearSquare(square);
        Display[square].piece = Board[square].piece;
        Display[square].color = Board[square].color;
        if (Board[square].piece != empty)
          PrintPiece(square, Board[square].piece,Board[square].color, SRCINVERT);
      }
  if (Level == easygame && !Editing)
    ShowAttacks();
}

static void DrawAlphaNum();


//
//  �n���ꂽ���W�̈ʒu�ɂ���}�X�������A������ player �̐F��
//  ��ǂ������ׂ�B
//
SQUARETYPE
GetValidSquare(TPoint p, COLORTYPE player, BOOL CheckPiece)
{
/*
int s= SQUARE_SIZE;
  for (SQUARETYPE square = 0; square <= 0x77; square++) {
    if (!(square & 0x88)) {
      TPoint point = GetSquareXY(square);
      TRect  sqrect(point, TSize(s,s));
      if (sqrect.Contains(p)) {
        if ((Display[square].color == player && Display[square].piece != empty) 
|| !CheckPiece)
          return square;
      }
    }
  }
  */
  return -1;
}

void
DrawNormalBitmap(SQUARETYPE square)
{
  ClearSquare(square);
  PrintPiece(square, Display[square].piece, Display[square].color, SRCINVERT);
}

void
DrawInvertedBitmap(SQUARETYPE square)
{
  PrintPiece(square, Display[square].piece, Display[square].color, NOTSRCERASE);
}

void
OpeningLibMsg()
{
//  TInfo->SetMessageText("�����ݸގg�p");
}

void
PrintNodes(NODEVAL* nodes, double time)
{
  double nodereal = nodes->nodebase * MAXINT + nodes->nodeoffset;
  char buf[80];
  if (time) {
    sprintf(buf, "%7.1f", nodereal/time);
 //   TInfo->SetSecondsText(buf);
  }
  sprintf(buf, "%7.0f ", nodereal);
//  TInfo->SetNodeText(buf);
}

//
//  ��ʂɍőP���\��
//
void
PrintBestMove(MOVETYPE *mainline, MAXTYPE mainevalu)
{
  if (ShowBestLine == FALSE)
    return;

  *buf = 0;
  DEPTHTYPE dep = 0;
  while (dep < 7 && mainline[dep].movpiece != empty) {
   strcat(buf, MoveStr(&mainline[dep++]));
   strcat(buf, " ");
  }
//  TInfo->SetBestLineText(buf);
  sprintf(buf, "%7.2f", mainevalu/256.0);
 // TInfo->SetValueText(buf);
}

void
ClearBestLine()
{
//  TInfo->SetBestLineText("");
}

void
ClearMessage()
{
 // TInfo->SetMessageText("");
}

static char * CharArray[] = { "a","b", "c", "d", "e", "f", "g", "h" };
static char * NumArray[] = { "1", "2", "3", "4", "5", "6", "7", "8" };

static void
DrawBump(HDC hDC, int x, int y)
{
  int x2 = x + CHARSIZE + 2;
  int y2 = y-- + LINESIZE + 1;
  x -= 2;

  HANDLE hOldBrush = SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
  HANDLE hOldPen = SelectObject(hDC, GetStockObject(WHITE_PEN));

  Rectangle(hDC, x, y, x2, y2);

  SelectObject(hDC, CreatePen(PS_SOLID, 1, RGB(128, 128, 128)));
  TPoint points[3];
  points[0].x = ++x;
  points[1].y = points[0].y = y2;
  points[2].x = points[1].x = x2;
  points[2].y = ++y;
  Polyline(hDC, points, 3);

  SelectObject(hDC, hOldBrush);
  DeleteObject(SelectObject(hDC, hOldPen));
}





