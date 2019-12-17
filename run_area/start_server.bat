@echo off
call clean_logs.bat
set PATH=%~dp0..\bin;%PATH%
start balancer_server.exe
rem start proxy_server.exe
