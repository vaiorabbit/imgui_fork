#!/usr/bin/sh

if [ ! -f NotoSansCJKjp-hinted.zip ]; then
    curl https://noto-website-2.storage.googleapis.com/pkgs/NotoSansCJKjp-hinted.zip -o NotoSansCJKjp-hinted.zip
fi

mkdir NotoSansCJKjp
unzip -o NotoSansCJKjp-hinted.zip -d NotoSansCJKjp

