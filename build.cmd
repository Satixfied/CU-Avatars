@echo off
setlocal
if "%~1"=="" (echo Usage: build.cmd "path to Script Hook V SDK" & exit /b 1)
if not exist "%~f1\inc\main.h" (echo Script Hook V SDK headers missing. & exit /b 1)
if not exist "%~f1\lib\ScriptHookV.lib" (echo Script Hook V SDK import library missing. & exit /b 1)
where cl >nul 2>nul
if errorlevel 1 (echo Run from an x64 Native Tools Command Prompt for Visual Studio. & exit /b 1)
if not exist "%~dp0build" mkdir "%~dp0build"
cl /nologo /LD /MT /EHsc /O2 /std:c++17 /I "%~f1\inc" "%~dp0CUAvatars.cpp" /Fo"%~dp0build\CUAvatars.obj" "%~f1\lib\ScriptHookV.lib" user32.lib winmm.lib /link /OUT:"%~dp0build\CUAvatars.asi" /IMPLIB:"%~dp0build\CUAvatars.lib"
exit /b %errorlevel%
