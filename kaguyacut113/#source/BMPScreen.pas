unit BMPScreen;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, Menus;

type
  TForm2 = class(TForm)
    PopupMenu1: TPopupMenu;
    Save1: TMenuItem;
    Close1: TMenuItem;
    SlideShow: TMenuItem;
    procedure FormPaint(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormActivate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormDblClick(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure FormMouseWheelDown(Sender: TObject; Shift: TShiftState;
      MousePos: TPoint; var Handled: Boolean);
    procedure FormMouseWheelUp(Sender: TObject; Shift: TShiftState;
      MousePos: TPoint; var Handled: Boolean);
    procedure FormHide(Sender: TObject);
    procedure Save1Click(Sender: TObject);
    procedure Close1Click(Sender: TObject);
    procedure SlideShowClick(Sender: TObject);
  public
    procedure Movement(Offset,BMPRange,FixRange:Integer; var Start,EndPt:Integer);
    procedure StopTimer;
  end;

var
  Form2: TForm2;

implementation

{$R *.dfm}

uses kaguya, Common;

const  crHand1 = 5;

procedure TForm2.StopTimer;
begin
  Form1.SlideShow.Enabled:=False;
  SlideShow.Checked:=False;
end;

procedure TForm2.FormPaint(Sender: TObject);
begin
  if BMPMove then
  Cursor := crHand1
  else
  Cursor := crDefault;
  BitBlt(Form2.Canvas.Handle,SRECT.Left,SRECT.Top,
  SRECT.Right-SRECT.Left,SRECT.Bottom-SRECT.Top,
  Form1.Image1.Picture.Bitmap.Canvas.Handle,
  PosL,PosT,SRCCOPY);
end;

procedure TForm2.FormCreate(Sender: TObject);
begin
  Form2.BorderStyle:=bsnone;
  Screen.Cursors[crHand1] := LoadCursor(HInstance, 'Hand1');
  SWidth:=Screen.Width;
  Form2.Canvas.Brush.Color:=clBlack;
end;

procedure TForm2.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  Form2.Hide;
end;

procedure TForm2.FormDblClick(Sender: TObject);
begin
  Form2.Hide;
end;

procedure TForm2.Movement(Offset,BMPRange,FixRange:Integer; var Start,EndPt:Integer);
begin
  StopTimer;
  Start:=Start+Offset;
  if Start<0 then Start:=0;
  EndPt:=Start+FixRange;
  if EndPt>BMPRange then
  begin
    EndPt:=BMPRange;
    Start:=EndPt-FixRange;
  end;
end;

procedure TForm2.FormMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
XDiff,YDiff:integer;
begin
   StopTimer;
   if (Button=mbLeft) and BMPMove then
   begin
     XDiff:=PosX-X;
     YDiff:=PosY-Y;

   if DisplayBMP.Width>Screen.Width then
      Movement(Xdiff,DisplayBMP.Width,Screen.Width,PosL,PosR);
   if DisplayBMP.Height>Screen.Height then
      Movement(Ydiff,DisplayBMP.Height,Screen.Height,PosT,PosB);
      Form2.FormPaint(Self);
   end;
end;

procedure TForm2.FormMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
   StopTimer;
   if (Button=mbLeft) and BMPMove then
   begin
      PosX:=X;
      PosY:=Y;
   end;
end;

procedure TForm2.FormActivate(Sender: TObject);
begin
  if SWidth<>Screen.Width then
  begin
    Placement;
    SWidth:=Screen.Width;
  end;
end;

procedure TForm2.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var
XDiff,YDiff:integer;
begin
   case Key of

   VK_Left,VK_Right:
   begin
     StopTimer;
     if Key=VK_Left then Xdiff:=-50 else Xdiff:=50;
     if DisplayBMP.Width>Screen.Width then
     Movement(Xdiff,DisplayBMP.Width,Screen.Width,PosL,PosR);
     Form2.FormPaint(Self)
   end;

   VK_Up,VK_Down:
   begin
     StopTimer;
     if Key=VK_Up then Ydiff:=-50 else Ydiff:=50;
     if DisplayBMP.Height>Screen.Height then
     Movement(Ydiff,DisplayBMP.Height,Screen.Height,PosT,PosB);
     Form2.FormPaint(Self)
   end;

   VK_ESCAPE, VK_RETURN: Form2.Hide;

   $20:   //SpaceBar,Next
   begin
     StopTimer;
     Form1.ListView.MultiSelect:=False;
     if LVIndex < Form1.ListView.Items.Count-1 then
     begin
       Form1.ListView.ItemIndex:=LVIndex+1;
       GetCG(Form1.ListView.ItemIndex);
       if Form2.Showing then
       Form1.Image1Click(Self);
     end;
   end;

   $08:    //Backspace,Prev
   begin
     StopTimer;
     Form1.ListView.MultiSelect:=False;
     if LVIndex >0 then
     begin
       Form1.ListView.ItemIndex:=LVIndex-1;
       GetCG(Form1.ListView.ItemIndex);
       if Form2.Showing then
       Form1.Image1Click(Self);
     end;
   end;

   $46: begin   //F,Faster
        if Form1.TrackBar1.Position > Form1.TrackBar1.Min then
           Form1.TrackBar1.Position:=Form1.TrackBar1.Position
                                  -Form1.TrackBar1.LineSize;
        end;

   $4C: begin     //L,Slower
        if Form1.TrackBar1.Position < Form1.TrackBar1.Max then
           Form1.TrackBar1.Position:=Form1.TrackBar1.Position
                               +Form1.TrackBar1.LineSize;
        end;
  end;
end;

procedure TForm2.FormMouseWheelDown(Sender: TObject; Shift: TShiftState;
  MousePos: TPoint; var Handled: Boolean);
begin
  PostMessage(Form2.Handle,WM_KEYDOWN,$20,0);
end;

procedure TForm2.FormMouseWheelUp(Sender: TObject; Shift: TShiftState;
  MousePos: TPoint; var Handled: Boolean);
begin
  PostMessage(Form2.Handle,WM_KEYDOWN,$08,0);
end;

procedure TForm2.FormHide(Sender: TObject);
var
pt:TPoint;
begin
  StopTimer;
  Form1.ListView.MultiSelect:=True;
  Form1.ListView.ItemIndex:=LVIndex;
  Form1.Show;
  Form1.ListView.Selected.MakeVisible(False);
  Form1.ListView.Scroll(0,(LVIndex-Form1.ListView.TopItem.Index-9)*16);
  pt:=Form1.ListView.Selected.GetPosition;
  PostMessage(Form1.ListView.Handle,WM_RBUTTONDOWN,0,pt.X+pt.Y shl 16);
  PostMessage(Form1.ListView.Handle,WM_RBUTTONUP,0,pt.X+pt.Y shl 16);
end;

procedure TForm2.Save1Click(Sender: TObject);
begin
  StopTimer;
  if Form1.Edit.Text<>'' then
  Form1.SaveDialog1.InitialDir:=IncludeTrailingPathDelimiter(Form1.Edit.Text)
  else
  Form1.SaveDialog1.InitialDir:=GetCurrentDir;
  Form1.SaveDialog1.FileName:=ChangeFileExt(ExtractFilename(DBMPName),'');
  if Form1.SaveDialog1.Execute then
  if Form1.SaveDialog1.FilterIndex=1 then
     DisplayBMP.SaveToFile(Form1.SaveDialog1.FileName)
  else
  begin
    DisplayPNG.Assign(DisplayBMP);
    DisplayPNG.SaveToFile(Form1.SaveDialog1.FileName);
  end;

  Form2.Hide;
end;

procedure TForm2.Close1Click(Sender: TObject);
begin
  Form2.Hide;
end;

procedure TForm2.SlideShowClick(Sender: TObject);
begin
  SlideShow.Checked:=not SlideShow.Checked;
  if SlideShow.Checked then Form1.SlideShow.Enabled:=True
  else Form1.SlideShow.Enabled:=False;
end;

end.
