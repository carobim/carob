#!/bin/sh
set -euo pipefail

help() {
    echo 'Usage: build-macos-bundle.sh BUILD_DIR EXE_FILENAME APP_NAME [WORLD_DATA_DIR WORLD_FILE]'
}

if [ $# = 0 ] || ! [ $# = 3 -o $# = 5 ] || \
   [ "$1" = '-h' -o "$1" = '--help' ]; then
    help
    exit 0
fi

build="$1"
exe_filename="$2"
app="$3"
worlddir="${4-}"
worldfile="${5-}"

if [ ! -d "$build" ] || [ ! -f "$build"/CMakeCache.txt ]; then
    help
    exit 1
fi

pwd="$PWD"
tsulib="${0%/*}/.."
packtool="$build"/Tsunagari/pack-tool

exe="$build"/"$exe_filename"
info="$tsulib"/data/Info.plist  # TODO: Make customizable.
icns="$tsulib"/data/Tsunagari.icns  # TODO: Make customizable.
client="$tsulib"/data/client.json  # TODO: Make customizable. Or replace.

staging="$build"/"$app".app.tmp
dst="$build"/"$app".app

contents="$staging"/Contents
macos="$contents"/MacOS
bundle_exe="$macos"/Tsunagari  # TODO: Make customizable.
resources="$contents"/Resources

for x in "$client" "$exe" "$icns" "$info" "$packtool"; do
    if [ ! -e "$x" ]; then
        echo "Cannot find \"$x\""
        exit 1
    fi
done

if [ -n "$worlddir" ]; then
    (
        cd "$worlddir"
        "$pwd"/"$packtool" create "$pwd"/"$build"/"$worldfile" *
    )
fi

trap 'rm -rf "$staging"' ERR
mkdir -p "$macos" "$resources"

cp -a "$exe" "$bundle_exe"
cp -a "$info" "$contents"
cp -a "$client" "$icns" "$build"/"$worldfile" "$resources"

rm -rf "$dst"
mv "$staging" "$dst"


dmg_staging="$build"/"$app".dmg.tmp
dmg="$build"/"$app".dmg

trap 'rm -rf "$dmg_staging"' ERR

mkdir -p "$dmg_staging"
cp -a "$dst" "$dmg_staging"/
ln -fs /Applications "$dmg_staging"/
hdiutil create -srcfolder "$dmg_staging" -volname "$app" -fs HFS+ -o "$dmg" -ov
