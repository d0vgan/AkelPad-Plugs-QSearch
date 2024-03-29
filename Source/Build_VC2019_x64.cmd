REM Visual Studio 2019 Community or Professional

set VC_ROOT=%ProgramFiles(x86)%\Microsoft Visual Studio\2019

if exist "%VC_ROOT%\Professional\VC\Auxiliary\Build\vcvarsall.bat" goto UseVcProfessional
if exist "%VC_ROOT%\Community\VC\Auxiliary\Build\vcvarsall.bat" goto UseVcCommunity

goto ErrorNoVcVarsAll

:UseVcProfessional
call "%VC_ROOT%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
goto Building

:UseVcCommunity
call "%VC_ROOT%\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
goto Building

:Building
cd "%~dp0."
if not exist "..\Plugs64" mkdir "..\Plugs64"
cd .\QSearch

set BIT=64
set MACHINE=AMD64

rc /R /DRC_VERSIONBIT=%BIT% /Fo"QSearch.res" "QSearch.rc"
cl /O1 /GS- /D "AKELPAD_X64" QSearch.c QSearchDlg.c QSearchSettDlg.c QSearchLng.c QSearchFindEx.c DialogSwitcher.c XMemStrFunc.c QSearch.res /LD /link kernel32.lib user32.lib comctl32.lib gdi32.lib Advapi32.lib /MACHINE:%MACHINE% /NODEFAULTLIB /ENTRY:DllMain /OUT:..\..\Plugs64\QSearch.dll

if exist QSearch.res del QSearch.res
if exist QSearch.lib del QSearch.lib
if exist QSearch.exp del QSearch.exp
if exist QSearch.obj del QSearch.obj
if exist QSearchDlg.obj del QSearchDlg.obj
if exist QSearchSettDlg.obj del QSearchSettDlg.obj
if exist QSearchLng.obj del QSearchLng.obj
if exist QSearchFindEx.obj del QSearchFindEx.obj
if exist DialogSwitcher.obj del DialogSwitcher.obj
if exist XMemStrFunc.obj del XMemStrFunc.obj
@PAUSE
goto End

:ErrorNoVcVarsAll
echo ERROR: Could not find "vcvarsall.bat"
goto End

:End
