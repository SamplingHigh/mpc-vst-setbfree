@echo off
setlocal
title setBfree Organ - Skin Studio
cd /d "%~dp0"

rem Prefer an explicitly configured mpc-vst-plugins checkout, then the
rem checkout used by this Force development workspace.
set "STUDIO_ROOT=%MPC_VST%"
if not exist "%STUDIO_ROOT%\tools\studio.py" set "STUDIO_ROOT=%~dp0..\..\working\research\mpc-vst-plugins-current"
if not exist "%STUDIO_ROOT%\tools\studio.py" (
  echo Could not find mpc-vst-plugins\tools\studio.py.
  echo Set MPC_VST to the Windows path of the mpc-vst-plugins checkout.
  pause
  exit /b 1
)

set "PY="
where py >nul 2>nul && set "PY=py -3"
if not defined PY if exist "%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" set PY="%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
if not defined PY where python >nul 2>nul && set "PY=python"
if not defined PY (
  echo Skin Studio needs Python 3.
  pause
  exit /b 1
)

echo Opening the setBfree Organ layout in Skin Studio...
echo Keep this window open while editing.
rem Port 0 asks Windows for a free local port, avoiding stale port 8765 servers.
%PY% "%STUDIO_ROOT%\tools\studio.py" serve "%~dp0vst.json" --host 127.0.0.1 --port 0 --open
if errorlevel 1 (
  echo.
  echo Skin Studio stopped with an error.
  pause
  exit /b 1
)
