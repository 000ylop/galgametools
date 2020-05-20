unit Common;

interface

uses
  Windows, SysUtils, Classes, Graphics, Forms, kaguya, BMPScreen;

  procedure Placement;
  function DupName(ImgName:string):string;
  procedure SaveOrView(DirName,DataName:string; a:integer=-1);
  procedure BMPSize(W,H,PF:Integer; ABitmap:TBitmap);
  function Naming(a:Integer; ImgName:string):string;
  procedure WriteOnePix(AFile:TStream; ABitmap:TBitmap); overload;
  procedure WriteOnePix(Input:PByte; ABitmap:TBitmap); overload;
  procedure GetNewPixel(Input,Output:PByteArray;
                        const Count:Integer;
                        var InPos,OutPos:Integer);
  procedure ReadOne(Input:PByteArray;var InPos,Value:Integer);
  procedure BuildPalette(ABMP: TBitmap);

var
SRect:TRect;          //SRect-ScreenRect
PosX,PosY,PosL,PosT,PosR,PosB,SWidth:Integer;
PixSize,SWidthByte,WidthByte,bpp:integer;
DisplayBMP:TBitmap;
BMPMove,BlackScreen:Boolean;
PixData:PByte;

implementation

function DupName(ImgName:string):string;
var
i,j:integer;
begin
  Result:=ImgName;
  i:=2;
  j:=AnsiPos(ExtractFileExt(ExtractFilename(Result)), ExtractFilename(Result));
  if j=0 then j:=Length(Result)+1
  else
  j:=Length(ExtractFilePath(Result))+j;
  while FileExists(Result) do
  begin
    Result:=ImgName;
    if i<10 then
    Insert('_0'+inttostr(i),Result, j)
    else
    Insert('_'+inttostr(i),Result, j);
    inc(i);
  end;
end;

function Naming(a:Integer; ImgName:string):string;
begin
  if a<=0 then    // Only 1 pic
  begin
     Result:=DupName(ImgName);
     Exit;
  end;
  if (a<10) then
       Insert('-00'+InttoStr(a),ImgName,Length(ImgName)-3)  //0-9
  else
  if (a<100) then
       Insert('-0'+InttoStr(a),ImgName,Length(ImgName)-3)  //10-99
  else Insert('-' +InttoStr(a),ImgName,Length(ImgName)-3);  //>=100

  Result:=DupName(ImgName);
end;

procedure SaveOrView(DirName, DataName:string; a:integer=-1);
var
BMPName:string;
begin
  if ShowBMP then Form1.Display
  else
  begin
    GotCG:=True;
    if Form1.PNGSaveBox.Checked then
    BMPname:=DirName+ChangeFileExt(Dataname,'.png')
    else
    BMPname:=DirName+ChangeFileExt(Dataname,'.bmp');
    BMPName:=Naming(a+1,BMPName);
    if Form1.PNGSaveBox.Checked then
    begin
      DisplayPNG.Assign(DisplayBMP);
      DisplayPNG.SaveToFile(BMPName);
    end
    else
    DisplayBMP.SaveToFile(BMPName);
  end;
end;

procedure Placement;
begin
  if (DisplayBMP.Width<SRect.Right-SRect.Left) or
     (DisplayBMP.Height<SRect.Bottom-SRect.Top) then
     BlackScreen:=True else BlackScreen:=False;

     SRect:=Rect(0,0,Screen.Width,Screen.Height);

   if DisplayBMP.Width<=Screen.Width then begin
      SRECT.Left:=(Screen.Width-DisplayBMP.Width) div 2;
      SRECT.Right:=SRECT.Left+DisplayBMP.Width; end;
   if DisplayBMP.Height<=Screen.Height then begin
      SRECT.Top:=(Screen.Height-DisplayBMP.Height) div 2;
      SRECT.Bottom:=SRECT.Top+DisplayBMP.Height; end;

      PosX:=0;  PosY:=0;
      PosL:=0;  PosT:=0;

   if DisplayBMP.Width>Screen.Width then
      PosR:=Screen.Width
   else
      PosR:=DisplayBMP.Width;

   if DisplayBMP.Height>Screen.Height then
      PosB:=Screen.Height
   else
      PosB:=DisplayBMP.Height;

  if (DisplayBMP.Width>Screen.Width) or
     (DisplayBMP.Height>Screen.Height) then
     BMPMove:=True
  else
     BMPMove:=False;
end;

procedure BMPSize(W,H,PF:Integer; ABitmap:TBitmap);
begin
  DisplayBMP.Canvas.Brush.Color:=clWhite;  //Make sure alpah=0, not $FF
  case PF of
  24:begin
       bpp:=3;
       ABitmap.PixelFormat:=pf24bit;
       SWidthByte:=W*3;
       ABitmap.Palette:=0;  {Do not use IgnorePalette:=False}
     end;                      {Wrong Palette $836 after converting 8 bit CG}
  32:begin
       bpp:=4;
       ABitmap.PixelFormat:=pf32bit;
       SWidthByte:=W*4;
       ABitmap.Palette:=0;
     end;
   8:begin
       bpp:=1;
       ABitmap.PixelFormat:=pf8bit;
       SWidthByte:=W;
     end;
  end;
  WidthByte:=(SWidthByte+3) and $FFFFFFFC;
  PixSize:=WidthByte*H;
  ABitmap.Width:=W;
  ABitmap.Height:=H;
end;

procedure WriteOnePix(AFile:TStream; ABitmap:TBitmap); overload;
var
i:integer;
P:PByteArray;
begin
  PixData:=ABitmap.ScanLine[ABitmap.Height-1];
  AFile.ReadBuffer(PixData^, PixSize);

  //School Project
  if XorEnable  and XorTableOK then
  begin
    P:=PByteArray(PixData);
    for i:=0 to PixSize-1 do
    P[i]:=P[i] xor XorTable[i mod XORTableSize];
  end;
end;

procedure WriteOnePix(Input:PByte; ABitmap:TBitmap);
begin
  PixData:=ABitmap.ScanLine[ABitmap.Height-1];
  CopyMemory(PixData, Input, PixSize);
end;

procedure GetNewPixel(Input,Output:PByteArray;
                      const Count:Integer;
                      var InPos,OutPos:Integer);
begin
  CopyMemory(@Output[OutPos],@Input[InPos],Count);
  InPos:=InPos+Count;
  OutPos:=OutPos+Count;
end;

procedure ReadOne(Input:PByteArray;var InPos,Value:Integer);
begin
  Value:=Input[InPos];
  InPos:=InPos+1;
end;

procedure BuildPalette(ABMP: TBitmap);
var
pal: PLogPalette;
i: integer;
begin
  GetMem(pal, 1028);
  pal.palVersion :=$300;
  pal.palNumEntries :=256;
  for i := 0 to 255 do
  begin
    pal.palPalEntry[i].peRed:=i;
    pal.palPalEntry[i].peGreen:=i;
    pal.palPalEntry[i].peBlue:=i;
    pal.palPalEntry[i].peFlags:=0;
  end;
  ABMP.Palette := CreatePalette(pal^);
  FreeMem(pal);
end;

end.
