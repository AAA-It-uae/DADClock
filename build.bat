@echo off
rem SPDX-License-Identifier: Apache-2.0
rem Copyright 2026 Mohammad Taghi Alavi
setlocal

set "ROOT=%~dp0"
set "VS="

if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat" set "VS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools"
if not defined VS if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" set "VS=%ProgramFiles%\Microsoft Visual Studio\2022\Community"
if not defined VS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" set "VS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"

if not defined VS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "tokens=*" %%I in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do if not defined VS set "VS=%%I"
)

if not defined VS (
    echo Visual Studio Build Tools with the x86 C++ workload was not found.
    exit /b 1
)

call "%VS%\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 exit /b 1

cl /nologo /c /O1 /Os /Oi- /GS- /GR- /EHs-c- /Zl /W4 /D_WIN32_WINNT=0x0501 /DWINVER=0x0501 /Fo"%ROOT%clock.obj" "%ROOT%clock.cpp"
if errorlevel 1 exit /b 1

rc /nologo /fo "%ROOT%clock.res" "%ROOT%DADClock.rc"
if errorlevel 1 exit /b 1

link /nologo /MANIFEST:NO /SUBSYSTEM:WINDOWS,5.01 /ENTRY:EntryPoint /NODEFAULTLIB /OPT:REF /OPT:ICF /INCREMENTAL:NO /Brepro /DYNAMICBASE /NXCOMPAT /OUT:"%ROOT%DADClock.exe" "%ROOT%clock.obj" "%ROOT%clock.res" kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib
set "RESULT=%ERRORLEVEL%"
del /q "%ROOT%clock.obj" 2>nul
del /q "%ROOT%clock.res" 2>nul
exit /b %RESULT%
