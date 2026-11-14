echo off
cd /d "%~dp0"
pwsh.exe .\MarkdownEditor\generate_build_time.ps1
type .\MarkdownEditor\res\build_time.h
pause
