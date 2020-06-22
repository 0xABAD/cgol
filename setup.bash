#!/bin/bash

# Find the root directory where the .git folder exists.
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
VENDOR="$ROOT/vendor"
SDL_DIR="SDL2-2.0.12"

if [[ ! -d "$VENDOR/$SDL_DIR" ]]
then
    echo "Downloading SDL2 source to vendor directory"
    cd $VENDOR
    curl "https://libsdl.org/release/$SDL_DIR.tar.gz" > "$SDL_DIR.tar.gz"
    tar -xzf "$SDL_DIR.tar.gz"
    rm "$SDL_DIR.tar.gz"
    cd ..
else
    echo "SDL2 already found in vendor directory"
fi
