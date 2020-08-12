{
  KiriKiri/KAG3 Embedded EXE Splitter & Merger
  Procedures & functions
  Copyright © 2008-2010 WinKiller Studio. Open Source.
  This software is free. Please see License for details.
}
unit s_m;

interface

uses Classes, SysUtils;

procedure SplitIt;
procedure MergeIt;

function OffsetLocate : int64;
function WhereAreWe : string;

procedure Help;

var ArchiveStream, Exe, Output : TStream;

implementation

procedure MergeIt;
label StopThis;
var Archive, Embedded : string;
begin
 if paramstr(4) <> '' then Embedded := paramstr(4) else Embedded := WhereAreWe+'embedded.exe';
 if (paramstr(3) <> '') and FileExists(paramstr(3)) then Archive := paramstr(3);
 if (paramstr(3) = '') and FileExists(WhereAreWe+'archive.xp3') then Archive := WhereAreWe+'archive.xp3';
 if (paramstr(3) = '') and (not FileExists(WhereAreWe+'archive.xp3')) then begin
  writeln('No archive for embedding has been found.'); writeln;
  goto StopThis;
 end;
 writeln('Merging:'); writeln;
 writeln(ExtractFileName(paramstr(2))+' & '+ExtractFileName(Archive)+' into '+ExtractFileName(Embedded)); writeln;
 try
  Exe := TFileStream.Create(paramstr(2),fmOpenRead);
  ArchiveStream := TFileStream.Create(Archive,fmOpenRead);
  Output := TFileStream.Create(Embedded,fmCreate);
  Output.CopyFrom(Exe,Exe.Size);
  Output.CopyFrom(ArchiveStream,ArchiveStream.Size);
  FreeAndNil(Exe);
  FreeAndNil(ArchiveStream);
  FreeAndNil(Output);
  write('Done. ');
 except
  FreeAndNil(Exe);
  FreeAndNil(ArchiveStream);
  FreeAndNil(Output);
  write('Failed. ');
 end;
StopThis:
end;

procedure SplitIt;
var Offset : int64;
label StopThis;
begin
 writeln('Splitting '+ExtractFileName(paramstr(2))+'...'); writeln;
 ArchiveStream := TFileStream.Create(WhereAreWe+paramstr(2),fmOpenRead);
 Offset := OffsetLocate;
 if Offset = -1 then goto StopThis;
 ArchiveStream.Position := 0;
 write('Extracting "_detach.exe"...');
 try
  Output := TFileStream.Create(WhereAreWe+'_detach.exe',fmCreate);
  Output.CopyFrom(ArchiveStream,Offset);
  FreeAndNil(Output);
  write('OK'); writeln; writeln;
 except
  FreeAndNil(Output);
  write('ERROR!'); writeln; writeln;
 end;
 write('Extracting "archive.xp3"...');
 try
  Output := TFileStream.Create(WhereAreWe+'archive.xp3',fmCreate);
  Output.CopyFrom(ArchiveStream,ArchiveStream.Size - Offset);
  FreeAndNil(Output);
  write('OK'); writeln; writeln;
 except
  FreeAndNil(Output);
  write('ERROR!'); writeln; writeln;
 end;
 write('Done. ');
StopThis:
 FreeAndNil(ArchiveStream);
end;

function WhereAreWe : string;
begin
 Result := ExtractFilePath(paramstr(0));
end;

function OffsetLocate : int64;
var i : integer; j : longword; SearchStream : TStream;
const buf_size = $1000000;
begin
 writeln('Locating offset. This may take a few seconds, please wait...'); writeln;
 Result := -1;

 SearchStream := TMemoryStream.Create;
 if ArchiveStream.Size < buf_size then SearchStream.Size := ArchiveStream.Size else SearchStream.Size := buf_size;
 SearchStream.CopyFrom(ArchiveStream,SearchStream.Size);

 with SearchStream do begin
  for i := 0 to Size-1 do begin
   Position := i; //resetting position
   j := 0;
   Read(j,4);
   if j = $0D335058 then begin
    j := 0;
    Read(j,4);
    if j = $1A0A200A then begin
     j := 0;
     Read(j,3);
     if j = $01678B then begin
      Result := i;
      writeln('Archive offset located at '+inttostr(i)+' (0x'+inttohex(i,8)+')'); writeln;
      break;
     end;
    end;
   end;
  end;
 end;

 if Result = -1 then writeln('Failed to locate the archive offset. Not an KiriKiri archive?');
end;

procedure Help;
var Help : array [0..27] of string; i : integer;
begin
 Help[ 0] := 'KiriKiri/KAG3 Embedded EXE Splitter & Merger [2010/10/04]';
 Help[ 1] := 'Copyright (c) 2008-2010 WinKiller Studio. Open Source.';
 Help[ 2] := 'This software is free. Please see License for details.';
 Help[ 3] := '';
 Help[ 4] := 'Usage:';
 Help[ 5] := '';
 Help[ 6] := ExtractFileName(paramstr(0))+' -option exe_name [archive] [output]';
 Help[ 7] := '';
 Help[ 8] := 'exe_name - embedded archive or executable filename';
 Help[ 9] := 'archive  - name of the archive for embedding. Default is "archive.xp3"';
 Help[10] := 'output   - name of the output embedded archive. Default is "embedded.exe"';
 Help[11] := '';
 Help[12] := 'Note: when no archive name is specified, it will try to look for "archive.xp3".';
 Help[13] := '';
 Help[14] := 'Possible options:';
 Help[15] := '';
 Help[16] := '-? or -h - show this help screen';
 Help[17] := '-e       - extract embedded archive and EXE file';
 Help[18] := '-m       - merge executable and archive';
 Help[19] := '';
 Help[20] := 'Examples:';
 Help[21] := '';
 Help[22] := ExtractFileName(paramstr(0))+' -e executable.exe';
 Help[23] := '';
 Help[24] := ExtractFileName(paramstr(0))+' -m executable.exe';
 Help[25] := ExtractFileName(paramstr(0))+' -m executable.exe archive.xp3';
 Help[26] := ExtractFileName(paramstr(0))+' -m executable.exe archive.xp3 embedded.exe';
 Help[27] := '';
 for i := 0 to 27 do writeln(Help[i]);
end;

end.
