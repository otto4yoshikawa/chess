object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'Form1'
  ClientHeight = 725
  ClientWidth = 998
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnMouseDown = onmouse
  OnMouseMove = onmoudemove
  OnMouseUp = onmpouseup
  OnPaint = paint
  PixelsPerInch = 96
  TextHeight = 13
  object Memo1: TMemo
    Left = 680
    Top = 357
    Width = 233
    Height = 198
    Lines.Strings = (
      'Memo1')
    TabOrder = 0
    OnKeyDown = onkeydown
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 706
    Width = 998
    Height = 19
    Panels = <
      item
        Width = 50
      end
      item
        Width = 50
      end
      item
        Width = 50
      end
      item
        Width = 60
      end
      item
        Width = 100
      end>
  end
  object Button1: TButton
    Left = 830
    Top = 0
    Width = 107
    Height = 25
    Caption = 'computer play'
    TabOrder = 3
    OnClick = Button1Click
  end
  object ListBox1: TListBox
    Left = 696
    Top = 31
    Width = 136
    Height = 114
    ItemHeight = 13
    Items.Strings = (
      '11111'
      '2222'
      '4444'
      'o-o'
      '888'
      '888'
      '888'
      '8888'
      '1'
      '1'
      '1'
      '1'
      '1'
      '1'
      '1'
      '4444'
      '22222'
      ''
      '333'
      '3'
      '3'
      '3'
      '3'
      '3'
      '3')
    MultiSelect = True
    ScrollWidth = 4
    TabOrder = 4
    OnEnter = onenter
  end
  object StringGrid1: TStringGrid
    Left = 680
    Top = 151
    Width = 185
    Height = 200
    ColCount = 3
    FixedColor = clMedGray
    RowCount = 45
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goDrawFocusSelected]
    TabOrder = 1
    OnSelectCell = cellselected
  end
  object MainMenu1: TMainMenu
    Left = 440
    Top = 32
    object N1: TMenuItem
      Caption = 'File'
      object load1: TMenuItem
        Caption = 'load'
        OnClick = load1Click
      end
    end
    object game1: TMenuItem
      Caption = 'game'
      object newgame1: TMenuItem
        Caption = 'newgame'
        OnClick = newgame1Click
      end
      object undo1: TMenuItem
        Caption = 'undo'
        OnClick = undo1Click
      end
      object resign1: TMenuItem
        Caption = 'resign'
      end
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 496
    Top = 64
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 624
    Top = 208
  end
end
