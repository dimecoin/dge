@echo off

REM This is a loader for DGEngine
REM both DJGPP and OpenWatcom have deps
REM Turbo C build does not.

echo ---------------------------------------------------
set DJGPP=C:\DJGPP\DJGPP.ENV
set PATH=%PATH%;C:\DJGPP\bin\;C:\;C:\vim\vim73;C:\bin;C:\build;C:\binw;C:\dge
set 
path
echo ---------------------------------------------------


set PROG=splash.exe

REM dos32a is required for OpenWatcom.
cd C:\dge\build\ow
REM dos32a.exe  C:\dge\build\ow\%PROG%

REM DJGPP needs to be set for DJGPP build
cd C:\dge\build\dj
REM C:\dge\build\dj\%PROG%

REM Turbo C requires nothing
cd C:\dge\build\tc
REM C:\dge\build\tc\%PROG%


