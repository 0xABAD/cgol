#!/bin/bash

# Before building, find the root directory where the .git folder exists.
while [ 1 ]
do
    if [[ -d '.git' ]]
    then
        break
    elif [[ $(pwd) = '/'  ]]
    then
        echo "No git repository found, exiting"
        exit 1
    else
        cd ..
    fi
done


ROOT="$(pwd)"
SRC="$ROOT/src"
VENDOR="$ROOT/vendor"

SDL_DIR="SDL2-2.0.12"

# Check if appropriate vendor directorys exist.
VENDORS="$VENDOR/$SDL_DIR"

for vendor in $VENDORS
do
    if [[ ! -d $vendor ]]
    then
        echo "Missing vendor directory $vendor... exiting"
        exit 1
    fi
done

# Setup a build directory.
if [[ ! -d "build" ]]
then
    mkdir build
fi
cd build

# Check if SDL2 has been built.  If not then build it.
if [[ ! -f "libSDL2.a" ]]
then
    echo "========================"
    echo "=   BUILDING SDL2      ="
    echo "========================\n"

    # setup temp directory
    mkdir sdl
    cd sdl

    # build
    $VENDOR/$SDL_DIR/configure
    make

    # copy static lib and remove temp directory
    cd ..
    cp "sdl/build/.libs/libSDL2.a" .
    rm -rf sdl
fi

SDL_INCLUDE="$VENDOR/$SDL_DIR/include"

SDL_LINK_FLAGS="-lm -liconv -lobjc"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,AVFoundation"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,AudioToolbox"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,CoreAudio"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,CoreVideo"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,CoreGraphics"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,CoreMotion"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,Cocoa"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,Carbon"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,Foundation"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,ForceFeedback"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,GameController"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-framework,IOKit"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-weak_framework,QuartzCore"
SDL_LINK_FLAGS="$SDL_LINK_FLAGS -Wl,-weak_framework,Metal"

clang -O2 -I $SDL_INCLUDE -L . -lSDL2 $SDL_LINK_FLAGS -std=c++11 -o cgol $SRC/main.cpp
