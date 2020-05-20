unit kaguyaCG;

interface

uses

  Windows, SysUtils, Classes, Graphics, kaguya, BMPScreen,
  Common, StrUtils, Forms;

  procedure GetLZSSImg;
  procedure GetBMPImg;
  procedure GetALPImg;
  procedure GetANMImg;

implementation

var
L,T,L2,T2:integer;   //AO paste Rect

procedure AlphaBlend(DestBMP, SrcBMP:TBitmap;
                     Left, Top, Width, Height:integer);
var
SrcBit, DestBit: PByteArray;
w,h,p,i,ColourBit:integer;
begin
  if DestBMP.PixelFormat=pf32bit then
  ColourBit:=4
  else
  ColourBit:=3;

  for h:=0 to (Height-1) do
  begin      //0
    DestBit:=DestBMP.ScanLine[Top+h];
    SrcBit:=SrcBMP.ScanLine[h];

    p:=3;
    for w:=0 to Width-1 do
    begin     //1
      for i:=0 to 2 do
      if SrcBit[p]=255 then DestBit[(Left+w)*ColourBit+i]:=SrcBit[4*w+i] else
      if SrcBit[p]=0 then break else
      DestBit[(Left+w)*ColourBit+i]:=(DestBit[(Left+w)*ColourBit+i]*not(SrcBit[p])
                                +SrcBit[4*w+i]*SrcBit[p]) div 255 ;
      inc(p,4);
    end;      //1
  end;        //0
end;

procedure GetBMPImg;
var
bmfh:TBitmapFileHeader;
bmih:TBitmapInfoHeader;
begin
  //Do not use BMP.LoadFromStream to avoid last two 00 error
  SFile.Position:=Offset;
  SFile.Read(bmfh, Sizeof(TBitmapFileHeader));
  SFile.Read(bmih, Sizeof(TBitmapInfoHeader));
  BMPSize(bmih.biWidth, bmih.biHeight, bmih.biBitCount, DisplayBMP);
  WriteOnePix(SFile, DisplayBMP);
  if MergeMode then Exit;    //No merge mode for bmp
  SaveorView(Dir, FFilename);
end;

procedure GetALPImg;
var
ALPID: array[0..3] of char;
W,H,L,T,PF:integer;
begin
  SFile.Position:=Offset;
  SFile.ReadBuffer(ALPID, 4);
try
  if ALPID='AP-0' then
  begin
    if MergeMode then Exit;
    SFile.Read(W,4);
    SFile.Read(H,4);
    Assert(W*H>0,'Width or Height is 0');
    BMPSize(W, H, 8, DisplayBMP);
    BuildPalette(DisplayBMP);
    WriteOnePix(SFile, DisplayBMP);
  end
  else
  if  (ALPID='AP-2') or (ALPID='AP-3') then
  begin
    SFile.Read(L,4);
    SFile.Read(T,4);
    SFile.Read(W,4);
    SFile.Read(H,4);
    SFile.Read(PF,4);
    Assert(W*H>0,'Width or Height is 0');
    if MergeMode then
    begin
      if (ALPID='AP-3') then
      BMPSize(W,H,24 ,TempBMP)
      else
      BMPSize(W,H,32 ,TempBMP);

      WriteOnePix(SFile, TempBMP);
      if L+W>DisplayBMP.Width  then DisplayBMP.Width:=L+W;
      if T+H>DisplayBMP.Height then DisplayBMP.Height:=T+H;

      if (ALPID='AP-3') then
         BitBlt(DisplayBMP.Canvas.Handle,L,T,W,H,TempBMP.Canvas.Handle,0,0,SRCCOPY)
      else
      if MergeType=AlphaBlendMode then
         AlphaBlend(DisplayBMP, TempBMP,L,T,W,H)
      else
         BitBlt(DisplayBMP.Canvas.Handle,L,T,W,H,TempBMP.Canvas.Handle,0,0,SRCCOPY);
    end
    else
    begin
      if (ALPID='AP-3') then
        BMPSize(W,H,24 ,DisplayBMP)
      else
        BMPSize(W,H,32,DisplayBMP);
      WriteOnePix(SFile, DisplayBMP);
    end;
  end
  else
  if (ALPID[0]='A') and (ALPID[1]='P') then
  begin
    if MergeMode then Exit;
    SFile.Position:=Offset+2;
    SFile.Read(W,4);
    SFile.Read(H,4);
    SFile.Read(PF,2);
    BMPSize(W, H, 32, DisplayBMP);
    WriteOnePix(SFile, DisplayBMP);
  end
  else
  if (ALPID[0]='B') and (ALPID[1]='M') then GetBMPImg
  else
  DisplayBMP.Assign(nil);

  SaveorView(Dir,FFilename);
