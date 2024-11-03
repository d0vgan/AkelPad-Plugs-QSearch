@echo off

set ARC_EXE=7z.exe
set PROJECT_NAME=QSearch
for /f "tokens=1-3 delims=/.- " %%a in ('DATE /T') do set ARC_DATE=%%c%%b%%a
set ARC_NAME=%PROJECT_NAME%-%ARC_DATE%

%ARC_EXE% u -tzip .\%ARC_NAME%.zip @.\%PROJECT_NAME%_zip.files -mx5
%ARC_EXE% t %ARC_NAME%.zip

pause