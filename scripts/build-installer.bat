@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0build-installer.ps1" %*
