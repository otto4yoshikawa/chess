//---------------------------------------------------------------------------

#ifndef chesmain2H
#define chesmain2H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE 管理のコンポーネント
	TMemo *Memo1;
	TMainMenu *MainMenu1;
	TMenuItem *N1;
	TMenuItem *load1;
	TOpenDialog *OpenDialog1;
	TStatusBar *StatusBar1;
	TButton *Button1;
	TListBox *ListBox1;
	TMenuItem *game1;
	void __fastcall onmouse(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall paint(TObject *Sender);
	void __fastcall onmoudemove(TObject *Sender, TShiftState Shift, int X, int Y);
	void __fastcall onmpouseup(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall load1Click(TObject *Sender);
	void __fastcall listbox1click(TObject *Sender);
 //	void __fastcall litnoxKeydown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall doubleclick(TObject *Sender);
	void __fastcall onkeydown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall onenter(TObject *Sender);
private:	// ユーザー宣言
public:		// ユーザー宣言
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
