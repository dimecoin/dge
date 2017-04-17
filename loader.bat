@echo off

REM This is a loader for DGEngine
REM both DJGPP and OpenWatcom have deps
REM Turbo C build does not.

set DRIVE=%1:
set BUILD=%2
set PROG=%3

if "%DRIVE%"=="" goto :HELP
if "%BUILD%"=="" goto :HELP
if "%PROG%"=="" goto :HELP

echo Starting [ %BUILD% ::  %PROG% ]

set RUNTIMES=%DRIVE%\dge\runtimes

echo ---------------------------------------------------
set DJGPP=%RUNTIMES%\DJGPP\DJGPP.ENV
set PATH=%RUNTIMES%\DJGPP\bin\;%RUNTIMES%\dos32a\
PATH %RUNTIMES%\DJGPP\bin\;%RUNTIMES%\dos32a\
set 
echo ---------------------------------------------------

set BUILDDIR=%DRIVE%\dge\build\%BUILD%
%DRIVE%
cd %BUILDDIR%

REM dos32a is required for OpenWatcom.
if "%BUILD%"=="ow" dos32a.exe %BUILDDIR%\%PROG%

REM DJGPP needs to be set for DJGPP build
if "%BUILD%"=="dj" %BUILDDIR%\%PROG%

REM Turbo C requires nothing
if "%BUILD%"=="tc" %BUILDDIR%\%PROG%


goto :END

:HELP
	echo ####################################################
	echo Usage: [DRIVE LETTER] [BUILD] [PROGRAM]
	echo ####################################################
goto :END

:END

