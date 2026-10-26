@echo off
setlocal
set "EXIT_CODE=1"
cd /d "%~dp0" || goto :cleanup

for /f "usebackq delims=" %%i in (`powershell -NoProfile -Command "Get-Date -Format 'yyyyMMddHHmmss'"`) do set "TIMESTAMP=%%i"

if not defined TIMESTAMP (
    echo Failed to generate timestamp.
    goto :cleanup
)

set "TEMP_INDEX=commit-review-Lumen-%TIMESTAMP%.index"
set "TEMP_DIFF=commit-temp-%TIMESTAMP%.diff"
set "OUT_FILE=commit-review-Lumen-%TIMESTAMP%.xml"

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

git diff --cached --quiet
if not errorlevel 1 (
    echo No changes to review.
    goto :cleanup
)

git diff --cached --binary > "%TEMP_DIFF%"
if errorlevel 1 (
    echo Failed to generate diff.
    goto :cleanup
)

echo Generating %OUT_FILE%...

powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference = 'Stop'; $diff = [System.IO.File]::ReadAllText($env:TEMP_DIFF, [System.Text.Encoding]::UTF8); $diff = [regex]::Replace($diff, '\r?\n', [Environment]::NewLine); $diff = $diff.Replace(']]>', ']]]]><![CDATA[>'); @('<git-review>', '  <patch><![CDATA[', $diff, '  ]]></patch>', '</git-review>') | Set-Content -Encoding utf8 $env:OUT_FILE"

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

if defined TEMP_DIFF (
    if exist "%TEMP_DIFF%" del /q "%TEMP_DIFF%" >nul 2>&1
)

if not "%EXIT_CODE%"=="0" echo Operation failed or was incomplete.

endlocal
exit /b %EXIT_CODE%