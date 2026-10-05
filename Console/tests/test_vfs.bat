@echo off
chcp 65001 > nul
setlocal
cd /d "%~dp0"

set EXE=Console.exe

if not exist %EXE% (
    echo [ERROR] %EXE% не найден!
    exit /b 1
)

echo === Тест 1: Минимальная VFS ===
%EXE% --vfs vfs_minimal.xml --script startup.txt

echo.
echo === Тест 2: Многоуровневая VFS ===
%EXE% --vfs vfs_deep.xml --script startup.txt

pause