@echo off
setlocal enableextensions

set PROJDIR=%CD%
set VENDOR=%PROJDIR%\vendor
set SDL_DEV=SDL2-devel-2.0.12-VC

pushd %VENDOR%

echo "Downloading SDL2 development libraries to vendor directory"  
curl "https://www.libsdl.org/release/%SDL_DEV%.zip" > "%SDL_DEV%.zip"
unzip "%SDL_DEV%.zip"
del "%SDL_DEV%.zip"
popd
