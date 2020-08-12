{
  KiriKiri/KAG3 Embedded EXE Splitter & Merger
  Program module
  Copyright © 2008-2010 WinKiller Studio. Open Source.
  This software is free. Please see License for details.
}
program kiri_sm;

{$APPTYPE CONSOLE}

uses
  SysUtils,
  Classes,
  s_m;

begin
 if (paramstr(1) <> '') and ((paramstr(1) <> '-?') and (paramstr(1) <> lowercase('-h'))) then begin
  if paramstr(1) = lowercase('-e') then if (paramstr(2) <> '') and FileExists(paramstr(2)) then SplitIt;
  if paramstr(1) = lowercase('-m') then if (paramstr(2) <> '') and FileExists(paramstr(2)) then MergeIt;
 end else Help;
 writeln('Press Enter to quit.');
 readln;
end.
