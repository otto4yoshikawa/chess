//----------------------------------------------------------------------------
// ObjectWindows - (C) Copyright 1991, 1993 by Borland International
//   examples\owl\chess\wchess.cpp
//
//   OWL Chess は、 TURBO PASCAL GameWorks でリリースされたオリジナルを
//   ベースにしたチェスプログラムです。チェスエンジンはＣに移植されただ
//   けで、本質的な変更はありません。このサンプルプログラムは、DOS コード
//   周辺をウィンドウズ用にした時の OWL の使われ方を示し、それらの部分は、
//   C または C++ で記述されています。
//----------------------------------------------------------------------------
/*
#include <owl/pch.h>
#include <owl/applicat.h>
#include <owl/framewin.h>
#include <owl/static.h>
#include <owl/opensave.h>
#include <owl/inputdia.h>
#include <owl/dc.h>
#include <string.h>
#include <stdio.h>
*/
#include <vcl.h>
#include "wcdefs.h"
#include "info.h"
#include "wchess.h"
#include "edit.h"
#include "colors.h"
#include "pvalue.h"
#include "externs.h"

#undef MAXPATH
#define MAXPATH  160                      //
//  グローバル変数
//
TColor WhiteSquareColor = clWhite;
TColor BlackSquareColor=clGreen;     // 緑

BOOL         ShowBestLine = true;
TInfoWindow* TInfo;
HBRUSH       hBlackBrush;
HBRUSH       hWhiteBrush;
HWND         hWndMain;
HBITMAP      PieceBmpArray[6][2];
HBITMAP      MaskArray[6];
HCURSOR      hArrowCursor;
HCURSOR      hWaitCursor;
HMENU        ThinkMenu;
HMENU        MainMenu;
HANDLE       hAccel;
COLORTYPE    ComputerColor;
short        LINESIZE;
short        CHARSIZE;
int          CAPTIONY;
BOOL         Editing;
int          BORDERSIZE;
const        BORDERYEXTRA = 4; // ４ピクセルの隙間
short        INFOXSIZE, INFOYSIZE;
BOOL         SoundOn;


// --------------- TMessageDialog ----------------------

//
// BWCC スタイルのメッセージボックスを RC ファイルからの静的テンプレート
// から作成します。他に、各メッセージのサイズに丁度いいメッセージボック
// スを動的に作成したり、表示するボタンのタイプをユーザが指定したり、
// ウィンドウズの MessageBox のようにしてもいいでしょう。
//
class TMessageDialog : public TDialog {
  public:
    TMessageDialog(TWindow* parent, int resourceId, const char far* message,
                   const char far* title);

  protected:
    void SetupWindow();

  private:
    const char far* NewMessage;
    const char far* NewTitle;
};

TMessageDialog::TMessageDialog(TWindow* parent, int resourceId,
                               const char far* message, const char far* title)
:
  TDialog(parent, resourceId)
{
   NewMessage = message;
   NewTitle = title;
}

void
TMessageDialog::SetupWindow()
{
  // 祖先の SetupWindow を最後に呼び出して失敗すると、結果が不定になる。
  // 常に、派生した SetupWindow を最初に呼ぶべき。
  //
  TDialog::SetupWindow();
  SetDlgItemText(ID_INPUT, NewMessage);
  SetWindowText(NewTitle);
}

// --------------- TEndDialog ----------------------

class TEndDialog : public TDialog {
  public:
    TEndDialog(TWindow* parent, int resourceId, const char far* message);

  protected:
    void SetupWindow();
    void CmYes() {CloseWindow(IDYES);}
    void CmNo() {CloseWindow(IDNO);}

  private:
    const char far* EndMessage;

  DECLARE_RESPONSE_TABLE(TEndDialog);
};

DEFINE_RESPONSE_TABLE1(TEndDialog, TDialog)
  EV_COMMAND(IDYES, CmYes),
  EV_COMMAND(IDNO, CmNo),
END_RESPONSE_TABLE;

TEndDialog::TEndDialog(TWindow* parent, int resourceId, const char far* message)
:
  TDialog(parent, resourceId)
{
  EndMessage = message;
}

void
TEndDialog::SetupWindow()
{
  TDialog::SetupWindow();
  SetDlgItemText(ID_INPUT, EndMessage);
}

// --------------- TChessWindow ----------------------

class TChessWindow : public TWindow {
  public:
    TChessWindow();
   ~TChessWindow();
    void SetupWindow();
    void GetWindowClass(WNDCLASS& wndClass);
    char far* GetClassName() {return "ChessWindow";}