except

end;
end;

procedure GetANMImg;

type

TANMRect=packed record
         Left, Top, Width, Height: integer;
         bpp, RectSize: integer;
         end;

var
ID: array[0..3] of char;
j,W,H,L,T,FrameNo:integer;
ANMRect:TANMRect;
ANMBMP:TBitmap;
StartTime:DWord;

begin
  if FileID<>'LINK5' then XorEnable:=False;
  SFile.Position:=Offset;
  SFile.ReadBuffer(ID,4);
try
  if (ID='AN00') or (ID='AN10') or (ID='PL00')
  or (ID='AN20') then
  begin
    FrameNo:=0;
    if ID='AN20' then     //cg056B4, cg71B5, cg71B6
    begin   //AN20
      SFile.Read(j,2);
      SFile.Read(FrameNo,2);
      if FrameNo<>0 then Exit;
      case j of
      $18: SFile.Position:=SFile.Position+174;
      $14: SFile.Position:=SFile.Position+142;
      else
      Exit;
      end;
    end;

    if (ID='PL00') or (ID='AN20') then SFile.Read(FrameNo,2);

    SFile.Read(L,4);
    SFile.Read(T,4);
    SFile.Read(W,4);
    SFile.Read(H,4);
    Assert(W*H>0,'Width or Height is 0');

    if (ID='AN00') or (ID='AN10') then
    begin
    SFile.Read(FrameNo,2);
    SFile.Position:=SFile.Position+FrameNo*4+4;
    end;

    if ID='AN00' then
    begin
      ANMRect.RectSize:=16;
      ANMRect.bpp:=4;
    end
    else  //AN10, AN20, PLT
      ANMRect.RectSize:=20;

    if MergeMode then
    begin
      ANMBMP:=TBitmap.Create;
      TempBMP.Assign(DisplayBMP);   //TempBMP is the BaseBMP
      for j:=0 to FrameNo-1 do
      begin
        SFile.Read(ANMRect, ANMRect.RectSize);
        BMPSize(ANMRect.Width, ANMRect.Height, ANMRect.bpp*8, ANMBMP);
        WriteOnePix(SFile, ANMBMP);
        DisplayBMP.Assign(TempBMP);

        if L+W>DisplayBMP.Width  then DisplayBMP.Width:=L+W;
        if T+H>DisplayBMP.Height then DisplayBMP.Height:=T+H;

        if MergeType=AlphaBlendMode then
        AlphaBlend(DisplayBMP, ANMBMP, L+ANMRect.Left, T+ANMRect.Top,
                   ANMRect.Width, ANMRect.Height)
        else
        BitBlt(DisplayBMP.Canvas.Handle,
               L+ANMRect.Left, T+ANMRect.Top, ANMRect.Width, ANMRect.Height,
               ANMBMP.Canvas.Handle, 0, 0, SRCCOPY);

         SaveorView(Dir, FFilename, j);
         if ShowBMP then
         if Form1.Anime1.Checked then
         begin
           StartTime:=GetTickCount;
           while GetTickCount-StartTime<TimeInterval do Application.ProcessMessages;
         end
         else
         break;
      end;
      ANMBMP.Free;
    end
    else
    if ShowBMP then
    begin
    if (Form1.Anime1.Checked=False)
    or Form1.SlideShow.Enabled
    then
    begin          //Merge all frames into 1 bmp, not valid for large BMP
      if Form1.SlideShow.Enabled then FrameNo:=1;
      BMPSize(W, FrameNo*H, ANMRect.bpp*8, DisplayBMP);
      for j:=0 to FrameNo-1 do
      begin
        SFile.Read(ANMRect,ANMRect.RectSize);
        BMPSize(ANMRect.Width, ANMRect.Height, ANMRect.bpp*8, TempBMP);
        WriteOnePix(SFile, TempBMP);
        BitBlt(DisplayBMP.Canvas.Handle,
               0, j*H, ANMRect.Width, ANMRect.Height,
               TempBMP.Canvas.Handle, 0, 0, SRCCOPY);
      end;
      SaveorView(Dir, FFilename, 0);
    end
    else
      for j:=0 to FrameNo-1 do
      begin
        SFile.Read(ANMRect,ANMRect.RectSize);
        BMPSize(ANMRect.Width, ANMRect.Height, ANMRect.bpp*8, DisplayBMP);
        WriteOnePix(SFile, DisplayBMP);
        SaveorView(Dir, FFilename, j);
        if Form2.Showing then Form1.Image1Click(nil);
        StartTime:=GetTickCount;
        while GetTickCount-StartTime<TimeInterval do Application.ProcessMessages;
      end;
    end
    else
      for j:=0 to FrameNo-1 do
      begin
        SFile.Read(ANMRect,ANMRect.RectSize);
        BMPSize(ANMRect.Width, ANMRect.Height, ANMRect.bpp*8, DisplayBMP);
        WriteOnePix(SFile, DisplayBMP);
        SaveorView(Dir, FFilename, j);
     end;
  end;
