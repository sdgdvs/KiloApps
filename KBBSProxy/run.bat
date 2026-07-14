@echo off
echo Starting KBBS Proxy...
cmd /c npm install
node telnet-proxy.js
pause