    void Paint(TDC& dc, bool erase, TRect& rect);

    void EvLButtonDown(uint, TPoint&);
    void EvMouseMove(uint, TPoint&);

    void CmNewGame();
    void CmRestoreGame();
    void CmSaveGame();
    void CmSaveGameAs();
    void CmUndoMove();
    void CmRedoUndo();
    void CmMovePieces();
    void CmMoveTime();
    void CmTotalTime();
    void CmDemo();
    void CmInfiniteSearch();
    void CmPlySearch();
    void CmMateSearch();
    void CmSinglePlayer();
    void CmTwoPlayer();
    void CmAutoPlay();
    void CmReverseBoard();
    void CmEasy();
    void CmHint();
    void CmPass();
    void CmDone();
    void CmClear();
    void CmCancel();
    void CmError();
    void CmMatching();
    void CmWhiteTurn();
    void CmBlackTurn();
    void CmColors();
    void CmPieceValues();
    void CmBestLine();
    void CmMouseDrag();
    void CmMouseSelect();
    void CmSound();
    void EvTimer(uint);

    void EndGame();
    void DoEdit(TPoint& point);
    void ComputersTurn();
    void UnCheckLevelMenu(LEVELTYPE);
    void NormalSetup();
    void CheckLevelMenu(LEVELTYPE);
    bool CanClose() {
      if (ComputerThinking || AutoPlay) {
        PostMessage(WM_COMMAND, CM_STOP);
        PostMessage(WM_COMMAND, IDM_EXIT);
        return false;
      }
      QuitProgram();
      return true;
    }

  private:
    enum TURNTYPE { player, computer };
    TInfoWindow*    TInfo;    // 情報ウィンドウ
    TEditBarWindow* TEditBar; // エディットバー
    bool       NewGame;       // 新しいゲームがセットアップされているか?
    char*      FileName;      // 最後に保存したゲーム
    TRect      MainWndRect;   // メインウィンドウの座標
    TRect      InfoAreaRect;  // 情報ウィンドウの座標
    HMENU      hMenu;         // メインメニューのハンドル
    HMENU      hThinkMenu;    // 小メニュー（「停止」）のハンドル
    HMENU      hEditMenu;     // 編集（「アレンジ」）メニューのハンドル
    TURNTYPE   WhoseTurn;     // プレイヤーかコンピュータの順番か？
    SQUARETYPE MoveStartSquare; // 移動の最初のマス
    SQUARETYPE MoveEndSquare; // 移動の終りのマス
    COLORTYPE  CurPlayer;     // 現在のプレイヤー
    bool       GotStartSquare; // ユーザは移動するために正しい駒を選択したか?
    HBRUSH     hBKBrush;      // メインウィンドウの背景を塗るブラシのハンドル
    HBITMAP    hBKBrushBmp;   // メインウィンドウの背景を塗る小さいビットマップ
    bool       Modified;      // 編集モードでボードが変更されたかどうか?
    bool       EditingBoard;  // ユーザがボードを編集しているかどうか
    bool       Dragging;      // ドラッグモード?
    BOARDTYPE* SaveBoard;     // ユーザが編集をキャンセルした場合のボードの保存

  DECLARE_RESPONSE_TABLE(TChessWindow);
};

// ------------- TChessWindow 応答テーブル関数 -----------------------