except

end;
end;

procedure CMP2Data(Input:PByte; Output: PByteArray; InSize, PixEnd:Longword);
const
   SWSize=$1000;
   SWStartPos=1;
   SWEndPos=$FFF;
   Threshold=2;
   Length_Bits=4;

var
i, data, Bits, Code,  dataPos, count, offset, OutPos: Longword;
datalog: array [0..SWEndPos] of byte;

procedure WritePix(var aPos:Longword);
begin
  if (PixEnd < SWSize) and (aPos=PixEnd) then
  begin
    CopyMemory(@Output[OutPos], @datalog[SWStartPos],PixEnd);
    PixEnd:=0;
  end
  else
  if aPos+1=SWStartPos then
  begin
    CopyMemory(@Output[OutPos], @datalog[SWStartPos],SWSize-SWStartPos);
    inc(OutPos, SWSize-SWStartPos);
    CopyMemory(@Output[OutPos], @datalog[0], SWStartPos);
    inc(OutPos, SWStartPos);
    Dec(PixEnd,SWSize);
  end;

  aPos:=(aPos+1) and SWEndPos;
end;

  procedure ReadCode(CodeSize:Longword);
  var
  CodeMask:Longword;
  begin
    CodeMask:=(1 shl CodeSize) - 1;
    while (Bits < CodeSize) and (InSize > 0) do
    begin
      inc(Data, Input^ shl (24 - Bits));
      inc(Bits, 8);
      inc(Input);
      dec(InSize);
    end;

    Code := (Data and ($FFFFFFFF - CodeMask)) shr (32 - CodeSize);
    Data := Data shl CodeSize;
    dec(Bits, CodeSize);
  end;

begin
  dataPos:=SWStartPos;
  OutPos:=0;
  Bits := 0;
  Data:=0;
  while (InSize>0) and (PixEnd>0) do
  begin
    ReadCode(9);
    if Code>255 then
    begin
      datalog[dataPos]:=Code and $FF;
      WritePix(dataPos);
    end
    else
    begin
      count:=0;
      offset:=Code;
      ReadCode(8);
      offset:=(Code shr Length_Bits) or (offset shl 4);
      count:=Code mod 16 + Threshold;
      for i:=1 to count do
      begin
        datalog[dataPos]:=datalog[offset];
        offset:=(offset+1) and SWEndPos;
        WritePix(dataPos);
      end;
    end;
  end;
