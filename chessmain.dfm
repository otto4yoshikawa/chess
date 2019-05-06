object Form1: TForm1
  Left = 270
  Top = 210
  Width = 825
  Height = 504
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnDragDrop = dragdrop
  OnMouseDown = mousedown
  OnMouseMove = mousemove
  OnMouseUp = mouseup
  OnPaint = paint
  PixelsPerInch = 96
  TextHeight = 12
  object Memo1: TMemo
    Left = 504
    Top = 32
    Width = 249
    Height = 377
    Lines.Strings = (
      'Memo1')
    TabOrder = 0
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 431
    Width = 817
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
      end>
    SimplePanel = False
  end
  object ToolBar1: TToolBar
    Left = 0
    Top = 0
    Width = 817
    Height = 29
    Caption = 'ToolBar1'
    TabOrder = 2
  end
  object MainMenu1: TMainMenu
    Left = 48
    Top = 56
    object file1: TMenuItem
      Caption = 'file'
      object load1: TMenuItem
        Caption = 'load'
        OnClick = load1Click
      end
      object save1: TMenuItem
        Caption = 'save'
      end
    end
    object tool1: TMenuItem
      Caption = 'tool'
    end
    object game1: TMenuItem
      Caption = 'game'
      OnClick = game1Click
    end
  end
  object OpenDialog1: TOpenDialog
    Left = 128
    Top = 40
  end
  object SaveDialog1: TSaveDialog
    Left = 184
    Top = 48
  end
end
