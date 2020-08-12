set enccfg=-q 12 -Q 16
set sourseX=C:\tmp\char\character1\
set outdir=J:\outuci\

@echo off
set sf1=%sourseX%*.bmp;%sourseX%*.png;%sourseX%*.jpg
set sf2=%sourseX%*.tlg
if not "%~1%" == "" (
set sf1=%~1%
set sf2=%~1%
%~d0%
cd %~p0%
set outdir=%~dp1%
)
mkdir %outdir% 
for %%i in (%sf1%) do (
imgdec - - < %%i | ucienc - -o "%outdir%%%~ni.uci" %enccfg% -quiet
)
for %%i in (%sf2%) do (
crage.exe -u kirikiri2 -p "%%i" -o %outdir% >NUL
ucienc "%outdir%%%~ni.bmp" %enccfg% -quiet 
del "%outdir%%%~ni.bmp"
)
if "%~1%" == "" (
echo 
echo ×ª»»Íê±Ï¡£
pause
)