end;

//人形の館
procedure IAF2DataB(Input,Output:PByteArray;
                    const EndPos, PixEnd:Longword);
const
   SWStartPos=$EF;
   SWEndPos=$FF;
   Threshold=2;
var
datalog:array [0..SWEndPos] of Byte;
i,j:Byte;
n,x,y,offset,count,dataPos:Longword;
ReadNext:Boolean;
begin
  x:=0;
  y:=0;
  dataPos:=SWStartPos;
  FillChar(datalog,Sizeof(datalog),0);
  count:=Threshold;
  ReadNext:=True;
  try
    while (x<EndPos) and (y<PixEnd) do
    begin  //1
      i:= Input[x];
      inc(x);
      j:=128;

      while (j>0) and (y<PixEnd) do
      begin  //2
        if i and j > 0 then
        begin
          Output[y]:=Input[x];
          datalog[dataPos]:=Input[x];
          inc(x);
          inc(y);
          dataPos:=(dataPos+1) and SWEndPos;
        end
        else
        begin
          offset:=Input[x];
          inc(x);

          if ReadNext then
          count:=Threshold+(Input[x] mod $10);

          for n:=1 to count do
          begin
            Output[y]:=datalog[offset];
            datalog[dataPos]:=datalog[offset];
            offset:=(offset+1) and SWEndPos;
            dataPos:=(datapos+1) and SWEndPos;
            inc(y);
          end;

          if ReadNext then
          begin
            count:=Threshold+Input[x] div $10;
            inc(x);
          end;
          ReadNext:=not ReadNext;
        end;

        j:=j div 2;
      end;  //2
    end;   //1
    except
    end;
end;

procedure GetLZSSImg;
var
ID:array [0..1] of char;
ImgSize,W,H,PF,i,k:integer;
SrcBuf:Pointer;
ImgBuf:PByteArray;
n:Word;
NameLength:Byte;

   procedure SelectID;
   var
   bmfh:PBitmapFileHeader;
   bmih:PBitmapInfoHeader;
   begin
     ID[0]:=Char(ImgBuf[0]);
     ID[1]:=Char(ImgBuf[1]);
     if ID='BM' then
     begin
       bmfh:=PBitmapFileHeader(THandle(ImgBuf));
       bmih:=PBitmapInfoHeader(THandle(ImgBuf)+Sizeof(TBitmapFileHeader));
       BMPSize(bmih.biWidth, bmih.biHeight, bmih.biBitCount, DisplayBMP);
       WriteOnePix(Pointer(THandle(ImgBuf)+bmfh.bfOffBits), DisplayBMP);
     end
     else
     if (ID='AP') then
     begin
       L2:=0;
       L:=0;
       T2:=0;
       T:=0;
       W:=PInt(THandle(ImgBuf)+2)^;
       H:=PInt(THandle(ImgBuf)+6)^;
       Assert(W*H>0,'Width or Height is 0');
       PF:=PWord(THandle(ImgBuf)+10)^;
       BMPSize(W,H,32,DisplayBMP);
       WriteOnePix(Pointer(THandle(ImgBuf)+12),DisplayBMP);
     end
     else
     if (ID='AO') then
     begin
       L2:=L;
       T2:=T;
       W:=PInt(THandle(ImgBuf)+2)^;
       H:=PInt(THandle(ImgBuf)+6)^;
       Assert(W*H>0,'Width or Height is 0');
       PF:=PWord(THandle(ImgBuf)+10)^;
       L:=PInt(THandle(ImgBuf)+12)^;
       T:=PInt(THandle(ImgBuf)+16)^;
       BMPSize(W,H,32,DisplayBMP);
       WriteOnePix(Pointer(THandle(ImgBuf)+20),DisplayBMP);
     end
     else
     begin
       W:=PInt(ImgBuf)^;
       H:=PInt(THandle(ImgBuf)+4)^;
       Assert(W*H>0,'Width or Height is 0');
       BMPSize(W, H, 8, DisplayBMP);
       BuildPalette(DisplayBMP);
       WriteOnePix(Pointer(THandle(ImgBuf)+10),DisplayBMP);
     end;
   end;

