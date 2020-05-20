unit kaguya;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  CommCtrl, ComCtrls, StdCtrls, DirDialog, ExtCtrls, Menus, Dynamic_Bass,
  Buttons, SyncObjs, PNG;

type

  TForm1 = class(TForm)
    ListView: TListView;
    DirDialog1: TDirDialog;
    Panel1: TPanel;
    Panel2: TPanel;
    Panel3: TPanel;
    Panel4: TPanel;
    Image1: TImage;
    BrowseBtn: TButton;
    BMPBtn: TButton;
    OutputFolder: TButton;
    CutBtn: TButton;
    OpenDialog1: TOpenDialog;
    Splitter1: TSplitter;
    ProgressBar: TProgressBar;
    Edit: TEdit;
    Label1: TLabel;
    PopupMenu1: TPopupMenu;
    PopupMenu2: TPopupMenu;
    Merge1: TMenuItem;
    AlphaBlendBtn: TMenuItem;
    Save1: TMenuItem;
    Face1: TMenuItem;
    CharFace1: TMenuItem;
    CharFaceAuto1: TMenuItem;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    PlayOgg: TMenuItem;
    StopOgg: TMenuItem;
    TrackBar1: TTrackBar;
    SlideShow: TTimer;
    BtnUp: TSpeedButton;
    BtnDown: TSpeedButton;
    SortPhrase1: TMenuItem;
    N4: TMenuItem;
    AddExtList1: TMenuItem;
    LoadXORTable1: TMenuItem;
    OpenDialog2: TOpenDialog;
    PNGSaveBox: TCheckBox;
    Anime1: TMenuItem;
    N5: TMenuItem;
    AlphaBlendAutoSave1: TMenuItem;
    SaveDialog1: TSaveDialog;

    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);

    procedure ListViewCustomDrawItem(Sender: TCustomListView;
      Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
    procedure ListViewData(Sender: TObject; Item: TListItem);
    procedure ListViewKeyUp(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure ListViewMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure ListViewMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure ListViewClick(Sender: TObject);
    procedure ListViewDblClick(Sender: TObject);

    procedure OutputFolderClick(Sender: TObject);
    procedure BrowseBtnClick(Sender: TObject);
    procedure BMPBtnClick(Sender: TObject);
    procedure CutBtnClick(Sender: TObject);

    procedure Image1Click(Sender: TObject);

    procedure Merge1Click(Sender: TObject);
    procedure Face1Click(Sender: TObject);
    procedure CharFace1Click(Sender: TObject);
    procedure CharFaceAuto1Click(Sender: TObject);
    procedure AlphaBlendBtnClick(Sender: TObject);

    procedure TrackBar1Change(Sender: TObject);
    procedure SlideShowTimer(Sender: TObject);
    procedure PlayOggClick(Sender: TObject);
    procedure StopOggClick(Sender: TObject);

    procedure Save1Click(Sender: TObject);
    procedure BtnUpClick(Sender: TObject);
    procedure BtnDownClick(Sender: TObject);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure SortPhrase1Click(Sender: TObject);
    procedure AddExtList1Click(Sender: TObject);
    procedure LoadXORTable1Click(Sender: TObject);
    procedure Anime1Click(Sender: TObject);
    procedure Splitter1Moved(Sender: TObject);
    procedure AlphaBlendAutoSave1Click(Sender: TObject);
    procedure SaveDialog1TypeChange(Sender: TObject);
  private
    CGList:TList;
    OggStream:HStream;
    procedure ClearCGList;
    function SetupSavePath:Boolean;
    procedure TGetListThreadTerminate(Sender: TObject);
    procedure TSaveCGThreadTerminate(Sender: TObject);
  public
    procedure ComparePhrase;
    procedure CompareSuffix;
    procedure Display;
    procedure CopyCutFile(SFile, DFile:TFileStream; buf:int64);
    procedure SaveList;
  end;

  TGetListThread = class(TThread)
  private
    Filename: TFileName;
    Offset: int64;
    CheckedSize: integer;
    Err, CGFile: Boolean;
  protected
     procedure Execute; override;
     procedure GetLINKFile(StartOffset:integer);
     procedure GetFile;
     procedure ShowProgress;
     procedure SetProgressMax;
  end;

  TSaveCGThread = class(TThread)
  private
    Filename: TFileName;
    Index: integer;
  protected
     procedure Execute; override;
     procedure ShowProgress;
  end;


    procedure GetCG(Index:integer);
    procedure GetFileInfo(Index:integer);

const
MaxNameSize = 63;
ATitle='KaguyaCut v1.13';

type

   PFileInfo = ^TFileInfo;
   TFileInfo = packed record
	       FileName: array[0..MaxNameSize] of char;
	       Offset,
	       FileSize,
               FullSize,
               Comp_Type:integer;
	       end;

   PkaARCFile=^TkaARCFile;          //ARC
   TkaARCFile=packed record
              FileSize: integer;
              Comp_Type: Byte;      //0:NonCompressed, 1:Compressed
              Unknown:Byte;
              FileDate: array[0..6] of Byte;
              NameSize:Word;
              EndFlag:Byte;
              end;

  TMergeMode= (BitBltMode,AlphaBlendMode);


var
  Form1: TForm1;
  TimeInterval:Dword;
  TempBMP, BufBMP:TBitmap;
  XorTable:PBytearray;
  XorEnable, XorTableOK, LIN2File:Boolean;
  GetList:TGetListThread;
  SaveCG:TSaveCGThread;
  FileID:array[0..4] of char;
  SFile: TFileStream;
  Dir,
  DBMPName,
  CGFilename,
  FFilename,
  PhraseText,
  XorPath, ID: string;
  Offset,
  FileSize,
  Comp_Type,
  LVIndex, FacePatternIndex:Integer;
  ShowBMP, GotCG, MergeMode, Converting: Boolean;
  MergeType: TMergeMode;
  FileInfo:PFileInfo;
  ExtList:TStringList;
  XORTableSize:integer;
  DisplayPNG:TPNGImage;
  DoList: array of Integer;


implementation

{$R *.dfm}

uses StrUtils, Common, BMPScreen, kaguyaCG, Phrase;

procedure TForm1.FormCreate(Sender: TObject);
begin
  WindowState:=wsMaximized;
  CGList:=TList.Create;
  DisplayBMP:=TBitmap.Create;
  TempBMP:=TBitmap.Create;
  BufBMP:=TBitmap.Create;
  Label1.Caption:='0';
  FacePatternIndex:=-1;
  case GetUserDefaultLangID of
   $411: Edit.Font.Name:='MS UI Gothic';
   $404: Edit.Font.Name:='MingLiU';
   else  Edit.Font.Name:='Times New Roman';
  end;
  Load_BASSDLL('');
    // Initialize audio - default device, 44100hz, stereo, 16 bits
  if BASS_Handle<>0 then BASS_Init(-1, 44100, 0, Handle, nil);
  XORTable:=nil;
  XorPath:=GetCurrentDir;
  Application.HintHidePause:=1000;
  TimeInterval:=50;
  GetList:=nil;
  ExtList:=TStringList.Create;
  ExtList.Add('.cg_');
  ExtList.Add('.cgw');
  ExtList.Add('.sp_');
  ExtList.Add('.bg_');
  ExtList.Add('.parts');
  ExtList.Add('.dan');
  XORTableSize:=0;
  DisplayPNG:=TPNGImage.Create;
  Caption:=ATitle;
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  ClearCGList;
  CGList.Free;
  DisplayBMP.Free;
  TempBMP.Free;
  BufBMP.Free;
  Unload_BASSDLL;
  if XorTable<>nil then FreeMem(XorTable);
  if GetList<>nil then GetList.Terminate;
  ExtList.Free;
  DisplayPNG.Free;
end;

function TForm1.SetupSavePath: Boolean;
begin
  Result:=False;
  if Edit.Text='' then OutputFolderClick(nil);
  if Edit.Text='' then Exit;
  Dir:=IncludeTrailingPathDelimiter(Edit.Text);
  if not DirectoryExists(Dir) then ForceDirectories(Dir);
  if not DirectoryExists(Dir) then
  ShowMessage('Invalid Output Folder')
  else
  Result:=True;
end;

procedure TForm1.ClearCGList;
var
n:integer;
begin
  for n:=0 to CGList.Count-1 do
  Dispose(PFileInfo(CGList[n]));
  CGList.Clear;
  Label1.Caption:='0';
end;

//---------------------------------------------------------------------
//GetListThread
//---------------------------------------------------------------------
function CompareValue(const A, B: Integer): Integer;
begin
  if A = B then
    Result := 0
  else if A < B then
    Result := -1
  else
    Result := 1;
end;

function CompareNames(Item1, Item2: Pointer): integer;
begin
  Result := AnsiCompareText(PFileInfo(Item1).Filename, PFileInfo(Item2).Filename);
  if Result=0 then
  Result:=CompareValue(PFileInfo(Item1).Offset, PFileInfo(Item2).Offset);
end;

function CompareExt(Item1, Item2: Pointer): integer;
begin
  Result := AnsiCompareText(ExtractFileExt(PFileInfo(Item1).FileName),
                            ExtractFileExt(PFileInfo(Item2).FileName));
  if Result=0 then
  Result:=CompareNames(Item1, Item2);
end;

function CompareSize(Item1, Item2: Pointer): integer;
begin
  Result:=CompareValue(PFileInfo(Item1).FileSize, PFileInfo(Item2).FileSize);
  if Result=0 then Result:=CompareNames(Item1, Item2);
end;

procedure TForm1.CompareSuffix;
var
i,j:integer;
begin
  if CGList.Count<=0 then Exit;
  j:=0;
  for i:=CGList.Count-1 downto 0 do
  if AnsiContainsText(PFileInfo(CGList.Items[i]).Filename, 'z.') then
  begin
    inc(j);
    CGList.Move(i, CGList.Count-j);
  end;
end;

procedure TGetListThread.ShowProgress;
begin
  Form1.ProgressBar.Position:=CheckedSize;
end;

procedure TGetListThread.SetProgressMax;
begin
  Form1.ProgressBar.Max:=100;
end;

procedure TGetListThread.GetFile;
var
n:integer;
begin
  New(FileInfo);
  FillChar(FileInfo^,80,$0);

  SFile.Read(FileInfo.FileSize,4);     //FileSize-->NameSize
  SFile.Read(FileInfo.FileName,FileInfo.FileSize);
  for n:=0 to FileInfo.FileSize-1 do
  FileInfo.FileName[n]:= Char(not Byte(FileInfo.FileName[n]));

  SFile.Read(FileInfo.Comp_Type,2);
  Offset:=Offset+FileInfo.FileSize+6;

  //CG
  if CGFile then
  begin
    if FileInfo.Comp_Type>0 then
       Offset:=Offset+8
    else
       Offset:=Offset+4;
 end
 else  //BGM, Comp_Type=2
       Offset:=Offset+4;

  FileInfo.Offset:=Offset;
  SFile.Read(FileInfo.FileSize,4);
  Offset:=Offset+FileInfo.FileSize;
  Form1.CGList.Add(FileInfo);
  CheckedSize:=SFile.Position  * 100 div SFile.Size;
  Synchronize(ShowProgress);
end;

procedure TGetListThread.GetLINKFile(StartOffset:integer);
var
KaFile:PkaARCFile;
begin
  New(KaFile);
  SFile.Position:=StartOffset;
  Offset:=StartOffset;
  while not Terminated and (Offset+4<SFile.Size) do
  begin
    New(FileInfo);
    FillChar(FileInfo^,80,$0);
    SFile.ReadBuffer(KaFile^,16);
    SFile.Read(FileInfo.FileName,KaFile.NameSize);

    FileInfo.Offset:=Offset+16+KaFile.NameSize;
    FileInfo.FileSize:=KaFile.FileSize-16-KaFile.NameSize;
    FileInfo.Comp_Type:=KaFile.Comp_Type;

    Form1.CGList.Add(FileInfo);
    SFile.Position:=Offset+KaFile.FileSize;
    Offset:=SFile.Position;
    CheckedSize:=Offset * 100 div SFile.Size;
    Synchronize(ShowProgress);
  end;
  CGFilename:=FileName;
  Dispose(KaFile);
end;

procedure TGetListThread.Execute;
var
Fileno,i,j:integer;
SaveCursor: TCursor;
ARIFilename:string;
begin
   SaveCursor := Screen.Cursor;
   Err:=True;
   LIN2File:=False;
   // for *.ari selection
   if AnsiContainsText(Filename, 'bgm')
   or AnsiContainsText(Filename, 'wav')
   or AnsiContainsText(Filename, 'vo') then CGFile:=False
   else CGFile:=True;

   SFile:=TFileStream.Create(FileName,fmOpenRead or fmShareDenyWrite);

   try
   if SFile<>nil then
   begin//1
     CheckedSize:=0;
     Synchronize(SetProgressMax);
     Synchronize(ShowProgress);
     SFile.Read(FileID,5);
     if FileID='LINK3' then GetLinkFile(8)
     else
     if (FileID='LINK4') or (FileID='LINK5') then GetLinkFile(10)
     else
     if AnsiContainsText(FileID,'LIN2') then
     begin
       LIN2File:=True;
       SFile.Position:=4;
       SFile.Read(Fileno,4);
       Offset:=8;
       i:=0;
       while not Terminated and (i<Fileno) do
       begin
         New(FileInfo);
         FillChar(FileInfo^,80,$0);
         SFile.Read(FileInfo.FileSize,2);     //FileSize-->NameSize
         SFile.Read(FileInfo.FileName,FileInfo.FileSize);
         for j:=0 to FileInfo.FileSize-1 do
         FileInfo.FileName[j]:= Char(not Byte(FileInfo.FileName[j]));
         SFile.Read(FileInfo.Offset,4);
         SFile.Read(FileInfo.FileSize,4);
         SFile.Read(FileInfo.Comp_Type,2);
         Form1.CGList.Add(FileInfo);
         inc(i);
       end;
       CGFilename:=FileName;
     end
     else     // HeartBeat CG Arc file
     if AnsiContainsText(FileID,'WFL1') then
     begin
       Offset:=4;
       ARIFilename:=ChangeFileExt(Filename,'.ari');
       if FileExists(ARIFilename) then
       begin
         SFile.Free;
         SFile:=TFileStream.Create(ARIFileName,fmOpenRead or fmShareDenyWrite);
         while not Terminated and (SFile.Position<SFile.Size) do
            GetFile;
       end
       else
       while not Terminated and (Offset+4<SFile.Size) do
       begin
         SFile.Position:=Offset;
         GetFile;
       end;
       CGFilename:=FileName;
     end
     else   // ARI File
     if AnsiContainsText(ExtractFileExt(Filename),'.ari') then
     begin
       SFile.Position:=0;
       Offset:=4;
       while not Terminated and (SFile.Position<SFile.Size) do
          GetFile;
       CGFilename:=ChangeFileExt(Filename,'.arc');
       if not FileExists(CGFilename) then
       ShowMessage(CGFilename +' does not exist!');
     end
     else
     Exit;       //Go to finally
   end;//1

   if not Terminated then
   begin
     Err:=False;
     SendMessage(Form1.ListView.Handle, LVM_SETITEMCOUNT, Form1.CGList.Count, 0);
     Terminate;
   end;

   finally
   Screen.Cursor := SaveCursor;
   SFile.Free;
   ID:=FileID;
   if Err then
   begin
     Form1.ClearCGList;
     Form1.Caption:=ATitle;
     ID:='';
     if Terminated then ShowMessage('A User Break')
     else
     begin
       Terminate;
       ShowMessage('Non Support File!');
     end;
   end;
   end;
end;

procedure TForm1.TGetListThreadTerminate(Sender: TObject);
begin
  ListView.Repaint;
  ListView.SetFocus;
  ProgressBar.Position:=0;
  GetList:=nil;

  if ListView.Items.Count>0 then
  begin
    Converting:=False;
    ListView.Items.Item[0].Selected:=True;
    ListView.Items.Item[0].Focused:=True;
    ListViewClick(Sender);
    PopupMenu1.AutoPopup:=True;
    PopupMenu2.AutoPopup:=True;
    Form1.Caption:=ATitle+'-'+CGFileName+'x'+inttostr(CGList.Count);
  end;
  Form1.Repaint;
end;

procedure TForm1.BrowseBtnClick(Sender: TObject);
begin
  if GetList<>nil then Exit;

  if OpenDialog1.Execute then
  begin
    StopOggClick(Self);
    if ListView.Items.Count>0 then
    begin
      ClearCGList;
      ListView.Items.Count:=0;
      ListView.Repaint;
      PopupMenu1.AutoPopup:=False;
      PopupMenu2.AutoPopup:=False;
    end;
    FacePatternIndex:=-1;
    Image1.Picture.Assign(nil);
    DisplayBMP.Assign(nil);
    TempBMP.Assign(nil);
    BufBMP.Assign(nil);
    Form1.Caption:=ATitle;

    if AnsiContainsText(Opendialog1.Filename,'tblstr')
    or AnsiContainsText(Opendialog1.Filename,'tblname')then
    begin
      ShowMessage('Non Support File!');
      Exit;
    end;

    GetList:=TGetListThread.Create(True);
    GetList.FreeOnTerminate:=True;
    GetList.OnTerminate:=TGetListThreadTerminate;
    GetList.Filename:=Opendialog1.FileName;
    GetList.Resume;
  end;
end;

//---------------------------------------------------------------------
//Draw ListView
//---------------------------------------------------------------------

procedure TForm1.ListViewCustomDrawItem(Sender: TCustomListView;
  Item: TListItem; State: TCustomDrawState; var DefaultDraw: Boolean);
begin
  if Item = nil then Exit;
  ListView.Canvas.Font.Color := clBlack;
end;

procedure TForm1.ListViewData(Sender: TObject; Item: TListItem);
begin
  if (Item.Index > CGList.Count) then Exit;
  with PFileInfo(CGList[Item.Index])^ do
  begin
    Item.Caption := Filename;
    Item.SubItems.Add(Inttohex(Offset,8));
    Item.SubItems.Add(InttoStr(FileSize));
  end;
end;

procedure TForm1.ListViewKeyUp(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  ListViewClick(Sender);
end;

procedure TForm1.ListViewMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  Label1.Caption:=inttostr(ListView.SelCount);
end;

procedure TForm1.ListViewMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
  Label1.Caption:=inttostr(ListView.SelCount);
end;

procedure TForm1.ListViewClick(Sender: TObject);
begin
  Label1.Caption:=inttostr(ListView.SelCount);
  if (ListView.Selected<>nil) and (ListView.ItemFocused<>nil) then
     GetCG(ListView.ItemFocused.Index);
end;

procedure TForm1.ListViewDblClick(Sender: TObject);
begin

end;

//---------------------------------------------------------------------
//Image Process
//---------------------------------------------------------------------

procedure SelectCG;
var
i:integer;
begin
  Converting:=True;
  XorEnable:=False;
  GotCG:=False;
  if Comp_Type=4 then XorEnable:=True;


  if AnsiContainsText(FFileName,'.bmp')
  and (Comp_Type in [0,4]) then GetBMPImg
  else
  if AnsiContainsText(FFileName,'.alp') then GetALPImg
  else
  if AnsiContainsText(FFileName,'.anm')
  or AnsiContainsText(FFileName,'.plt') then GetANMImg
  else
  for i:=0 to ExtList.Count-1 do
  if AnsiContainsText(FFileName, ExtList.Strings[i]) then
  begin
    GetLZSSImg;
    Break;
  end;

  if GotCG=False then
  begin
    if Form2.Showing then Form2.Hide;
    Form1.Image1.Picture.Assign(nil);
  end;
  Converting:=False;
end;

procedure GetFileInfo(Index: integer);
begin
  FFilename:=PFileInfo(Form1.CGList[Index]).FileName;
  Offset:=PFileInfo(Form1.CGList[Index]).Offset;
  FileSize:=PFileInfo(Form1.CGList[Index]).FileSize;
  Comp_Type:=PFileInfo(Form1.CGList[Index]).Comp_Type;
end;

procedure OpenFile;
begin
  if FileExists(CGFilename) then
  try
     SFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
     SelectCG;
  finally
     SFile.Free;
  end;
end;

procedure GetCG(Index:integer);
begin
  if Converting then Exit;
  DisplayBMP.Assign(nil);
  LVIndex:=Index;

  if FileExists(CGFilename) then
  begin
    Dir:='';
    ShowBMP:=True;
    MergeMode:=False;
    MergeType:=BitBltMode;
    GetFileInfo(Index);
    DBMPName:=ExtractFilename(FFileName);
    OpenFile;
  end
  else
  Form1.Image1.Picture.Assign(nil);
end;

procedure TForm1.Image1Click(Sender: TObject);
begin
  if not Image1.Picture.Bitmap.Empty then
  begin
    if not Form2.Showing then Form2.Show;
    Placement;
    if BlackScreen then Form2.Canvas.FillRect(Form2.ClientRect);
    Form2.FormPaint(Self);
  end
  else
  if Form2.Showing then Form2.Hide;
end;

procedure TForm1.Save1Click(Sender: TObject);
begin
  Form2.Save1Click(Self);
end;

procedure TForm1.AlphaBlendBtnClick(Sender: TObject);
var
i:integer;
begin
  if Converting then Exit;
  if FileExists(CGFilename) then
  begin
    SFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
  try
   if not BufBMP.Empty then DisplayBMP.Assign(BufBMP)
   else BMPSize(800,600,32,DisplayBMP);

   Dir:='';
   ShowBMP:=True;
   MergeMode:=True;
   MergeType:=AlphaBlendMode;
   if (Sender=Form1.AlphaBlendAutoSave1)
   and SetupSavePath then
   ShowBMP:=False;

   SaveList;
   for i:=0 to Length(DoList)-1 do
   begin
      GetFileInfo(DoList[i]);
      SelectCG;
      Form1.Repaint;
   end;

  finally
    ListView.SetFocus;
    SFile.Free;
  end;
 end;
end;

procedure MergeSelect;
var
i:integer;
begin
  if FileExists(CGFilename) then
  begin
    SFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
  try
    Dir:='';
    ShowBMP:=True;
    MergeMode:=True;
    MergeType:=BitBltMode;

    Form1.SaveList;
    GetFileInfo(DoList[0]);
    if AnsiContainsText(FFileName,'.bmp')
    and (Comp_Type in [0,4]) then GetBMPImg
    else
    begin
      DisplayBMP.Assign(nil);
      BMPSize(800,600,32,DisplayBMP);
      SelectCG;
    end;

    Form1.Repaint;

   for i:=1 to Length(DoList)-1 do
   begin
      GetFileInfo(DoList[i]);
      SelectCG;
      Form1.Repaint;
   end;

  finally
    Form1.ListView.SetFocus;
    SFile.Free;
  end;
  end;
end;

procedure TForm1.Merge1Click(Sender: TObject);
begin
  if Converting then Exit;
  MergeSelect;
  if not DisplayBMP.Empty then BufBMP.Assign(DisplayBMP);
end;

procedure TForm1.Face1Click(Sender: TObject);
begin
  FacePatternIndex:=ListView.Selected.Index;    //store the face pattern
end;

procedure TForm1.CharFace1Click(Sender: TObject);
var
Item: TListItem;
begin
  if Converting or
   (Listview.SelCount=0) or (FacePatternIndex=-1) then Exit;
  Item := ListView.Selected;
  GetFileInfo(Item.Index);
  if AnsiContainsText(FFileName,'.bmp')
  or AnsiContainsText(FFileName,'.anm')
  or AnsiContainsText(FFileName,'.alp') then
     Merge1Click(Self);

  // To add the face expression

  if FileExists(CGFilename) then
  begin
    SFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
  try
    Dir:='';
    ShowBMP:=True;
    MergeMode:=True;
    MergeType:=AlphaBlendMode;
    GetFileInfo(FacePatternIndex);
    SelectCG;
    Form1.Repaint;
  finally
    ListView.SetFocus;
    SFile.Free;
  end;
  end;

  if not DisplayBMP.Empty then BufBMP.Assign(DisplayBMP);
end;

procedure TForm1.CharFaceAuto1Click(Sender: TObject);
var
i:integer;
begin
  if Converting
  or (Listview.SelCount=0) or (FacePatternIndex=-1) then Exit;
  if FileExists(CGFilename) and SetupSavePath then
  begin
    SFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
  try
    MergeMode:=True;
    SaveList;

  for i:=0 to Length(DoList)-1 do
  begin
    //DisplayBMP is still exist, so all anm frames will be saved
    BufBMP.Assign(nil);  //To clear previous image
    ShowBMP:=True;       //No Save
    MergeType:=BitBltMode;
    GetFileInfo(DoList[i]);
    if AnsiContainsText(FFileName,'.bmp')
    and (Comp_Type in [0,4]) then GetBMPImg
    else
    begin
      DisplayBMP.Assign(nil);
      BMPSize(800,600,32,DisplayBMP);
      SelectCG;
    end;
    //To add the face pattern
    ShowBMP:=False;  //Save
    MergeType:=AlphaBlendMode;
    GetFileInfo(FacePatternIndex);
    SelectCG;
  end;
  finally
    ListView.SetFocus;
    SFile.Free;
    ShowBMP:=True;
    Display;
  end;
  end;
end;

procedure TSaveCGThread.ShowProgress;
begin
  Form1.ProgressBar.Position:=Form1.ProgressBar.Position + 1;
end;

procedure TSaveCGThread.Execute;
begin
  SFile:=TFileStream.Create(Filename,fmOpenRead);
  Index:=0;
  try
  while not Terminated and (Index<Length(DoList)) do
  begin
    GetFileInfo(DoList[Index]);
    SelectCG;
    inc(Index);
    Synchronize(ShowProgress);
  end;
  finally
  SFile.Free;
  end;
  if not Terminated then
  begin
    Terminate;
    ShowMessage(' Complete');
  end
  else
  ShowMessage('A User Break'); 
end;

procedure TForm1.TSaveCGThreadTerminate(Sender: TObject);
begin
  Progressbar.Position:=0;
  ListView.SetFocus;
  PopupMenu1.AutoPopup:=True;
  SaveCG:=nil;
end;

procedure TForm1.BMPBtnClick(Sender: TObject);
begin
  if  (SaveCG=nil)
  and (GetList=nil)
  and (ListView.SelCount>0)
  and FileExists(CGFilename)
  and SetupSavePath then
  begin
    SaveList;
    ShowBMP:=False;
    MergeMode:=False;
    MergeType:=BitBltMode;
    PopupMenu1.AutoPopup:=False;
    Progressbar.Max:=ListView.SelCount;
    SaveCG:=TSaveCGThread.Create(True);
    SaveCG.FreeOnTerminate:=True;
    SaveCG.OnTerminate:=TSaveCGThreadTerminate;
    SaveCG.Filename:=CGFilename;
    SaveCG.Resume;
   end;
end;

//---------------------------------------------------------------------
//File Process
//---------------------------------------------------------------------

procedure TForm1.CopyCutFile(SFile, DFile:TFileStream; buf:int64);
begin
  ProgressBar.Max:= ProgressBar.Max+buf div $100000+1;
  try
  while buf>$100000 do
  begin
    DFile.CopyFrom(SFile,$100000);
    buf:=buf-$100000;
    ProgressBar.StepIt;
  end;
   // buf=0 will copy whole file as default of TFileStream
  if buf>0 then Dfile.CopyFrom(SFile,buf);
     ProgressBar.StepIt;
  except
    ShowMessage('Error in copying!');
  end;
end;

procedure TForm1.CutBtnClick(Sender: TObject);
var
DFile:TFileStream;
Item: TListItem;
DFilename:string;
begin
  if (GetList=nil) and (ListView.SelCount>0)
  and FileExists(CGFilename) and SetupSavePath then
  begin   //1
    ProgressBar.Max:=0;

    Item := ListView.Selected;

    SFile:=TFileStream.Create(CGFilename,fmOpenRead);
   try
     while Item <> nil do
     begin
       SFile.Position:=PFileInfo(CGList[Item.Index]).Offset;
       DFilename:=Dupname(Dir+PFileInfo(CGList[Item.Index]).Filename);
       DFile:=TFileStream.Create(DFilename, fmCreate);
       try
        CopyCutFile(SFile, DFile, PFileInfo(CGList[Item.Index]).FileSize);
       finally
       DFile.Free;
       end;
       Item := ListView.GetNextItem(Item, sdAll, [isSelected]);
       Form1.Repaint;
    end;
     ProgressBar.Position:=100;
     ShowMessage('Complete');
   finally
     SFile.Free;
     ProgressBar.Position:=0;
     ListView.SetFocus;
   end;
   end;    //1
end;
             
//---------------------------------------------------------------------
//Others
//---------------------------------------------------------------------

procedure TForm1.OutputFolderClick(Sender: TObject);
begin
  if DirectoryExists(IncludeTrailingPathDelimiter(Edit.Text)) then
     DirDialog1.Root:=ExcludeTrailingPathDelimiter(Edit.Text)
  else DirDialog1.Root:='';
  if DirDialog1.Execute then Edit.Text:=DirDialog1.DirName;
end;

procedure TForm1.TrackBar1Change(Sender: TObject);
begin
  case TrackBar1.Position of
  0..9: begin
          TimeInterval:=(TrackBar1.Position+1)*10;
          TrackBar1.Hint:='TPF: '+inttostr(TimeInterval)+' ms';
        end;

 10..18:begin
          TimeInterval:=(TrackBar1.Position-8)*100;
          if TimeInterval=1000 then TrackBar1.Hint:='TPF: 1 s'
          else
          TrackBar1.Hint:='TPF: '+inttostr(TimeInterval)+' ms';
        end;

 19..27:begin
          TimeInterval:=(TrackBar1.Position-17)*1000;
          TrackBar1.Hint:='TPF: '+inttostr(TimeInterval div 1000)+' s';
        end;
     end;
  Form1.SlideShow.Interval:=TimeInterval;
end;

procedure TForm1.SlideShowTimer(Sender: TObject);
begin
  ListView.MultiSelect:=False;
  if LVIndex < Form1.ListView.Items.Count-1 then
  begin
    Form1.ListView.ItemIndex:=LVIndex+1;
    GetCG(ListView.ItemIndex);
    Form1.Image1Click(Self);
  end
  else
  Form2.StopTimer;
end;

procedure TForm1.PlayOggClick(Sender: TObject);
var
ID: array [0..3] of char;
SrcFile: TFileStream;
begin
  if (ListView.SelCount>0) and FileExists(CGFilename)
  and (BASS_Handle<>0)then
  begin
    StopOggClick(Self);
    GetFileInfo(ListView.Selected.Index);
    SrcFile:=TFileStream.Create(CGFilename,fmOpenRead or fmShareDenyWrite);
    SrcFile.Position:=Offset;
    SrcFile.Read(ID,4);
    SrcFile.Free;

    if ID='OggS' then
    begin
      OggStream:=BASS_StreamCreateFile(False, PChar(CGFilename),
                 Offset, FileSize, BASS_STREAM_AUTOFREE);
      BASS_ChannelPlay(OggStream, True);
    end;
  end;
end;

procedure TForm1.StopOggClick(Sender: TObject);
begin
  if BASS_Handle<>0 then BASS_ChannelStop(OggStream);
end;

procedure TForm1.BtnUpClick(Sender: TObject);
var
FocusIndex, Index:integer;
Item:TListItem;
pt:TPoint;
begin
  if (GetList<>nil) or (ListView.SelCount=0) then Exit;

  FocusIndex := ListView.ItemFocused.Index-1;

  Item := ListView.Selected;
  if Item.Index=0 then Exit;
  while Item <> nil do
  begin
    Index:=Item.Index;
    CGList.Move(Index, Index-1);
    ListView.Items.Item[Index].Selected:=False;
    ListView.Items.Item[Index-1].Selected:=True;
    ListView.UpdateItems(Index-1, Index);
    Item := ListView.GetNextItem(Item, sdAll, [isSelected]);
  end;
  ListView.Items.Item[FocusIndex].Focused:=True;
  pt:=Form1.ListView.ItemFocused.GetPosition;
  PostMessage(Form1.ListView.Handle,WM_RBUTTONDOWN,0,pt.X+pt.Y shl 16);
end;

procedure TForm1.BtnDownClick(Sender: TObject);
var
FocusIndex, Index, FirstSelndex, LastSelIndex: integer;
Item: TListItem;
pt: TPoint;
begin
  if (GetList<>nil) or (ListView.SelCount=0) then Exit;

  FocusIndex := ListView.ItemFocused.Index+1;

  Item := ListView.Selected;
  FirstSelndex:=Item.Index;
  LastSelIndex:=Item.Index;
  while Item <> nil do
  begin
    LastSelIndex:=Item.Index;
    Item := ListView.GetNextItem(Item, sdAll, [isSelected]);
  end;

  if LastSelIndex=CGList.Count-1 then Exit;

  for Index:=LastSelIndex downto FirstSelndex do
  begin
    if ListView.Items.Item[Index].Selected then
    begin
      CGList.Move(Index, Index+1);
      ListView.Items.Item[Index].Selected:=False;
      ListView.Items.Item[Index+1].Selected:=True;
      ListView.UpdateItems(Index, Index+1);
    end;
  end;
  ListView.Items.Item[FocusIndex].Focused:=True;
  pt:=Form1.ListView.ItemFocused.GetPosition;
  PostMessage(Form1.ListView.Handle,WM_RBUTTONDOWN,0,pt.X+pt.Y shl 16);
end;

procedure TForm1.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if (Shift=[ssCtrl]) and (Key=Ord('Z'))
  and(GetList<>nil) then GetList.Terminate
  else
  if (Shift=[ssCtrl]) and (Key=Ord('Z'))
  and (SaveCG<>nil) then SaveCG.Terminate
  else
  if (SaveCG<>nil) or (GetList<>nil) then Exit;
  
  if (Shift=[ssCtrl]) and (Key=Ord('A')) then ListView.SelectAll
  else
  if (Key=VK_F11) then ListView.ViewStyle:=vsList
  else
  if (Key=VK_F12) then ListView.ViewStyle:=vsReport
  else
  if (Key=VK_F5) then ListView.Refresh
  else
  if (Key=VK_NUMPAD1) then
  begin
    CGList.Sort(CompareNames);
    ListView.Repaint;
  end
  else
  if (Key=VK_NUMPAD2) then
  begin
    CGList.Sort(CompareExt);
    ListView.Repaint;
  end
  else
  if (Key=VK_NUMPAD3) then
  begin
    CompareSuffix;
    ListView.Repaint;
  end
  else
  if (Key=VK_NUMPAD4) then
  begin
    CGList.Sort(CompareSize);
    ListView.Repaint;
  end
  else
  if (Key=VK_RETURN) then Image1Click(nil);
end;

procedure TForm1.ComparePhrase;
var
i,j:integer;
begin
  PhraseDlg.Close;
  j:=0;
  for i:=CGList.Count-1 downto 0 do
  if AnsiContainsText(PFileInfo(CGList.Items[i]).Filename, PhraseText) then
  begin
    inc(j);
    CGList.Move(i, CGList.Count-j);
  end;
  ListView.Repaint;
end;

procedure TForm1.SortPhrase1Click(Sender: TObject);
begin
  PhraseDlg.Show;
end;

procedure TForm1.AddExtList1Click(Sender: TObject);
var
Item: TListItem;
begin
  if (GetList<>nil) or (ListView.SelCount=0) then Exit;
  Item := ListView.Selected;
  ExtList.Add(ExtractFileExt(PFileInfo(CGList[Item.Index]).Filename));
end;

procedure TForm1.LoadXORTable1Click(Sender: TObject);
var
AFile:TFileStream;
begin
  XorTableOK:=False;
  OpenDialog2.InitialDir:=XorPath;
  if OpenDialog2.Execute then
  begin
  AFile:=TFileStream.Create(OpenDialog2.FileName, fmOpenRead or fmShareDenyWrite);
  XORTableSize:=AFile.Size;
  if XORTableSize>$753000 then Exit;    //1600*1200*4

  if (XORTable=nil) then
     GetMem(XorTable, XORTableSize)
  else
     ReAllocMem(XorTable, XORTableSize);
  AFile.Read(XorTable^, XORTableSize);
  AFile.Free;
  XorTableOK:=True;
  ListViewClick(Self);
  end;
end;

procedure TForm1.Anime1Click(Sender: TObject);
begin
  Anime1.Checked:=not Anime1.Checked;
end;

procedure TForm1.Display;
begin
  PopUpMenu2.AutoPopup:=False;
  if ShowBMP and (not DisplayBMP.Empty) then
  begin
    Image1.Visible:=True;
    if DisplayBMP.Width>DisplayBMP.Height then
    begin
       Image1.Width:=Panel3.Width-2;
    if Image1.Width>=DisplayBMP.Width then Image1.Width:=DisplayBMP.Width;
       Image1.Height:=DisplayBMP.Height*Image1.Width div DisplayBMP.Width;
    end
    else
    begin
       Image1.Height:=Panel3.Height-2;
    if Image1.Height>=DisplayBMP.Height then Image1.Height:=DisplayBMP.Height;
       Image1.Width:=DisplayBMP.Width*Image1.Height div DisplayBMP.Height;
    end;
  Image1.Picture.Bitmap.Assign(DisplayBMP);
  PopUpMenu2.AutoPopup:=True;
  GotCG:=True;
  end
  else Image1.Visible:=False;
end;

procedure TForm1.SaveList;
var
Item:TListItem;
i:integer;
begin
  SetLength(DoList, ListView.SelCount);
  Item:=ListView.Selected;
  i:=0;
  while Item<>nil do
  begin
    DoList[i]:=Item.Index;
    inc(i);
    Item := ListView.GetNextItem(Item, sdAll, [isSelected]);
  end;
end;

procedure TForm1.Splitter1Moved(Sender: TObject);
begin
  if ListView.ItemFocused.Index<>ListView.ItemIndex then Exit;
  Display;
end;

procedure TForm1.AlphaBlendAutoSave1Click(Sender: TObject);
begin
  AlphaBlendBtnClick(AlphaBlendAutoSave1);
end;

procedure TForm1.SaveDialog1TypeChange(Sender: TObject);
begin
  case SaveDialog1.FilterIndex of
  1: begin
       SaveDialog1.DefaultExt:='.bmp';
       SaveDialog1.FileName:=ChangeFileExt(SaveDialog1.FileName, '.bmp');
     end;
  2: begin
       SaveDialog1.DefaultExt:='.png';
       SaveDialog1.FileName:=ChangeFileExt(SaveDialog1.FileName, '.png');
     end;
  end;
end;

end.