DEFINE_RESPONSE_TABLE1(TChessWindow, TWindow)
  EV_WM_LBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_COMMAND(CM_FILENEW, CmNewGame),
  EV_COMMAND(CM_FILEOPEN, CmRestoreGame),
  EV_COMMAND(CM_FILESAVE, CmSaveGame),
  EV_COMMAND(CM_FILESAVEAS, CmSaveGameAs),
  EV_COMMAND(CM_UNDO, CmUndoMove),
  EV_COMMAND(CM_REDO, CmRedoUndo),
  EV_COMMAND(IDM_MOVEPIECE, CmMovePieces),
  EV_COMMAND(IDM_MOVETIME, CmMoveTime),
  EV_COMMAND(IDM_TOTALTIME, CmTotalTime),
  EV_COMMAND(IDM_DEMO, CmDemo),
  EV_COMMAND(IDM_INFINITE, CmInfiniteSearch),
  EV_COMMAND(IDM_PLY, CmPlySearch),
  EV_COMMAND(IDM_MATE, CmMateSearch),
  EV_COMMAND(IDM_SINGLE, CmSinglePlayer),
  EV_COMMAND(IDM_TWOPLAYER, CmTwoPlayer),
  EV_COMMAND(IDM_AUTO, CmAutoPlay),
  EV_COMMAND(IDM_REVERSE, CmReverseBoard),
  EV_COMMAND(IDM_EASY, CmEasy),
  EV_COMMAND(IDM_HINT, CmHint),
  EV_COMMAND(IDM_PASS, CmPass),
  EV_COMMAND(EM_DONE, CmDone),
  EV_COMMAND(EM_CLEAR, CmClear),
  EV_COMMAND(EM_CANCEL, CmCancel),
  EV_COMMAND(EM_ERROR, CmError),
  EV_COMMAND(IDM_MATCHING, CmMatching),
  EV_COMMAND(IDM_WHITETURN, CmWhiteTurn),
  EV_COMMAND(IDM_BLACKTURN, CmBlackTurn),
  EV_COMMAND(IDM_COLORS, CmColors),
  EV_COMMAND(IDM_PIECEVALUES, CmPieceValues),
  EV_COMMAND(IDM_BESTLINE, CmBestLine),
  EV_COMMAND(IDM_MOUSEDRAG, CmMouseDrag),
  EV_COMMAND(IDM_MOUSESELECT, CmMouseSelect),
  EV_COMMAND(IDM_SOUND, CmSound),

  EV_WM_TIMER,
END_RESPONSE_TABLE;

// ------------- TChessWindow メンバ関数 -----------------

TChessWindow::TChessWindow()
:
  TWindow(0, 0, 0)
{
  TScreenDC dc;
  TEXTMETRIC tm;
  dc.GetTextMetrics(tm);
  CHARSIZE = short(tm.tmAveCharWidth);
  LINESIZE = short(tm.tmHeight + tm.tmExternalLeading);

  CAPTIONY = ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYMENU);
  BORDERSIZE = LINESIZE + MYFRAMESIZE;
  TInfo = new TInfoWindow(this, "InfoWindow");
  TEditBar = new TEditBarWindow(this, "EditBar");
  SoundOn = true;
  ::TInfo = TInfo;

  Attr.Style = WS_CHILD | WS_VISIBLE;
  Attr.X = 5;
  Attr.Y = 5;
  Attr.H = CAPTIONY + BORDERSIZE*2 + INFOYSIZE + BORDERYEXTRA +
    2*GetSystemMetrics(SM_CYBORDER);

  Attr.W = BORDERSIZE*4 + MAXBDSIZE*SQUARE_SIZE + INFOXSIZE +
    2*GetSystemMetrics(SM_CXBORDER) + 2*MYFRAMESIZE;

  SetBkgndColor(TColor::Sys3dFace);

  FileName = new char[MAXPATH];
  WhoseTurn = player;
  CurPlayer = white;
  ::ComputerColor = black;
  ::Editing = EditingBoard = GotStartSquare = FALSE;
  NewGame = TRUE;
}

TChessWindow::~TChessWindow()
{
  delete FileName;
  delete TInfo;
  ::DeleteObject(hWhiteBrush);
  ::DeleteObject(hBlackBrush);
  ::DeleteObject(hBKBrushBmp);
  ::DestroyMenu(hThinkMenu);
  ::DestroyMenu(hEditMenu);
  for (int i = 0; i < 6; i++) {
    ::DeleteObject(MaskArray[i]);
    for (int j = 0; j < 2; j++)
      ::DeleteObject(PieceBmpArray[i][j]);
  }
  KillTimer(TIMEID);
}

void
TChessWindow::GetWindowClass(WNDCLASS& wndClass)
{
  // これも、SetupWindow と InitInstance のように、
  // 最初に祖先を呼ぶ
  //
  TWindow::GetWindowClass(wndClass);

  //  OWL に登録するウィンドウクラスに固有の値を設定
  //
  ::hArrowCursor = wndClass.hCursor = ::LoadCursor(0, IDC_ARROW);
  hBKBrushBmp = GetApplication()->LoadBitmap("BKBrush");
  hBKBrush = ::CreatePatternBrush(hBKBrushBmp);

  wndClass.hbrBackground = hBKBrush;
  wndClass.lpszMenuName = 0;
  wndClass.hIcon = LoadIcon(*GetApplication(), "ChessIcon");
}

