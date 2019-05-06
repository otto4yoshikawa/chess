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
//  グローバル変数
//
BOARDIDTYPE Display[0x78];
char *PieceLetter = " KQRBNP";
char buf[280];   //  汎用文字列バッファ、各所で使用

//
//  静的グローバル変数
//
static TRect BoardRect;

//
//  指定した駒のビットマップハンドル
//
HBITMAP
GetBitmapHandle(PIECETYPE piece, COLORTYPE pcolor)
{
  if (!piece)
    return 0;
  return PieceBmpArray[piece - 1][pcolor];
}


//
//  情報表示ウィンドウのすべての情報を消去
//
void
ClearInfoWindow()
{

}

//
//  プレイする側の色を表示
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
//  駒の移動を文字列に変換
//
char*
MoveStr(MOVETYPE* move)
{
  static char str[7];

  strcpy(str, "   ");
  if (move->movpiece != empty) {
    if (move->spe && move->movpiece == king) { //  キャスリング
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
//  ３次元枠を描画
//
void
DrawFrame(HDC hDC, TRect& rect, BOOL drawBackground)
{
  int x1 = rect.left;
  int x2 = rect.right;
  int y1 = rect.top;
  int y2 = rect.bottom;

  //
  // 枠の輪郭を描き、要求があれば背景を消去する
  //
  HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC,
      GetStockObject(drawBackground ? LTGRAY_BRUSH : NULL_BRUSH));
  HPEN hOldPen = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
  Rectangle(hDC, x1, y1, x2, y2);

  //
  // 枠の高い部分であるライトグレーの長方形を描く
  //
  HPEN hPen = CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
  SelectObject(hDC, hPen);
  Rectangle(hDC, x1+1, y1+1, x2-1, y2-1);
  DeleteObject(SelectObject(hDC, GetStockObject(WHITE_PEN)));

  //
  // 枠の右と下部の明るい部分を描く
  //
  TPoint points[3];
  points[0].x = x1 + 2;
  points[1].y = points[0].y = y2 - 3;
  points[2].x = points[1].x = x2 - 3;
  points[2].y = y1 + 2;
  Polyline(hDC, points, 3);

  //
  // 枠の右と下部の影を描く
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
  // 枠の左と上部の影を描く
  //
  points[1].x = points[0].x = x1 + 2;
  points[0].y = y2 - 3;
  points[2].y = points[1].y = y1 + 2;
  points[2].x = x2 - 3;
  Polyline(hDC, points, 3);
  SelectObject(hDC, hOldBrush);
  DeleteObject(SelectObject(hDC, hOldPen));

  //
  // 影と明るい部分が交わる斜線の角を忘れてはいけない
  //
  SetPixel(hDC, x2-1, y1,   RGB(192, 192, 192));
  SetPixel(hDC, x2-3, y1+2, RGB(192, 192, 192));
  SetPixel(hDC, x1,   y2-1, RGB(192, 192, 192));
  SetPixel(hDC, x1+2, y2-3, RGB(192, 192, 192));
}

//
//  現在のレベルを表示
//
void
PrintCurLevel()
{
  extern BOOL MultiMove;

  if (MultiMove)
    strcpy(buf, "２人用");
  else {
    switch (Level) {
      case normal:
        sprintf(buf, "%1.0f 秒 / 手", AverageTime);
        break;
      case fullgametime:
        sprintf(buf, "%2.2f 分 / 試合", AverageTime);
        break;
      case easygame:
        strcpy(buf, "やさしい");
        break;
      case infinite :
        strcpy(buf, "無限先読み");
        break;
      case plysearch :
        sprintf(buf, "先読み深さ = %d", MaxLevel);
        break;
      case matesearch:
        strcpy(buf, "詰めを読む");
        break;
      case matching :
        strcpy(buf, "対等な時間");
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
//  画面にボードを表示
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
//  効き筋にある駒の枠を表示。
//  Defended が TRUE (守護されている)なら黒、そうでなければ赤。
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
//  渡された座標の位置にあるマスを見つけ、そこが player の色の
//  駒かどうか調べる。
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
//  TInfo->SetMessageText("ｵｰﾌﾟﾆﾝｸﾞ使用");
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
//  画面に最善手を表示
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





