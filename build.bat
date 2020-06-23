@echo off
setlocal enableextensions

set PROJDIR=%CD%
set SRC=%PROJDIR%\src
set VENDOR=%PROJDIR%\vendor
set SDL=%VENDOR%\SDL2-devel-2.0.12-VC\SDL2-2.0.12
set SDLLIB=%SDL%\lib\x64

set INCLUDES=-I%SDL%\include
set IMPORTS=%SDLLIB%\SDL2main.lib %SDLLIB%\SDL2.lib Shell32.lib
set CXXFLAGS=-O2 -nologo -GR- -EHs- -Zi -WX -W4 -wd4100 -wd4505 -wd4996 -wd4244 -wd4201 -wd4018

if not exist build (mkdir build)
pushd build

xcopy /Q /Y %SDLLIB%\SDL2.dll

cl %CXXFLAGS% %INCLUDES% %SRC%\main.cpp ^
   -Fd:cgol.pdb -Fe:cgol.exe ^
   -MT -link %IMPORTS% -opt:REF -subsystem:WINDOWS

popd
