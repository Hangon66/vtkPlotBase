@echo off
REM ============================================================
REM  VTKtest 编译脚本 — Qt 6.7.3 + MSVC 2019 64-bit + qmake/jom
REM  用法: 双击运行，或在 x64 Native Tools 终端中运行
REM ============================================================

echo.
echo ============================================================
echo  VTKtest Build Script (qmake + jom)
echo ============================================================
echo.

REM --- 加载 MSVC 编译器环境 ---
where cl.exe >nul 2>nul
if errorlevel 1 (
    echo [ENV] Loading MSVC x64 environment...
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

REM --- 移除 MinGW 路径干扰 (Strawberry Perl 等) ---
set PATH=%PATH:D:\Strawberry\c\bin;=%

REM --- 路径设置 ---
set SRC=d:\Programs\Qt\6.7.3Project\VTKtest
set OUT=d:\Programs\Qt\6.7.3Project\VTKtest\build\Desktop_Qt_6_7_3_MSVC2019_64bit-Release
set QT_KIT=C:\Programs\Qt\Qt6.7.3\6.7.3\msvc2019_64
set QMAKE=%QT_KIT%\bin\qmake.exe
set JOM=C:\Programs\Qt\Qt6.7.3\Tools\QtCreator\bin\jom.exe

echo  Source: %SRC%
echo  Output: %OUT%
echo  Qt Kit: %QT_KIT%
echo  qmake:  %QMAKE%
echo  jom:    %JOM%
echo.

REM --- 创建构建目录 ---
if not exist "%OUT%" (
    mkdir "%OUT%"
)

cd /d "%OUT%"

REM --- qmake 生成 Makefile ---
echo [1/2] Running qmake...
"%QMAKE%" "%SRC%\VTKtest.pro" -spec win32-msvc "CONFIG+=release"
if errorlevel 1 (
    echo [ERROR] qmake failed!
    pause
    exit /b 1
)

REM --- jom 编译 (若 jom 不存在则回退到 nmake) ---
echo [2/2] Building...
if exist "%JOM%" (
    "%JOM%" -j%NUMBER_OF_PROCESSORS%
) else (
    echo [WARN] jom not found, falling back to nmake...
    nmake
)

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ============================================================
echo  Build OK!
echo  Output: %OUT%
echo ============================================================
pause
