echo off
cd /d "%~dp0"
MSBuild.exe .\MarkdownEditor.slnx -m -p:Configuration=MTRelease -p:Platform=ARM64
MSBuild.exe .\MarkdownEditor.slnx -m -p:Configuration=MTRelease -p:Platform=x64
pause
