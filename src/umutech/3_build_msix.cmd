echo off
cd /d "%~dp0"
pwsh.exe .\msix\build-msix.ps1
pause
