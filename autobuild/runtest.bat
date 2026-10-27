@echo off
setlocal

:: Optional test filter passed as first argument (default: BrainCloudS2S)
set FILTER=%~1
if "%FILTER%"=="" set FILTER=BrainCloudS2S

set UPROJECT=%WORKSPACE%\PluginBuild\HostProject\HostProject.uproject

if not exist "%UPROJECT%" (
    echo ERROR: HostProject not found at %UPROJECT%
    echo Run: node bccm test unreal_s2s
    exit /b 1
)

echo.
echo Running Unreal automation tests: %FILTER%
echo UE:      %UE_INSTALL_PATH%
echo Project: %UPROJECT%
echo.

"%UE_INSTALL_PATH%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
    "%UPROJECT%" ^
    -ExecCmds="Automation RunTests %FILTER%;quit" ^
    -unattended ^
    -NullRHI ^
    -nosplash ^
    -nocontroller ^
    -nosound ^
    -log="%WORKSPACE%\artifacts\tests.log" ^
    -TestExit="Automation Test Queue Empty"

exit /b %ERRORLEVEL%
