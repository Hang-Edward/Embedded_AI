@echo off
set "UCRT64_PATH=C:\Users\35342\scoop\apps\msys2\current\ucrt64\bin"
set "PATH=%UCRT64_PATH%;%PATH%"
start "" "%~dp0build-msys2-gui\embedded_ai_gui.exe"
