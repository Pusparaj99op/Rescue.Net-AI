@echo off
title RescueNet AI - Startup Script
color 0A

echo.
echo  ====================================
echo   RescueNet AI - Emergency Response
echo   System Startup Script v6.12
echo  ====================================
echo.

echo [1/5] Checking Node.js installation...
node --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Node.js is not installed or not in PATH
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
)
echo ✓ Node.js is installed

echo.
echo [2/5] Checking MongoDB connection...
ping localhost -n 1 >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Cannot ping localhost
)

echo.
echo [3/5] Installing/updating dependencies...
npm install
if %errorlevel% neq 0 (
    echo ERROR: Failed to install dependencies
    pause
    exit /b 1
)
echo ✓ Dependencies installed

echo.
echo [4/5] Checking configuration...
if not exist ".env" (
    echo WARNING: .env file not found, creating from example...
    copy .env.example .env >nul
    echo ✓ Created .env file from example
    echo IMPORTANT: Please edit .env file with your settings
) else (
    echo ✓ Configuration file exists
)

echo.
echo [5/5] Starting RescueNet AI server...
echo.
echo Server will start on http://localhost:3000
echo WebSocket will start on ws://localhost:8080
echo.
echo Press Ctrl+C to stop the server
echo.

npm start

echo.
echo Server stopped. Press any key to exit...
pause >nul
