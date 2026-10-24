@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Update-SkinOnForce.ps1"
if errorlevel 1 (
  echo.
  echo Skin update failed.
  echo Full log: %~dp0Update-SkinOnForce.log
  pause
  exit /b 1
)
echo.
pause
