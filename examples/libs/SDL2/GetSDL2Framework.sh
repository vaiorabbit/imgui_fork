#!/usr/bin/sh
mkdir tmp
cd tmp
curl https://www.libsdl.org/release/SDL2-2.0.12.dmg -o SDL2-2.0.12.dmg
hdiutil mount SDL2-2.0.12.dmg

cp -a /Volumes/SDL2/ ../

cd ..
hdiutil unmount /Volumes/SDL2

# Usage:
# - $ mkdir imgui/examples/libs/SDL2
# - Put this script at imgui/examples/libs/SDL2
# - $ cd imgui/examples/libs/SDL2
# - $ zsh GetSDL2Framework.sh
# -> Then you'll get 'SDL2.framework' directory here.
