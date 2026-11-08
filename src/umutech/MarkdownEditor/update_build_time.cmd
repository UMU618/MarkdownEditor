echo off
cd /d "%~dp0"
pwsh.exe generate_build_time.ps1
type res\build_time.h
pause
