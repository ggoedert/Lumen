@echo off
setlocal

:: ================================
:: Arguments passed by Visual Studio
:: ================================
set PROJECT_DIR=%~1
set OUT_DIR=%~2

echo Preparing Data directory
echo Project: %PROJECT_DIR%
echo Output:  %OUT_DIR%

:: ================================
:: Material Design Icons source
:: ================================
set MATERIAL_DESIGN_ICONS_DIR=%PROJECT_DIR%\External\material-design-icons
echo MaterialDesignIcons: %MATERIAL_DESIGN_ICONS_DIR%

if not exist "%MATERIAL_DESIGN_ICONS_DIR%\" (
    echo Error: material-design-icons data source missing
    exit /b 1
)

:: ================================
:: Data directory
:: ================================
set DATA_DIR=%OUT_DIR%\Data
echo DataDir: %DATA_DIR%

if not exist "%DATA_DIR%\" (
    mkdir "%DATA_DIR%"
    echo DataDir created
) else (
    echo DataDir already exists
)

:: ================================
:: Copy Material Design Icons
:: ================================
echo Copying Material Design Icons to %DATA_DIR%...
robocopy "%MATERIAL_DESIGN_ICONS_DIR%" "%DATA_DIR%\material-design-icons" /E /XO /R:0 /W:0 /NFL /NDL
if %ERRORLEVEL% GEQ 8 (
    echo ERROR: Robocopy failed with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

:: -- OTHER COMMANDS HERE --

:: ================================
:: Success
:: ================================
exit /b 0
