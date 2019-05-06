//---------------------------------------------------------------------------

#ifndef chessmainH
#define chessmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ToolWin.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
        TMemo *Memo1;
        TStatusBar *StatusBar1;
        TToolBar *ToolBar1;
        TMainMenu *MainMenu1;
        TMenuItem *file1;
        TMenuItem *load1;
        TMenuItem *save1;
        TMenuItem *tool1;
        TMenuItem *game1;
        TOpenDialog *OpenDialog1;
        TSaveDialog *SaveDialog1;
        void __fastcall paint(TObject *Sender);
        void __fastcall dragdrop(TObject *Sender, TObject *Source, int X,
          int Y);
        void __fastcall mousedown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall mouseup(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall mousemove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall load1Click(TObject *Sender);
        void __fastcall game1Click(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
