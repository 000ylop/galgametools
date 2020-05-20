object Form2: TForm2
  Left = 256
  Top = 180
  BorderStyle = bsNone
  Caption = 'Form2'
  ClientHeight = 273
  ClientWidth = 427
  Color = clBlack
  Font.Charset = SHIFTJIS_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = #65325#65331' '#65328#12468#12471#12483#12463
  Font.Style = []
  OldCreateOrder = False
  PopupMenu = PopupMenu1
  WindowState = wsMaximized
  OnActivate = FormActivate
  OnClose = FormClose
  OnCreate = FormCreate
  OnDblClick = FormDblClick
  OnHide = FormHide
  OnKeyDown = FormKeyDown
  OnMouseDown = FormMouseDown
  OnMouseUp = FormMouseUp
  OnMouseWheelDown = FormMouseWheelDown
  OnMouseWheelUp = FormMouseWheelUp
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 12
  object PopupMenu1: TPopupMenu
    Left = 256
    Top = 75
    object Save1: TMenuItem
      Caption = 'Save'
      OnClick = Save1Click
    end
    object Close1: TMenuItem
      Caption = 'Close'
      OnClick = Close1Click
    end
    object SlideShow: TMenuItem
      Caption = 'ShowSlide'
      OnClick = SlideShowClick
    end
  end
end
