# Tsunagari Tile Engine

| BUILD   | STATUS                                                                                                                                      |
| ------- | ------------------------------------------------------------------------------------------------------------------------------------------- |
| Windows | [![Appveyor](https://ci.appveyor.com/api/projects/status/github/TsunagariEngine/Tsunagari?svg=true)](https://ci.appveyor.com/project/TsunagariEngine/Tsunagari) |
| macOS   | [![TravisCI](https://api.travis-ci.com/TsunagariEngine/Tsunagari.svg)](https://travis-ci.com/TsunagariEngine/Tsunagari)                                         |
| Linux   | [![CircleCI](https://circleci.com/gh/TsunagariEngine/Tsunagari.svg?style=shield)](https://circleci.com/gh/TsunagariEngine/Tsunagari)                            |
| FreeBSD | [![builds.sr.ht](https://builds.sr.ht/~tsunagariengine/tsunagaric/freebsd.yml.svg)](https://builds.sr.ht/~tsunagariengine/tsunagaric/freebsd.yml)                   |
| NetBSD  | [![builds.sr.ht](https://builds.sr.ht/~tsunagariengine/tsunagaric/netbsd.yml.svg)](https://builds.sr.ht/~tsunagariengine/tsunagaric/netbsd.yml)                     |

[![LoC](https://tokei.rs/b1/github/TsunagariEngine/Tsunagari)](https://github.com/XAMPPRocky/tokei)

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

Requirements:

| NAME        | LICENSE     | LINK                   |
| ----------- | ----------- | ---------------------- |
| SDL2        | zlib        | http://www.libsdl.org  |

or

| NAME        | LICENSE     | LINK                         |
| ----------- | ----------- | ---------------------------- |
| libgosu     | MIT         | http://github.com/gosu/gosu  |
