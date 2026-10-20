@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-SkinOnForce.ps1"
if errorlevel 1 (
  echo.
  echo Skin update failed.
  pause
  exit /b 1
)
echo.
pause
