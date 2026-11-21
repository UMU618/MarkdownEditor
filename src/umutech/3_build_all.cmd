echo off
cd /d "%~dp0"
pwsh.exe .\msix\pack.ps1 -Compress
pwsh.exe .\msix\build-msix.ps1
pause
