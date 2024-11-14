REM Visual Studio 8.0 (2005) Express

@ECHO OFF
Set SdkProgramFiles=%ProgramFiles%
if "%ProgramFiles(x86)%" == "" goto programfiles_ok
Set ProgramFiles=%ProgramFiles(x86)%
:programfiles_ok
Set VCDIR=%ProgramFiles%\Microsoft Visual Studio 8\VC
Set VSCOMMON=%ProgramFiles%\Microsoft Visual Studio 8\Common7\IDE
Set MSSDK=%SdkProgramFiles%\Microsoft Platform SDK for Windows Server 2003 R2

if exist "%MSSDK%" goto sdk_ok
Set MSSDK=%VCDIR%\PlatformSDK
:sdk_ok

::###################################::
Set PATH=%VCDIR%\bin;%MSSDK%\bin;%VSCOMMON%;%PATH%
Set INCLUDE=%MSSDK%\include;%VCDIR%\include;%INCLUDE%
Set LIB=%MSSDK%\lib;%VCDIR%\lib;%LIB%

cd "%~dp0."
if not exist "..\Plugs" mkdir "..\Plugs"
cd .\QSearch

rc /r /Fo"QSearch.res" "QSearch.rc"
cl /O1 /GS- QSearch.c QSearchDlg.c QSearchSettDlg.c QSearchLng.c QSearchFindEx.c DialogSwitcher.c XMemStrFunc.c QSearch.res /LD /link kernel32.lib user32.lib comctl32.lib gdi32.lib Advapi32.lib /OPT:NOWIN98 /NODEFAULTLIB /ENTRY:DllMain /OUT:..\..\Plugs\QSearch.dll

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
