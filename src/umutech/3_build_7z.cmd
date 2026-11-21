echo off
cd /d "%~dp0"
pwsh.exe .\msix\pack.ps1 -Compress
pause
