REM Visual Studio 12.0 (2013) Express

@ECHO OFF
Set SdkProgramFiles=%ProgramFiles%
if "%ProgramFiles(x86)%" == "" goto programfiles_ok
Set ProgramFiles=%ProgramFiles(x86)%
:programfiles_ok
Set VCDIR=%ProgramFiles%\Microsoft Visual Studio 12.0\VC
Set VSCOMMON=%ProgramFiles%\Microsoft Visual Studio 12.0\Common7\IDE
Set MSSDK=%ProgramFiles%\Windows Kits\8.1

if exist "%MSSDK%" goto sdk_ok
Set MSSDK=%VCDIR%\PlatformSDK
:sdk_ok

set CLFLAGS=/Wall /wd4100 /wd4201 /wd4204 /wd4255 /wd4310 /wd4619 /wd4668 /wd4701 /wd4706 /wd4711 /wd4820 /wd4826
set BIT=64

::###################################::
set "PATH=%MSSDK%\bin\x86;%MSSDK%\bin;%VCDIR%\bin\x86_amd64;%VSCOMMON%;%VCDIR%\bin;%PATH%"
set "INCLUDE=%MSSDK%\include\um;%MSSDK%\include\shared;%VCDIR%\include;%INCLUDE%"
set "LIB=%MSSDK%\lib\winv6.3\um\x64;%VCDIR%\lib\amd64;%LIB%"
set MACHINE=AMD64

cd .\QSearch

rc /R /DRC_VERSIONBIT=%BIT% /Fo"QSearch.res" "QSearch.rc"
cl /O1 /GS- %CLFLAGS% /D "AKELPAD_X64" QSearch.c QSearchDlg.c QSearchLng.c QSearchFindEx.c DialogSwitcher.c XMemStrFunc.c QSearch.res /LD /link kernel32.lib user32.lib comctl32.lib gdi32.lib Advapi32.lib /MACHINE:%MACHINE% /NODEFAULTLIB /ENTRY:DllMain /OUT:..\..\Plugs64\QSearch.dll
REM + assembler output: /FAcs

if exist QSearch.res del QSearch.res
if exist QSearch.lib del QSearch.lib
if exist QSearch.exp del QSearch.exp
if exist QSearch.obj del QSearch.obj
if exist QSearchDlg.obj del QSearchDlg.obj
if exist QSearchLng.obj del QSearchLng.obj
if exist QSearchFindEx.obj del QSearchFindEx.obj
if exist DialogSwitcher.obj del DialogSwitcher.obj
if exist XMemStrFunc.obj del XMemStrFunc.obj
@PAUSE
