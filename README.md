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

Tsunagari works on many common PC operating systems. It requires:

- Clang 3.0+, GCC 4.4+, or MSVC 2010+
- CMake 2.8.2+
- SDL2 and SDL2\_mixer (for game engine builds)

It supports:

- Windows 10 1809+
- Mac OS X 10.11+
- Alpine Linux 3.1+, Arch Linux, Debian 7+, Fedora 20+, every version of NixOS, Ubuntu 11.04+, and Void Linux
- FreeBSD 11+
- NetBSD 7+
- Raspbian

Specifically for GNU Linux, it has first class support for x86\_64, and
best-effort support for i386 and armelhf architectures with glibc 2.13+ or musl
1.1.5+, and binutils 2.22+. musl 1.2 i386 is not supported.
