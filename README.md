# Tsunagari Tile Engine

| BUILD      | STATUS                                                                                                                                                                            |
|------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Windows    | [![Appveyor](https://ci.appveyor.com/api/projects/status/github/TsunagariEngine/Tsunagari?svg=true)](https://ci.appveyor.com/project/TsunagariEngine/Tsunagari)                   |
| macOS      | [![TravisCI](https://api.travis-ci.com/TsunagariEngine/Tsunagari.svg)](https://app.travis-ci.com/github/TsunagariEngine/Tsunagari)                                                |
| Linux      | [![CircleCI](https://circleci.com/gh/TsunagariEngine/Tsunagari.svg?style=shield)](https://circleci.com/gh/TsunagariEngine/Tsunagari)                                              |
| FreeBSD 12 | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-12.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-12.yml)                                     |
| FreeBSD 13 | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-13.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/freebsd-13.yml)                                     |
| NetBSD 9   | [![builds.sr.ht](https://builds.sr.ht/~pdm/tsunagari/commits/netbsd-9.yml.svg)](https://builds.sr.ht/~pdm/tsunagari/commits/netbsd-9.yml)                                         |

[![LoC](https://tokei.rs/b1/github/TsunagariEngine/Tsunagari?category=code)](https://github.com/XAMPPRocky/tokei)

Tsunagari is a tiling game engine and C++ runtime.

Building Tsunagari requires:

- Clang 3.0+, GCC 4.4+, or MSVC 2010+
- CMake 2.8.2+

It works on many common PC operating systems. Specifically:

- Windows 10 1809+
- Mac OS X 10.11+
- Alpine Linux 3.1+, Arch Linux, Debian 7+, Fedora 20+, every version of NixOS, Ubuntu 11.04+, and Void Linux
  - Specifically, glibc 2.13+ or musl 1.1.5+, and binutils 2.22+. musl 1.2
    i386 is not supported.
- FreeBSD 11+
- NetBSD 7+
- Raspbian

It supports the following CPU architectures:

- x86\_64
- i686
- aarch64
- armelhf

For game engine builds it also requires:

- SDL2 and SDL2\_mixer

And supports:

- SDL2 input and audio/video output
- SDL2 and OpenGL 2 renderers
- [Tiled](https://www.mapeditor.org/) maps
- 16-bit-styled graphics
