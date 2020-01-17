@echo off
rem call clean_logs.bat
set PATH=%~dp0..\bin;%PATH%
rem start balancer_server.exe
start proxy_server.exe
