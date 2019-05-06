//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//----------------------------------------------------------------------------
#include <vcl.h>
#include "wcdefs.h"
#include "info.h"
#include "externs.h"

short LEFTMARGIN = 6;
short SEPERATOR = 4;
#define MAXLINE    (INFOXSIZE - LEFTMARGIN -1)

#define COLORX     (LEFTMARGIN + CHARSIZE * 6)
#define COLORY     (SEPERATOR + MYFRAMESIZE)
#define COLORW     (CHARSIZE * 6)
#define TIMERX    (LEFTMARGIN + CHARSIZE * 6)
#define TIMERY    (COLORY + LINESIZE + SEPERATOR)
#define TIMERW    (MAXLINE - TIMERX)
#define WHITEINFOX    (LEFTMARGIN + CHARSIZE * 4)
#define WHITEINFOY    (TIMERY + LINESIZE + SEPERATOR * 2)
#define WHITEINFOW    (MAXLINE - WHITEINFOX)
#define BLACKINFOX    (WHITEINFOX)
#define BLACKINFOY   (WHITEINFOY + LINESIZE + SEPERATOR)
#define BLACKINFOW   (WHITEINFOW)
#define LEVELX   (LEFTMARGIN + CHARSIZE * 6)
#define LEVELY   (BLACKINFOY + LINESIZE + SEPERATOR)
#define LEVELW   (MAXLINE - LEVELX)
#define VALUEX   (LEFTMARGIN + CHARSIZE * 8)
#define VALUEY   (LEVELY + LINESIZE + SEPERATOR * 2)
#define VALUEW   (MAXLINE - VALUEX)
#define NODEX    (LEFTMARGIN + CHARSIZE * 6)
#define NODEY    (VALUEY + LINESIZE + SEPERATOR)
#define NODEW    (MAXLINE - NODEX)
#define SECX    (LEFTMARGIN + CHARSIZE * 9)
#define SECY    (NODEY + LINESIZE + SEPERATOR)
#define SECW    (MAXLINE - SECX)
#define DEPTHY   (SECY + LINESIZE + SEPERATOR)
#define DEPTHX   (LEFTMARGIN + CHARSIZE * 6)
#define DEPTHW   (MAXLINE - DEPTHX)
#define STATBESTY (DEPTHY + LINESIZE + SEPERATOR * 2)
#define BESTX    (LEFTMARGIN)
#define BESTY    (STATBESTY + LINESIZE + 1)
#define BESTW    (MAXLINE - LEFTMARGIN)
#define BESTH    (LINESIZE * 3 + SEPERATOR * 3 - 9)
#define MESSX    (LEFTMARGIN)
#define MESSY    (BESTY + BESTH + SEPERATOR * 2)
#define MESSW    (MAXLINE - MESSX)


#define EmptyStr  ""


//--------------- TNoEraseBkGndStatic ÉÅÉ\ÉbÉh -------------------

DEFINE_RESPONSE_TABLE1(TNoEraseBkGndStatic, TStatic)
  EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

TNoEraseBkGndStatic::TNoEraseBkGndStatic(TWindow* parent, int id, const char* title, int x,
                    int y, int w, int h, WORD textLen)
  : TStatic(parent, id, title, x, y, w, h, textLen)
{
  DoErase = TRUE;
  LastLen = 0;
}

BOOL
TNoEraseBkGndStatic::EvEraseBkGnd(HDC hDC)
{
  if (DoErase) {
    TDC dc(hDC);
    dc.SelectStockObject(LTGRAY_BRUSH);
    dc.PatBlt(GetClientRect(), PATCOPY);
    DoErase = FALSE;
    return TRUE;
  }
  return FALSE;
}

void
TNoEraseBkGndStatic::SetText(const char* text)
{
  int len = strlen(text);
  if (len < LastLen)
    SetErase();
  TStatic::SetText(text);
  LastLen = len;
}

//---------------- TInfoWindow ÉÅÉ\ÉbÉh -------------------------

DEFINE_RESPONSE_TABLE1(TInfoWindow, TWindow)
  EV_WM_PAINT,
  EV_WM_CTLCOLOR,
END_RESPONSE_TABLE;

