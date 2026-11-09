@echo off
setlocal
if "%~1"=="" (echo Usage: build.cmd "path to Script Hook V SDK" & exit /b 1)
if not exist "%~f1\lib\ScriptHookV.lib" exit /b 1
if not exist "%~dp0build" mkdir "%~dp0build"
cl /nologo /LD /MT /EHsc /O2 /std:c++17 /I "%~f1\inc" "%~dp0RuntimeEntry.cpp" /Fo"%~dp0build\CUAvatars.obj" "%~f1\lib\ScriptHookV.lib" user32.lib winmm.lib /link /OUT:"%~dp0build\CUAvatars.asi" /IMPLIB:"%~dp0build\CUAvatars.lib"
exit /b %errorlevel%