begin
   case Comp_Type of
   1:begin
      //人形の館
     if LIN2File then
     begin
       SFile.Position:=Offset;
       SFile.Read(ImgSize, 4);
       GetMem(SrcBuf, FileSize-4);
       ImgBuf:=AllocMem(ImgSize);
       SFile.Read(SrcBuf^, FileSize-4);
       IAF2DataB(SrcBuf, ImgBuf, FileSize-4, ImgSize);
       SelectID;
       FreeMem(ImgBuf);
       FreeMem(SrcBuf);
     end
     else
     begin
        SFile.Position:=Offset-4;
        SFile.Read(ImgSize, 4);
        GetMem(SrcBuf, FileSize);
        ImgBuf:=AllocMem(ImgSize);
        SFile.Read(SrcBuf^, FileSize);
        CMP2Data(SrcBuf, ImgBuf, FileSize, ImgSize);
        SelectID;
        FreeMem(ImgBuf);
        FreeMem(SrcBuf);
     end;
     end;

   0: begin
        SFile.Position:=Offset;
        SFile.Read(ID, 2);
        //ダンジョンクルセイダーズ *.parts
        if AnsiContainsText(FFileName,'.parts') then
        begin
          if ID='AP' then GetALPImg
          else
          begin
          if ID=Chr(4)+'A' then
          begin
            SFile.Position:=Offset+1;
            SFile.Read(k,4);
            if k<>$33535041 then Exit;  //'APS3'
            SFile.Read(k,4);
            SFile.Position:=SFile.Position+4;
            for i:= 1 to k do
            begin
              SFile.Read(NameLength, 1);
              SFile.Position:=SFile.Position+32+NameLength;
            end;
          end
          else
          begin
          for i:=1 to Word(ID) do
          begin
            SFile.Read(k,4);
            SFile.Position:=SFile.Position+k;
          end;
          SFile.Read(n,2);
          //system_backsel.parts
          for i:=1 to n do
          begin
            SFile.Read(k,4);    //NameSize
            SFile.Position:=SFile.Position+k+2;
            SFile.Read(k,2);
            SFile.Position:=SFile.Position+k*$20;
          end;
          end;


          SFile.Read(n,2);
          if n<>1 then Exit;
          SFile.Read(FileSize, 4);
          SFile.Read(ImgSize, 4);
          GetMem(SrcBuf, FileSize);
          ImgBuf:=AllocMem(ImgSize);
          SFile.Read(SrcBuf^, FileSize);
          CMP2Data(SrcBuf, ImgBuf, FileSize, ImgSize);
          SelectID;
          FreeMem(ImgBuf);
          FreeMem(SrcBuf);
          SaveorView(Dir, FFilename);
          end;
        end
        else
        if AnsiContainsText(FFileName,'.dan') then Exit
        else
        //non compressed .bg_ bmp
        //Serina BG_.ARC s_ser170.bg_ & s_ser100.bg_
        //DC 城謁見の間.bg_
        if ID<>'AP' then GetBMPImg else GetALPImg;

        ID:='';    //To skip following merge function
                   //& avoid double file save due to GetBMPImg & GetALPImg
      end;
   end;

   if MergeMode and (not BufBMP.Empty)
   and ((ID='AP') or (ID='AO')) then
   begin
     AlphaBlend(BufBMP, DisplayBMP,abs(L-L2),abs(T-T2),W,H);
     DisplayBMP.Assign(BufBMP);
   end;
   if not DisplayBMP.Empty then BufBMP.Assign(DisplayBMP);
   if ID<>'' then SaveorView(Dir, FFilename);
end;

end.
