@echo off
setlocal
set "EXIT_CODE=1"
cd /d "%~dp0" || goto :cleanup

for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Get-Date -Format 'yyyyMMddHHmmss'"`) do set "TIMESTAMP=%%i"

if not defined TIMESTAMP (
    echo Failed to generate timestamp.
    goto :cleanup
)

set "TEMP_INDEX=repomix-Lumen-%TIMESTAMP%.index"
set "TEMP_REPO=repomix-temp-%TIMESTAMP%"
set "OUT_FILE=repomix-Lumen-%TIMESTAMP%.xml"

for /f "usebackq delims=" %%i in (`git rev-parse --git-path index`) do set "REAL_INDEX=%%i"

if not defined REAL_INDEX (
    echo Failed to locate the git index.
    goto :cleanup
)

set "REAL_INDEX=%REAL_INDEX:/=\%"

copy /y "%REAL_INDEX%" "%TEMP_INDEX%" >nul
if errorlevel 1 (
    echo Failed to copy the git index.
    goto :cleanup
)

REM use a temporary index, dont touch the real index
set "GIT_INDEX_FILE=%TEMP_INDEX%"

REM temp index already holds any new files you added explicitly
REM stage modifications and deletions of tracked files
REM untracked files are left out
git add -u
if errorlevel 1 (
    echo Failed to stage changes in the temporary index.
    goto :cleanup
)

set "GIT_TARGET=%~1"
if "%GIT_TARGET%"=="" (
    for /f "usebackq delims=" %%t in (`git write-tree`) do set "GIT_TARGET=%%t"
)
git archive --format=zip -o "%TEMP_REPO%.zip" %GIT_TARGET%

echo Generating %OUT_FILE%...

powershell -NoProfile -Command "Expand-Archive -Path '%TEMP_REPO%.zip' -DestinationPath '%TEMP_REPO%'"
C:\Users\ggoed\.bun\bin\repomix "%TEMP_REPO%" -o "%OUT_FILE%"

if errorlevel 1 (
    echo Failed to generate XML.
    goto :cleanup
)

set "EXIT_CODE=0"

:cleanup

if defined TEMP_INDEX (
    if exist "%TEMP_INDEX%" del /q "%TEMP_INDEX%" >nul 2>&1
    if exist "%TEMP_INDEX%.lock" del /q "%TEMP_INDEX%.lock" >nul 2>&1
)

if defined TEMP_REPO (
    if exist "%TEMP_REPO%.zip" del /q "%TEMP_REPO%".zip >nul 2>&1
    rmdir /s /q "%TEMP_REPO%"
)

if not "%EXIT_CODE%"=="0" echo Operation failed or was incomplete.

endlocal
exit /b %EXIT_CODE%
