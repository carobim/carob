#!/bin/bash
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


dmg_staging="$build"/."$app".staging
dmg_intermediate="$build"/."$app".dmg
dmg="$build"/"$app".dmg

trap 'rm -rf "$dmg_staging"' ERR

mkdir -p "$dmg_staging"
cp -a "$dst" "$dmg_staging"/
ln -fs /Applications "$dmg_staging"/

hdiutil create -format UDRW -srcfolder "$dmg_staging" -volname "$app" -fs HFS+ \
               -o "$dmg_intermediate" -ov >/dev/null
rm -r "$dmg_staging"

trap 'rm -f "$dmg_intermediate"' ERR

dev=$(hdiutil attach -readwrite -noverify -noautoopen "$dmg_intermediate" |
      awk '/^\/dev/ { print $1; exit }')

trap 'echo AppleScript failed; hdiutil detach "$dev" >/dev/null; rm -f "$dmg_intermediate"' \
     ERR

while [ ! -d /Volumes/"$app" ]; do
    sleep 1
done

icon_height=152
icon_width=152
icon_offsetx=160
icon_offsety=128

app_x=$((icon_offsetx + 52))
app_y=$((icon_offsety + 56))
apps_x=$((app_x + 320))

menu_bar_height=30
window_width=$((apps_x + icon_offsetx + 58))
window_height=$((icon_offsety * 2 + icon_height + menu_bar_height))

window_top=128
window_bottom=$((window_top + window_height))
window_left=320
window_right=$((window_left + window_width))

osascript >/dev/null <<EOF
tell application "Finder"
    tell disk ("$app")
        open

        tell the icon view options of container window
            set arrangement to not arranged
            set icon size to 128
            set text size to 16
        end tell

        tell container window
            set current view to icon view
            set statusbar visible to false
            set toolbar visible to false
            set the bounds to { \
                $window_left, \
                $window_top, \
                $window_right, \
                $window_bottom \
            }
        end tell

        set position of item "$app.app" to {$app_x, $app_y}
        set position of item "Applications" to {$apps_x, $app_y}

        set the extension hidden of item "$app.app" to true

        close
    end tell
end tell
EOF

while [ ! -f /Volumes/"$app"/.DS_Store ]; do
    echo waiting
    sleep 1
done

bless --folder /Volumes/"$app" --openfolder /Volumes/"$app"

hdiutil detach "$dev" >/dev/null
while [ -d /Volumes/"$app" ]; do
    echo waiting
    sleep 1
done

trap 'rm "$dmg_intermediate"' ERR

hdiutil convert "$dmg_intermediate" -format UDRO -o "$dmg_intermediate" -ov \
    >/dev/null
mv "$dmg_intermediate" "$dmg"
