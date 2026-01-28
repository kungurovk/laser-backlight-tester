@echo off
setlocal enabledelayedexpansion

set OUTPUT_FILE=git_version.h
set TEMP_FILE=%OUTPUT_FILE%.tmp

REM Get git tag (latest tag)
for /f "usebackq tokens=*" %%i in (`git describe --tags --abbrev=0^>nul`) do set GIT_TAG=%%i

REM Fallback to commit hash if no tags or command failed
if "%GIT_TAG%"=="" (
    for /f "usebackq tokens=*" %%i in (`git rev-parse --short HEAD 2^>nul`) do set GIT_TAG=%%i
)

REM If git command failed, set default
if "%GIT_TAG%"=="" set GIT_TAG=unknown

REM Generate header file
(
echo #ifndef GIT_VERSION_H
echo #define GIT_VERSION_H
echo.
echo namespace GitVersion {
echo     const char* const GIT_TAG = "%GIT_TAG%";
echo }
echo.
echo #endif // GIT_VERSION_H
) > %TEMP_FILE%

REM Only update if content changed
fc /b %TEMP_FILE% %OUTPUT_FILE% >nul 2>&1
if errorlevel 1 (
    move /y %TEMP_FILE% %OUTPUT_FILE% >nul
) else (
    del %TEMP_FILE% >nul
)

echo Generated git version: %GIT_TAG%
