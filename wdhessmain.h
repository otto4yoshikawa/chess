//---------------------------------------------------------------------------

#ifndef wdhessmainH
#define wdhessmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE �Ǘ��̃R���|�[�l���g
	void __fastcall mousedown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall mouseup(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
private:	// ���[�U�[�錾
public:		// ���[�U�[�錾
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