void
TChessWindow::SetupWindow()
{
  TWindow::SetupWindow();

  ::hWndMain = GetHandle();
  ::hWaitCursor = ::LoadCursor(0, IDC_WAIT);

  ::PieceBmpArray[pawn-1][white] = GetApplication()->LoadBitmap("WPawn");
  ::PieceBmpArray[pawn-1][black] = GetApplication()->LoadBitmap("BPawn");
  ::PieceBmpArray[rook-1][white] = GetApplication()->LoadBitmap("WRook");
  ::PieceBmpArray[rook-1][black] = GetApplication()->LoadBitmap("BRook");
  ::PieceBmpArray[knight-1][black] = GetApplication()->LoadBitmap("BKnight");
  ::PieceBmpArray[bishop-1][black] = GetApplication()->LoadBitmap("BBishop");
  ::PieceBmpArray[queen-1][black] = GetApplication()->LoadBitmap("BQueen");

  ::PieceBmpArray[knight-1][white] = GetApplication()->LoadBitmap("WKnight");
  ::PieceBmpArray[bishop-1][white] = GetApplication()->LoadBitmap("WBishop");
  ::PieceBmpArray[queen-1][white] = GetApplication()->LoadBitmap("WQueen");

  ::PieceBmpArray[king-1][white] =  GetApplication()->LoadBitmap("WKing");
  ::PieceBmpArray[king-1][black] =  GetApplication()->LoadBitmap("BKing");

  ::MaskArray[pawn-1] =   GetApplication()->LoadBitmap("PMask");
  ::MaskArray[rook-1] =   GetApplication()->LoadBitmap("RMask");
  ::MaskArray[knight-1] = GetApplication()->LoadBitmap("KTMask");
  ::MaskArray[bishop-1] = GetApplication()->LoadBitmap("BMask");
  ::MaskArray[queen-1] =  GetApplication()->LoadBitmap("QMask");
  ::MaskArray[king-1] =   GetApplication()->LoadBitmap("KMask");

  InfoAreaRect = MainWndRect = GetClientRect();
  InfoAreaRect.left = MainWndRect.right -= BORDERSIZE*2 + INFOXSIZE;

  MainMenu = hMenu = GetApplication()->LoadMenu("TChessMenu");
  ::ThinkMenu = hThinkMenu = GetApplication()->LoadMenu("TChessThinkMenu");
  hEditMenu = GetApplication()->LoadMenu("TChessEditMenu");
  GetParentO()->SetMenu(hMenu);

  ::Talk();
  CheckLevelMenu(Level);
  ::hWhiteBrush = ::CreateSolidBrush(WhiteSquareColor);
  ::hBlackBrush = ::CreateSolidBrush(BlackSquareColor);

  Dragging = true;
}

void
TChessWindow::Paint(TDC& dc, bool, TRect&)
{
  DrawFrame(dc, MainWndRect);
  DrawFrame(dc, InfoAreaRect);
  PrintBoard();
}

void
TChessWindow::EvLButtonDown(uint, TPoint& point)
{
  if (EditingBoard) {
    DoEdit(point);
    return;
  }

  if (WhoseTurn == computer && !NoComputerMove)
    return;

  if (!GotStartSquare) {
    MoveStartSquare = GetValidSquare(point, CurPlayer, true);
    if (MoveStartSquare == -1)
      return;
    GotStartSquare = true;
    if (!Dragging)
      DrawInvertedBitmap(MoveStartSquare);
    else
      DragStart(MoveStartSquare, point);

  }
  else {
    MoveEndSquare = GetValidSquare(point, CurPlayer, false);
    GotStartSquare = false;
    if (MoveEndSquare == -1) {
      if (Dragging)
        DragEnd(FALSE);
      Warning("不正な移動です");
      DrawNormalBitmap(MoveStartSquare);
      return;
    }
    if (!MoveCheck(MoveStartSquare, MoveEndSquare)) {
      if (Dragging)
        DragEnd(FALSE);
      DrawNormalBitmap(MoveStartSquare);
      return;
    }

    ::GotValidMove = true;
    ::EnableMenuItem(hMenu, CM_UNDO, MF_BYCOMMAND | MF_ENABLED);
    if (GameOver)
      EndGame();
    else if (NoComputerMove == false)
      ComputersTurn();
    else if (::MultiMove)
      CurPlayer = (CurPlayer == white) ? black : white;
  }
}

