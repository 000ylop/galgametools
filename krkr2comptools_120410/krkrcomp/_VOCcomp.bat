set sourseX=I:\project_airSE_\Voice2\
set outdir=J:\vocout\

@echo off
mkdir %outdir%
mkdir %sourseX%conved
del file.lst
cls
echo ׼���ļ��б��С���
for %%i in (%sourseX%*.ogg) do (
echo %%i>>file.lst
echo %outdir%%%~ni.ogg>>file.lst
)
IF EXIST file.lst (
oggdec file.lst -m 4 -f 1 -l 
IF ERRORLEVEL 1 (
color cf
echo 
echo ת���������жϡ�
) else echo ת����ϡ�
del file.lst
for %%i in (%outdir%*.ogg) do if %%~zi LEQ 192 (del "%%i") else if exist "%sourseX%%%~ni.ogg" move "%sourseX%%%~ni.ogg" "%sourseX%conved\%%~ni.ogg"
)
echo.
pause
