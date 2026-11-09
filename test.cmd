@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (echo Run from an x64 Native Tools Command Prompt for Visual Studio. & exit /b 1)
if not exist "%~dp0build" mkdir "%~dp0build"
for %%T in (CosmeticPolicy HurtReactionPolicy ManualSwitchStamp RespawnDelay) do (
 cl /nologo /EHsc /std:c++17 /Fo"%~dp0build\Test-%%T.obj" /Fe"%~dp0build\Test-%%T.exe" "%~dp0Test-%%T.cpp"
 if errorlevel 1 exit /b 1
 "%~dp0build\Test-%%T.exe"
 if errorlevel 1 exit /b 1
)
exit /b 0
