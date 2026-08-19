@echo off
setlocal
title VEX Desktop Simulator
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0sim\start-simulator.ps1"
if errorlevel 1 (
  echo.
  echo Simulator startup failed. Check Node.js and Visual Studio C++ Build Tools.
  pause
  exit /b 1
)
endlocal