TInfoWindow::TInfoWindow(TWindow* parent, const char* title)
  : TWindow(parent, title)
{
  Attr.Style |= WS_CHILD | WS_VISIBLE;
  Attr.X = 3*BORDERSIZE + MAXBDSIZE*SQUARE_SIZE + 2*MYFRAMESIZE;
  Attr.Y = BORDERSIZE;

  LEFTMARGIN= short(MYFRAMESIZE + CHARSIZE/2);
  SEPERATOR = short(LINESIZE/4);
  INFOXSIZE = short(28*CHARSIZE + 2*LEFTMARGIN);
  INFOYSIZE = short(MESSY + LINESIZE + SEPERATOR + MYFRAMESIZE);
  short temp = MAXBDSIZE * SQUARE_SIZE + 2*MYFRAMESIZE;
  INFOYSIZE = (temp > INFOYSIZE) ? temp : INFOYSIZE;
  InfoRect.right = Attr.W = INFOXSIZE;
  InfoRect.bottom = Attr.H = INFOYSIZE;
  InfoRect.left = InfoRect.top =  0;

  new TStatic(this, -1, " íÖéË:", LEFTMARGIN, COLORY, CHARSIZE*6, LINESIZE);
  new TStatic(this, -1, " îí:", LEFTMARGIN, WHITEINFOY, CHARSIZE*4, LINESIZE);
  new TStatic(this, -1, " çï:", LEFTMARGIN, BLACKINFOY, CHARSIZE*4, LINESIZE);
  new TStatic(this, -1, " éûä‘:", LEFTMARGIN, TIMERY, CHARSIZE*6, LINESIZE);
  new TStatic(this, -1, " ⁄ÕﬁŸ:", LEFTMARGIN, LEVELY, CHARSIZE*6, LINESIZE);
  new TStatic(this, -1, " ï]âøíl:", LEFTMARGIN, VALUEY, CHARSIZE*8, LINESIZE);
  new TStatic(this, -1, " …∞ƒﬁ:", LEFTMARGIN, NODEY, CHARSIZE*6, LINESIZE);
  new TStatic(this, -1, " …∞ƒﬁ/ïb:", LEFTMARGIN, SECY, CHARSIZE*9, LINESIZE);
  new TStatic(this, -1, " ê[Ç≥:", LEFTMARGIN, DEPTHY, CHARSIZE*6, LINESIZE);
  new TStatic(this, -1, " ç≈ëPéË:", LEFTMARGIN-2, STATBESTY, MAXLINE-2, LINESIZE);

  Color = new TStatic(this, -1, EmptyStr, COLORX, COLORY, COLORW, LINESIZE);
  WhiteInfo = new TStatic(this, -1, EmptyStr, WHITEINFOX, WHITEINFOY,
    WHITEINFOW, LINESIZE, WHITEINFOW/CHARSIZE);
  BlackInfo = new TStatic(this, -1, EmptyStr, BLACKINFOX, BLACKINFOY,
    BLACKINFOW, LINESIZE, BLACKINFOW / CHARSIZE);
  Timer = new TNoEraseBkGndStatic(this, -1, EmptyStr, TIMERX, TIMERY,
    TIMERW, LINESIZE, WORD(TIMERW /CHARSIZE));
  Level = new TStatic(this, -1, EmptyStr, LEVELX, LEVELY,
    LEVELW, LINESIZE, LEVELW / CHARSIZE);
  Value = new TStatic(this, -1, EmptyStr, VALUEX, VALUEY,
    VALUEW, LINESIZE, VALUEW / CHARSIZE);
  Nodes = new TStatic(this, -1, EmptyStr, NODEX, NODEY,
    NODEW, LINESIZE, NODEW / CHARSIZE);
  Seconds = new TStatic(this, -1, EmptyStr, SECX, SECY,
    SECW, LINESIZE, SECW / CHARSIZE);
  Depth = new TNoEraseBkGndStatic(this, -1, EmptyStr, DEPTHX, DEPTHY,
    DEPTHW, LINESIZE, WORD(DEPTHW /CHARSIZE));
  BestLine = new TStatic(this, -1, EmptyStr, BESTX, BESTY,
    BESTW, BESTH, ((BESTW / CHARSIZE) * 3));
  Message = new TStatic(this, -1, EmptyStr, MESSX, MESSY,
    MESSW, LINESIZE, MESSW / CHARSIZE);
  SetBkgndColor(TColor::LtGray);
}

void
TInfoWindow::EvPaint()
{
  TPaintDC paintDC(HWindow);
  DrawInfoFrame(paintDC);
  DrawStaticFrames(paintDC);
}

HBRUSH
TInfoWindow::EvCtlColor(HDC hDC, HWND, UINT)
{
  SetBkColor(hDC, TColor(192, 192, 192));
  return (HBRUSH)GetStockObject(LTGRAY_BRUSH);
}

void
TInfoWindow::Reset()
{
  Color->SetText("îí");
  WhiteInfo->Clear();
  BlackInfo->Clear();
  Level->Clear();
  Value->Clear();
  Nodes->Clear();
  Seconds->Clear();
  BestLine->Clear();
  Message->Clear();
}

void
TInfoWindow::IterReset()
{
  Color->SetText("îí");
  Value->Clear();
  Nodes->Clear();
  Seconds->Clear();
  BestLine->Clear();
}

void
TInfoWindow::DrawSFrame(TDC& dc, TRect& rect)
{
  dc.SelectStockObject(LTGRAY_BRUSH);
  dc.PatBlt(rect);

  TPoint points[3];
  TPen pen(TColor::Gray);
  dc.SelectObject(pen);
  points[1].x = points[0].x = rect.left;
  points[0].y = rect.bottom;
  points[2].y = points[1].y = rect.top;
  points[2].x = rect.right;
  dc.Polyline(points, 3);

  dc.SelectStockObject(WHITE_PEN);
  points[1].x = rect.right;
  points[1].y = rect.bottom;
  points[2].y = rect.top - 1;
  dc.Polyline(points, 3);
}

void
TInfoWindow::DrawStaticFrames(TDC& dc)
{
  TRect rect;

  rect.left = LEFTMARGIN - 1;
  rect.right = MAXLINE + 1;

  rect.top = COLORY - 1;
  rect.bottom = COLORY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = WHITEINFOY - 1;
  rect.bottom = WHITEINFOY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = BLACKINFOY - 1;
  rect.bottom = BLACKINFOY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = TIMERY - 1;
  rect.bottom = TIMERY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = LEVELY - 1;
  rect.bottom = LEVELY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = VALUEY - 1;
  rect.bottom = VALUEY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = NODEY - 1;
  rect.bottom = NODEY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = SECY - 1;
  rect.bottom = SECY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = DEPTHY - 1;
  rect.bottom = DEPTHY + LINESIZE + 1;
  DrawSFrame(dc, rect);

  rect.top = BESTY - 1;
  rect.bottom = BESTY + BESTH + 1;
  DrawSFrame(dc, rect);

  rect.top = MESSY - 1;
  rect.bottom = MESSY + LINESIZE + 1;
  DrawSFrame(dc, rect);
}

void
TInfoWindow::DrawInfoFrame(TDC& dc)
{
  DrawFrame(dc, InfoRect);
}
