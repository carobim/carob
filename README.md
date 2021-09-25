# Tsunagari Tile Engine

| BUILD       | STATUS                                                                                                                                                          |
| ----------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Windows     | [![Appveyor](https://ci.appveyor.com/api/projects/status/github/TsunagariEngine/Tsunagari?svg=true)](https://ci.appveyor.com/project/TsunagariEngine/Tsunagari) |
| macOS 10.11 | [![TravisCI](https://api.travis-ci.com/TsunagariEngine/Tsunagari.svg)](https://app.travis-ci.com/github/TsunagariEngine/Tsunagari)                              |
| macOS 11    | [![GitHub Actions](https://github.com/TsunagariEngine/Tsunagari/workflows/Build/badge.svg)](https://app.travis-ci.com/github/TsunagariEngine/Tsunagari)         |
| Linux       | [![CircleCI](https://circleci.com/gh/TsunagariEngine/Tsunagari.svg?style=shield)](https://circleci.com/gh/TsunagariEngine/Tsunagari)                            |
| FreeBSD 11  | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-11.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-11.yml)                   |
| FreeBSD 13  | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-13.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-13.yml)                   |
| NetBSD 9    | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/netbsd-9.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/netbsd-9.yml)                       |

[![LoC](https://tokei.rs/b1/github/TsunagariEngine/Tsunagari?category=code)](https://github.com/XAMPPRocky/tokei)

Tsunagari is a tiling game engine inspired by the cult classic game Yume
Nikki. It is intended as an open source alternative to the popular proprietary
RPGMaker game development suite that the original and most fangames were/are
written on. Tsunagari is written in C++, using the SDL2 framework.

Features:
Tsunagari will be a comprehensive game design suite for singleplayer 2D games.
The engine will support several styles, such as roguelikes, and old
console-style RPGs. Multiplayer support is planned for the second stable
release. Tsunagari allows C++ scripting for the event system, and for
additional custom functionality.

Current features include:
* Yume Nikki-like and roguelike movement modes.
* Music and sound effects handling.
* Tile and sprite animations.
* Intelligent viewport tracking.
* Support for infinite graphical and walkable layers.
* Resource caching for nonexistent load times.
* Command line options for fine-tuning.
* Subpixel rendering.
* Looping areas support.
* NPCs.
* Event scripting interface in C++.

Tsunagari works on many common PC operating systems. It requires Clang 3.0+,
GCC 4.4+, or MSVC 2010+, CMake 2.8.2, and SDL2, SDL2\_image, and SDL2\_mixer.
It has support for Windows 10 1809+, Mac OS X 10.11+, Alpine Linux 3.1+, Arch
Linux, Debian 7+, Fedora 20+, every version of NixOS, Ubuntu 11.04+, and Void
Linux, FreeBSD 11+ and NetBSD 7+, and the Raspberry Pi 1.

Specifically for GNU Linux, it has first class support for x86\_64, and
best-effort support for i386 and armelhf architectures with glibc 2.13+ or musl
1.1.5+, and binutils 2.22+. musl 1.2 i386 is not supported.

Requirements:

| NAME        | LICENSE     | LINK                   |
| ----------- | ----------- | ---------------------- |
| SDL2        | zlib        | http://www.libsdl.org  |