void
TChessWindow::ComputersTurn()
{
  WhoseTurn = computer;
  SetClassWindowCursor(GetHandle(), ::hWaitCursor);
  GetParentO()->SetMenu(hThinkMenu);
  ProgramMove();
  GetParentO()->SetMenu(hMenu);

  WhoseTurn = player;
  SetClassWindowCursor(GetHandle(), hArrowCursor);
  if (GameOver)
    EndGame();
}

void
TChessWindow::CmNewGame()
{
  KillTimer(TIMEID);
  TInfo->SetTimerText("");
  ::EnableMenuItem(hMenu, CM_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  ::EnableMenuItem(hMenu, CM_REDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  NewGame = TRUE;
  ::NewGame();
  ::PrintBoard();
  CurPlayer = Player;
  ComputerColor = Opponent;
}

void
TChessWindow::CmRestoreGame()
{
  TOpenSaveDialog::TData data (
    OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_NOREADONLYRETURN,
    "ﾁｪｽｹﾞｰﾑﾌｧｲﾙ (*.CHS)\0*.chs\0",
    0,
    0,
    "CHS"
  );

  if (TFileOpenDialog(this, data).Execute() == IDOK) {
    NewGame = FALSE;
    strcpy(FileName, data.FileName);
    ::RestoreGame(FileName);
    CurPlayer = Player;
    ComputerColor = Opponent;
    ::EnableMenuItem(hMenu, CM_REDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    ::EnableMenuItem(hMenu, CM_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  }
}

void
TChessWindow::CmSaveGameAs()
{
  TOpenSaveDialog::TData data (
    OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
    "ﾁｪｽｹﾞｰﾑﾌｧｲﾙ (*.CHS)|*.chs|",
    0,
    0,
    "CHS"
  );
  if (TFileSaveDialog(this, data).Execute() == IDOK) {
    NewGame = FALSE;
    strcpy(FileName, data.FileName);
    ::SaveGame(FileName);
  }
}

void
TChessWindow::CmSaveGame()
{
  if (NewGame)
    CmSaveGameAs();
  else
    ::SaveGame(FileName);
}

void
TChessWindow::CmError()
{
  TMessageDialog(this, DR_CHESSERROR, buf, "OWL Chess ｴﾗｰ").Execute();
}

void
TChessWindow::DoEdit(TPoint& point)
{
  Modified = TRUE;

  SQUARETYPE square = GetValidSquare(point, black, FALSE);
  if (Board[square].piece != empty) {
    Board[square].piece = empty;
    ::UpdateBoard();
    return;
  }
  int selectedItem = TEditBar->GetSelectedItem();

  Board[square].piece = (PIECETYPE)(selectedItem % 6 + 1);
  Board[square].color = (selectedItem < 6) ? white : black;
  ::UpdateBoard();
}

void
TChessWindow::CmMovePieces()
{
  SaveBoard = new BOARDTYPE[0x78];
  if (!SaveBoard) {
    Error("ﾒﾓﾘ不足です");
    return;
  }
  memcpy(SaveBoard, Board, sizeof(BOARDTYPE) * 0x78);
  Editing = EditingBoard = TRUE;
  TInfo->Show(SW_HIDE);
  GetParentO()->SetMenu(hEditMenu);
  Modified = FALSE;
  if (CurPlayer == white) {
    CheckMenuItem(hEditMenu, IDM_BLACKTURN, MF_UNCHECKED);
    CheckMenuItem(hEditMenu, IDM_WHITETURN, MF_CHECKED);
  } else {
    CheckMenuItem(hEditMenu, IDM_BLACKTURN, MF_CHECKED);
    CheckMenuItem(hEditMenu, IDM_WHITETURN, MF_UNCHECKED);
  }
  TEditBar->Show(SW_NORMAL);
}

void
TChessWindow::CmDone()
{
  SQUARETYPE sq;
  int        kingCount[2] = { 0, 0 };
  int        totalCount[2] = { 0, 0 };
  bool       done = false;

  if (Modified) {
    for (sq = 0; sq < 0x78; sq++)
      if (!(sq & 0x88)) {
        if (Board[sq].piece != empty) {
          totalCount[Board[sq].color]++;
          if (Board[sq].piece == king)
            kingCount[Board[sq].color]++;
        }
      }
    if (totalCount[white] <= 16 && kingCount[white] == 1 &&
        totalCount[black] <= 16 && kingCount[black] == 1) {
      ResetNewPos();
      if (!Attacks(Player, PieceTab[Opponent][0].isquare))
        done = TRUE;
      else
        Error("ｷﾝｸﾞの位置が不正です");
    } else
       Error("駒の個数が不正です");
  } else
    NormalSetup();

  if (done)
    NormalSetup();
}

void
TChessWindow::CmClear()
{
  Modified = TRUE;
  for (SQUARETYPE sq = 0; sq <= 0x77; sq++)
    Board[sq].piece = empty;
  ::UpdateBoard();
}

void
TChessWindow::CmCancel()
{
  memcpy(Board, SaveBoard, sizeof(BOARDTYPE) * 0x78);
  ResetNewPos();
  NormalSetup();
}

void
TChessWindow::NormalSetup()
{
  TEditBar->Show(SW_HIDE);
  TInfo->Show(SW_NORMAL);
  GetParentO()->SetMenu(hMenu);
  delete [] SaveBoard;
  Editing = EditingBoard = FALSE;
  ::UpdateBoard();
}

void
TChessWindow::CmMoveTime()
{
  char* secsPerMove = new char[40];
  sprintf(secsPerMove, "%.2lf", ::AverageTime);

  if (TInputDialog(this, "１手の時間設定",
      "１手の時間(秒):", secsPerMove, 19).Execute() == IDOK) {
    double newMoveTime = atof(secsPerMove);
    if (newMoveTime > 0.0) {
      if (::Level != normal) {
        UnCheckLevelMenu(Level);
        ::CheckMenuItem(hMenu, IDM_MOVETIME, MF_CHECKED);
      }
      ::Level = normal;
      ::AverageTime = newMoveTime;
      PrintCurLevel();
      for (COLORTYPE color = white; color <= black; ((int)color)++)
        ::ChessTime[color].totaltime = (::MoveNo / 2) * ::AverageTime;
      ::MaxLevel = MAXPLY;

    } else
      Error("入力が不正です. 変更は行われません.");
  }
  delete secsPerMove;
}

void
TChessWindow::CmTotalTime()
{
  char* totalMoveTime = new char[40];
  sprintf(totalMoveTime, "%.2lf", ::AverageTime);

  if (TInputDialog(this, "試合の時間設定", "試合の時間(分):",
      totalMoveTime, 19).Execute() == IDOK) {
    double newMoveTime = atof(totalMoveTime);
    if (newMoveTime > 0.0) {
      if (::Level != fullgametime) {
        UnCheckLevelMenu(Level);
        ::CheckMenuItem(hMenu, IDM_TOTALTIME, MF_CHECKED);
      }
      ::Level = fullgametime;
      ::AverageTime = newMoveTime;
      ::PrintCurLevel();
      for (COLORTYPE color = white; color <= black; ((int)color)++)
        ::ChessTime[color].totaltime = (::MoveNo / 2) * ::AverageTime;
      ::MaxLevel = MAXPLY;

    } else
      Error("入力が不正です. 変更は行われません.");
  }
  delete totalMoveTime;
}

void
TChessWindow::CmDemo()
{
  bool isEasy = false;
  if (::Level == easygame) {
    isEasy = true;
    ::Level = normal;
    HideAttacks();
  }
  AutoPlay = true;
  ComputersTurn();
  if (isEasy) {
    ::Level = easygame;
    ::UpdateBoard();
  }
  CurPlayer = Player;
  ComputerColor = Opponent;
  ::PrintCurLevel();
}

void
TChessWindow::CmMatching()
{
  if (::Level != matching) {
    UnCheckLevelMenu(Level);
    ::CheckMenuItem(hMenu, IDM_MATCHING, MF_CHECKED);
  }
  ::Level = matching;
  ::PrintCurLevel();
}

void
TChessWindow::CmInfiniteSearch()
{
  if (::Level != infinite) {
    UnCheckLevelMenu(Level);
    ::CheckMenuItem(hMenu, IDM_INFINITE, MF_CHECKED);
  }
  ::Level = infinite;
  ::MaxLevel = MAXPLY;
  ::PrintCurLevel();
}

void
TChessWindow::CmPlySearch()
{
  char* plySearchDepth = new char[40];
  sprintf(plySearchDepth, "%d", ::MaxLevel);

  if (TInputDialog(this, "先読み深さ設定", "先読みの深さ:",
      plySearchDepth, 19).Execute() == IDOK) {
    int newPlyDepth = atoi(plySearchDepth);
    if (newPlyDepth > 0) {
      if (::Level != plysearch) {
        UnCheckLevelMenu(Level);
        ::CheckMenuItem(hMenu, IDM_PLY, MF_CHECKED);
      }
      ::MaxLevel = (BYTE)((newPlyDepth > MAXPLY) ? MAXPLY : newPlyDepth);
      ::Level = plysearch;
      ::PrintCurLevel();
    } else
      ::Error("入力が不正です. 変更は行われません.");
  }
  delete[] plySearchDepth;
}

void
TChessWindow::CmMateSearch()
{
  if (::Level != matesearch) {
    UnCheckLevelMenu(Level);
    ::CheckMenuItem(hMenu, IDM_MATE, MF_CHECKED);
  }
  ::Level = matesearch;
  ::PrintCurLevel();
  ComputersTurn();
}

void
TChessWindow::CmSinglePlayer()
{
}

void TChessWindow::CmTwoPlayer()
{
  static LEVELTYPE OldLevel = normal;
  ::MultiMove = !::MultiMove;

  if (::MultiMove) {
    OldLevel = ::Level;
    ::ModifyMenu(hMenu, IDM_TWOPLAYER, MF_BYCOMMAND | MF_STRING,
      IDM_TWOPLAYER, "１人用(&S)");
    ::EnableMenuItem(hMenu, 2, MF_GRAYED | MF_BYPOSITION);
    GetParentO()->DrawMenuBar();
    ::Level = normal;
    ::PrintCurLevel();

  } else {
    ::ModifyMenu(hMenu, IDM_TWOPLAYER, MF_BYCOMMAND | MF_STRING,
      IDM_TWOPLAYER, "２人用(&T)");
    ::EnableMenuItem(hMenu, 2, MF_ENABLED | MF_BYPOSITION);
    GetParentO()->DrawMenuBar();
    ::Level = OldLevel;
    ::PrintCurLevel();
  }
}

void TChessWindow::CmAutoPlay()
{
}

void
TChessWindow::CmReverseBoard()
{
  ::Turned = !::Turned;
  ::PrintBoard();
}

void
TChessWindow::CmUndoMove()
{
  if (ComputerThinking) {
    MessageToPost = CM_UNDO;
    return;
  }
  if (!Undo())
    ::EnableMenuItem(hMenu, CM_UNDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  ::EnableMenuItem(hMenu, CM_REDO, MF_BYCOMMAND | MF_ENABLED);
  CurPlayer = Player;
  ComputerColor = Opponent;
}

void
TChessWindow::CmRedoUndo()
{
  if (ComputerThinking) {
    MessageToPost = CM_REDO;
    return;
  }
  if (!Redo())
    ::EnableMenuItem(hMenu, CM_REDO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
  ::EnableMenuItem(hMenu, CM_UNDO, MF_BYCOMMAND | MF_ENABLED);
  CurPlayer = Player;
  ComputerColor = Opponent;
}

void
TChessWindow::UnCheckLevelMenu(LEVELTYPE level)
{
  WORD checkItem = IDM_EASY;
  switch (level) {
    case normal:
      checkItem = IDM_MOVETIME;
      break;
    case fullgametime:
      checkItem = IDM_TOTALTIME;
      break;
    case plysearch:
      checkItem = IDM_PLY;
      break;
    case easygame:
      checkItem = IDM_EASY;
      break;
    case infinite:
      checkItem = IDM_INFINITE;
      break;
    case matesearch:
      checkItem = IDM_MATE;
      break;
    case matching:
      checkItem = IDM_MATCHING;
      break;
  }
  ::CheckMenuItem(hMenu, checkItem, MF_UNCHECKED);
}

void
TChessWindow::CheckLevelMenu(LEVELTYPE level)
{
  WORD checkItem = IDM_EASY;
  switch (level) {
    case normal:
      checkItem = IDM_MOVETIME;
      break;
    case fullgametime:
      checkItem = IDM_TOTALTIME;
      break;
    case plysearch:
      checkItem = IDM_PLY;
      break;
    case easygame:
      checkItem = IDM_EASY;
      break;
    case infinite:
      checkItem = IDM_INFINITE;
      break;
    case matesearch:
      checkItem = IDM_MATE;
      break;
    case matching:
      checkItem = IDM_MATCHING;
      break;
  }
  ::CheckMenuItem(hMenu, checkItem, MF_CHECKED);
}

void
TChessWindow::CmEasy()
{
  if (::Level != easygame) {
    UnCheckLevelMenu(Level);
    ::CheckMenuItem(hMenu, IDM_EASY, MF_CHECKED);
  }
  ::Level = easygame;
  ::AverageTime = 5.;
  ::MaxLevel = MAXPLY;
  ::PrintCurLevel();
}

void
TChessWindow::EndGame()
{
  if (TEndDialog(this, DR_ENDGAMEDLG, EndGameMessage).Execute() != IDYES) {
    PostMessage(WM_COMMAND, CM_EXIT);
    return;
  }
  GameOver = false;
  PostMessage(WM_COMMAND, CM_FILENEW);
}

void
TChessWindow::CmHint()
{
  FindHintMove();
  ShowHint();
}

void
TChessWindow::CmPass()
{
  if (::Level == easygame)
    HideAttacks();
  CurPlayer = Opponent;
  ComputerColor = Player;
  ComputersTurn();
}

void
TChessWindow::CmWhiteTurn()
{
  CheckMenuItem(hEditMenu, IDM_BLACKTURN, MF_UNCHECKED);
  CheckMenuItem(hEditMenu, IDM_WHITETURN, MF_CHECKED);
  CurPlayer = white;
  ComputerColor = black;
  if (CurPlayer != Player) {
    Opponent = ComputerColor;
    Player = ProgramColor = CurPlayer;
  }
}

void
TChessWindow::CmBlackTurn()
{
  CheckMenuItem(hEditMenu, IDM_BLACKTURN, MF_CHECKED);
  CheckMenuItem(hEditMenu, IDM_WHITETURN, MF_UNCHECKED);
  CurPlayer = black;
  ComputerColor = white;
  if (CurPlayer != Player) {
    Opponent = ComputerColor;
    Player = ProgramColor = CurPlayer;
  }
}

void
TChessWindow::CmColors()
{
  TColorsDialog(this, "ColorsDialog").Execute();
  DeleteObject(hWhiteBrush);
  DeleteObject(hBlackBrush);
  ::hWhiteBrush = CreateSolidBrush(WhiteSquareColor);
  ::hBlackBrush = CreateSolidBrush(BlackSquareColor);
}

void
TChessWindow::CmPieceValues()
{
  TPieceValueDialog(this, "PieceValueDlg").Execute();
}

void
TChessWindow::CmBestLine()
{
  ShowBestLine = !ShowBestLine;

  if (ShowBestLine)
    ::CheckMenuItem(hMenu, IDM_BESTLINE, MF_CHECKED);
  else {
    ::CheckMenuItem(hMenu, IDM_BESTLINE, MF_UNCHECKED);
    TInfo->SetBestLineText("");
  }
}

void
TChessWindow::CmMouseDrag()
{
  TPoint point;

  if (!Dragging && GotStartSquare) {
    DrawNormalBitmap(MoveStartSquare);
    GetCursorPos(point);
    DragStart(MoveStartSquare, point);
  }
  Dragging = true;
  ::CheckMenuItem(hMenu, IDM_MOUSEDRAG, MF_CHECKED);
  ::CheckMenuItem(hMenu, IDM_MOUSESELECT, MF_UNCHECKED);
}

void
TChessWindow::CmMouseSelect()
{
  DragEnd(true);
  Dragging = false;
  ::CheckMenuItem(hMenu, IDM_MOUSEDRAG, MF_UNCHECKED);
  ::CheckMenuItem(hMenu, IDM_MOUSESELECT, MF_CHECKED);
  if (GotStartSquare)
    DrawInvertedBitmap(MoveStartSquare);
}

void
TChessWindow::CmSound()
{
  SoundOn = !SoundOn;
  if (SoundOn)
    ::CheckMenuItem(hMenu, IDM_SOUND, MF_CHECKED);
  else
    ::CheckMenuItem(hMenu, IDM_SOUND, MF_UNCHECKED);
}

void
TChessWindow::EvMouseMove(uint, TPoint& point)
{
  if (!GotStartSquare || !Dragging)
    return;
  Drag(point);
}

void
TChessWindow::EvTimer(uint)
{
  DisplayTime();
}


// --------------- TChess ----------------------

class TChessApp : public TApplication {
  public:
   TChessApp() : TApplication() {}
   void InitMainWindow();
};

void
TChessApp::InitMainWindow()
{
  TFrameWindow* frame = new TFrameWindow(0, "OWL Chess", new TChessWindow, true);
  frame->Attr.AccelTable = "ChessCommands";
  frame->Attr.Style &= ~WS_THICKFRAME;

  SetMainWindow(frame);
  EnableCtl3d();
}

int OwlMain(int,char**)
{
  return TChessApp().Run();
}
