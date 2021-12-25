cmake_minimum_required(VERSION 2.8.2...3.22)
#https://cmake.org/cmake/help/v2.8.2/cmake.html

# CMake 2.4.0 or earlier (<= 2006/04/18)
#   function cmake_minimum_required
#   function execute_progress
#   function find_package
#   function get_directory_property + PARENT_DIRECTORY
#   function get_filename_component + PATH
#   function source_group
#   function string + REPLACE
#   function target_link_libraries
#   keyword if(MATCHES)
#   variable BUILD_SHARED_LIBS
#   variable CMAKE_EXECUTABLE_SUFFIX
#   variable CMAKE_GENERATOR
#   variable CMAKE_<LANG>_COMPILER
#   variable CMAKE_OSX_SYSROOT
#   variable CMAKE_SYSTEM_NAME
#   variable MACOSX_BUNDLE*

# CMake 2.6.0 (2008/05/05)
#   package PkgConfig + pkg_search_module
#   variable CMAKE_<LANG>_COMPILER_ID

# CMake 2.8.0 (2009/11/13)
#   keyword set(PARENT_SCOPE)
#   keyword if(VERSION_LESS)
#   syntax generator-expressions
#   variable CMAKE_OSX_DEPLOYMENT_TARGET

# CMake 2.8.2 (2010/06/28)
#   compiler CLANG

# CMake 2.8.8 (2012/04/19)
#   variable CMAKE_<LANG>_COMPILER_VERSION

# CMake 2.8.11 (2013/05/15)
#   function target_compile_definitions

# CMake 3.8.0 (2017/04/10)
#   keyword source_group(TREE) + source_group(FILES)

project(TsunagariEngine C CXX)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "DEFAULT")
endif()


#
# CMake
#

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
    set(CMAKE_GTE_31 1)
endif()
if(NOT ${CMAKE_VERSION} VERSION_LESS 3.8)
    set(CMAKE_GTE_38 1)
endif()


#
# Compiler
#

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    # Assume that CMake >= 2.8.8 when Clang is used.
    set(CLANG ${CMAKE_CXX_COMPILER_VERSION})
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(GCC ${CMAKE_CXX_COMPILER_VERSION})
    if(NOT GCC)
        # CMake 2.8.7 or lower.
        execute_process(COMMAND "${CMAKE_CXX_COMPILER}" --version
                        OUTPUT_VARIABLE GCC_OUT)
        if("${GCC_OUT}" MATCHES "(4\\.[0-9])")
            set(GCC "${CMAKE_MATCH_1}")
        else()
            message(WARNING "Could not determine libc version")
            message(WARNING "${GCC_OUT}")
        endif()
    endif()
    if(NOT GCC VERSION_LESS 4.5)
        set(GCC_GTE_45 1)
    endif()
    if(NOT GCC VERSION_LESS 4.6)
        set(GCC_GTE_46 1)
    endif()
    if(GCC VERSION_LESS 4.7)
        set(GCC_LT_47 1)
    endif()
    if(NOT GCC VERSION_LESS 4.7)
        set(GCC_GTE_47 1)
    endif()
    if(NOT GCC VERSION_LESS 4.8)
        set(GCC_GTE_48 1)
    endif()
    if(NOT GCC VERSION_LESS 4.9)
        set(GCC_GTE_49 1)
    endif()
    if(NOT GCC VERSION_LESS 6)
        set(GCC_GTE_6 1)
    endif()
    if(GCC VERSION_LESS 7)
        set(GCC_LT_7 1)
    endif()
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    set(MSVC ${CMAKE_CXX_COMPILER_VERSION})
endif()


#
# OS
#

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(LINUX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    set(FREEBSD 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES NetBSD)
    set(NETBSD 1)
endif()


#
# Options
#

option(AV_NULL "Disable audio and video output")
option(AV_SDL2 "Use SDL2 for audio and video output")
option(AV_SDL2_GL "Use SDL2 for audio and window, and OpenGL for graphics")
option(AV_EM "Use Emscripten for audio and video output")
option(AV_CUSTOM "Select the AV backend options manually")
option(STATIC_SDL "Use a static SDL2 library at chosen location")
if(APPLE)
    option(UNIVERSAL "Build an AMD64/ARM64 universal binary")
endif()
if(CLANG OR GCC)
    option(NO_LTO "Disable link time optimization")
endif()

option(USE_SDL2_PKGCONFIG "Use pkg-config to find SDL2" ON)

option(BUILD_SHARED_LIBS "Build Tsunagari as a shared library")
option(STATIC_LINK "Build statically linked binaries")


#
# Option constraints
#

if(NOT AV_SDL2 AND NOT AV_SDL2_GL AND NOT AV_EM AND NOT AV_CUSTOM)
    set(AV_NULL ON)
endif()

if(GCC AND NOT GCC_GTE_45)
    set(NO_LTO ON)
endif()


#
# Variables
#

# Whether this is a subproject in a larger project
get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

# Project generator
if(CMAKE_GENERATOR MATCHES Ninja)
    set(NINJA 1)
endif()

if(AV_EM)
    set(WIN32 0)
    set(APPLE 0)
    set(LINUX 1)
    set(FREEBSD 0)
    set(NETBSD 0)
endif()

if(AV_NULL)
    set(AUDIO_NULL 1)
    set(WINDOW_NULL 1)
    set(RENDERER_NULL 1)
endif()

if(AV_SDL2)
    set(AUDIO_SDL2 1)
    set(WINDOW_SDL2 1)
    set(RENDERER_SDL2 1)
endif()

if(AV_SDL2_GL)
    set(AUDIO_SDL2 1)
    set(WINDOW_SDL2 1)
    set(RENDERER_GL 1)
endif()

if(AV_EM)
    set(AUDIO_SDL2 1)
    set(WINDOW_SDL2 1)
    set(RENDERER_SDL2 1)
endif()

if(AUDIO_SDL2 OR WINDOW_SDL2 OR RENDERER_SDL2)
    set(SDL2 ON)
endif()

# Build type
set(IS_DEBUG
    "$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>")
set(IS_RELEASE
    "$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>")


#
# Sources
#

get_filename_component(HERE "${CMAKE_CURRENT_LIST_FILE}" PATH)


#
# Sources in src
#

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/main.cpp
    ${HERE}/src/config.h
)

set(NULL_WORLD_SOURCES ${NULL_WORLD_SOURCES}
    ${HERE}/src/null-world.cpp
)
set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/pack/main.cpp
)

set(TEST_SOURCES ${TEST_SOURCES}
    ${HERE}/test/util/string-view.cpp
    ${HERE}/test/main.cpp
)

if(AUDIO_NULL)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/null/music.cpp
        ${HERE}/src/av/null/sounds.cpp
    )
endif()
if(WINDOW_NULL)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/null/window.cpp
    )
endif()
if(RENDERER_NULL)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/null/images.cpp
    )
endif()

if(SDL2)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/sdl2/error.cpp
        ${HERE}/src/av/sdl2/error.h
    )
endif()
if(AUDIO_SDL2)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/sdl2/music.cpp
        ${HERE}/src/av/sdl2/sounds.cpp
    )
endif()
if(WINDOW_SDL2)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/sdl2/window.cpp
        ${HERE}/src/av/sdl2/window.h
    )
endif()
if(RENDERER_SDL2)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/sdl2/images.cpp
    )
endif()

if(RENDERER_GL)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/gl/images.cpp
    )
endif()

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/data/data-area.cpp
    ${HERE}/src/data/data-area.h
    ${HERE}/src/data/data-world.h
    ${HERE}/src/data/inprogress.cpp
    ${HERE}/src/data/inprogress.h
    ${HERE}/src/data/inprogress-sound.h
    ${HERE}/src/data/inprogress-timer.h
)

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/os/c.h
    ${HERE}/src/os/chrono.h
    ${HERE}/src/os/mapped-file.h
    ${HERE}/src/os/mutex.h
    ${HERE}/src/os/thread.h
    ${HERE}/src/os/os.h
)
set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/os/c.h
    ${HERE}/src/os/condition-variable.h
    ${HERE}/src/os/io.h
    ${HERE}/src/os/mapped-file.h
    ${HERE}/src/os/mutex.h
    ${HERE}/src/os/os.h
    ${HERE}/src/os/thread.h
)
set(UTIL_SOURCES ${UTIL_SOURCES}
    ${HERE}/src/os/c.h
    ${HERE}/src/os/chrono.h
    ${HERE}/src/os/mapped-file.h
    ${HERE}/src/os/mutex.h
    ${HERE}/src/os/os.h
    ${HERE}/src/os/thread.h
)

if (WIN32)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/memmove.cpp

        # On MSVC2013 including this file results in a warning. In MSVC2015+
        # omitting it results in an error.
        ${HERE}/src/os/windows/c.cpp

        ${HERE}/src/os/windows/c.h
        ${HERE}/src/os/windows/chrono.cpp
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
        ${HERE}/src/os/windows/mapped-file.cpp
        ${HERE}/src/os/windows/mapped-file.h
        ${HERE}/src/os/windows/windows.cpp
        ${HERE}/src/os/windows/windows.h
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/memmove.cpp
        ${HERE}/src/os/windows/c.cpp
        ${HERE}/src/os/windows/c.h
        ${HERE}/src/os/windows/condition-variable.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
        ${HERE}/src/os/windows/mapped-file.cpp
        ${HERE}/src/os/windows/mapped-file.h
        ${HERE}/src/os/windows/mutex.h
        ${HERE}/src/os/windows/thread.h
        ${HERE}/src/os/windows/windows.cpp
        ${HERE}/src/os/windows/windows.h
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/memmove.cpp
        ${HERE}/src/os/windows/c.cpp
        ${HERE}/src/os/windows/c.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
        ${HERE}/src/os/windows/windows.cpp
        ${HERE}/src/os/windows/windows.h
    )
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/os/memmove.cpp

        # On MSVC2013 including this file results in a warning. In MSVC2015+
        # omitting it results in an error.
        ${HERE}/src/os/windows/c.cpp

        ${HERE}/src/os/windows/c.h
        ${HERE}/src/os/windows/chrono.cpp
        ${HERE}/src/os/windows/condition-variable.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
        ${HERE}/src/os/windows/mapped-file.cpp
        ${HERE}/src/os/windows/mapped-file.h
        ${HERE}/src/os/windows/mutex.h
        ${HERE}/src/os/windows/thread.h
        ${HERE}/src/os/windows/windows.cpp
        ${HERE}/src/os/windows/windows.h
    )
elseif(APPLE)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/mac/c.h
        ${HERE}/src/os/mac/chrono.cpp
        ${HERE}/src/os/mac/thread.cpp
        ${HERE}/src/os/mac/thread.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mapped-file.h
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/unix.cpp
    )
    if(NOT WINDOW_NULL OR NOT AUDIO_NULL)
        set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
            ${HERE}/src/os/mac/gui.mm
            ${HERE}/src/os/mac/gui.h
        )
    endif()
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/mac/c.h
        ${HERE}/src/os/mac/thread.cpp
        ${HERE}/src/os/mac/thread.h
        ${HERE}/src/os/unix/condition-variable.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mapped-file.h
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/unix.cpp
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/mac/c.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/unix.cpp
    )
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/os/mac/c.h
        ${HERE}/src/os/mac/chrono.cpp
        ${HERE}/src/os/mac/thread.cpp
        ${HERE}/src/os/mac/thread.h
        ${HERE}/src/os/unix/condition-variable.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mapped-file.h
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/unix.cpp
    )
else()
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/linux/c.h
        ${HERE}/src/os/unix/chrono.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/thread.h
        ${HERE}/src/os/unix/unix.cpp
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/linux/c.h
        ${HERE}/src/os/unix/condition-variable.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mapped-file.h
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/thread.h
        ${HERE}/src/os/unix/unix.cpp
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/linux/c.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/unix.cpp
    )
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/os/linux/c.h
        ${HERE}/src/os/unix/chrono.cpp
        ${HERE}/src/os/unix/condition-variable.h
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/thread.h
        ${HERE}/src/os/unix/unix.cpp
    )
endif()

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/pack/file-type.cpp
    ${HERE}/src/pack/file-type.h
    ${HERE}/src/pack/pack-reader.cpp
    ${HERE}/src/pack/pack-reader.h
)

set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/pack/file-type.cpp
    ${HERE}/src/pack/file-type.h
    ${HERE}/src/pack/pack-reader.cpp
    ${HERE}/src/pack/pack-reader.h
    ${HERE}/src/pack/pack-writer.cpp
    ${HERE}/src/pack/pack-writer.h
    ${HERE}/src/pack/walker.cpp
    ${HERE}/src/pack/walker.h
)

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/resources/pack.cpp
)

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/tiles/animation.cpp
    ${HERE}/src/tiles/animation.h
    ${HERE}/src/tiles/area.cpp
    ${HERE}/src/tiles/area.h
    ${HERE}/src/tiles/area-json.cpp
    ${HERE}/src/tiles/area-json.h
    ${HERE}/src/tiles/character.cpp
    ${HERE}/src/tiles/character.h
    ${HERE}/src/tiles/client-conf.cpp
    ${HERE}/src/tiles/client-conf.h
    ${HERE}/src/tiles/cooldown.cpp
    ${HERE}/src/tiles/cooldown.h
    ${HERE}/src/tiles/display-list.cpp
    ${HERE}/src/tiles/display-list.h
    ${HERE}/src/tiles/entity.cpp
    ${HERE}/src/tiles/entity.h
    ${HERE}/src/tiles/images.h
    ${HERE}/src/tiles/jsons.cpp
    ${HERE}/src/tiles/jsons.h
    ${HERE}/src/tiles/log.cpp
    ${HERE}/src/tiles/log.h
    ${HERE}/src/tiles/music.cpp
    ${HERE}/src/tiles/music-worker.h
    ${HERE}/src/tiles/music.h
    ${HERE}/src/tiles/npc.cpp
    ${HERE}/src/tiles/npc.h
    ${HERE}/src/tiles/overlay.cpp
    ${HERE}/src/tiles/overlay.h
    ${HERE}/src/tiles/player.cpp
    ${HERE}/src/tiles/player.h
    ${HERE}/src/tiles/resources.h
    ${HERE}/src/tiles/sounds.h
    ${HERE}/src/tiles/tile.cpp
    ${HERE}/src/tiles/tile.h
    ${HERE}/src/tiles/tile-grid.cpp
    ${HERE}/src/tiles/tile-grid.h
    ${HERE}/src/tiles/vec.cpp
    ${HERE}/src/tiles/vec.h
    ${HERE}/src/tiles/viewport.cpp
    ${HERE}/src/tiles/viewport.h
    ${HERE}/src/tiles/window.cpp
    ${HERE}/src/tiles/window.h
    ${HERE}/src/tiles/world.cpp
    ${HERE}/src/tiles/world.h
)

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/util/algorithm.h
    ${HERE}/src/util/align.h
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/compiler.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/function.h
    ${HERE}/src/util/hash.cpp
    ${HERE}/src/util/hash.h
    ${HERE}/src/util/hashtable.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/jobs.cpp
    ${HERE}/src/util/jobs.h
    ${HERE}/src/util/json.cpp
    ${HERE}/src/util/json.h
    ${HERE}/src/util/likely.h
    ${HERE}/src/util/list.h
    ${HERE}/src/util/markable.h
    ${HERE}/src/util/math2.h
    ${HERE}/src/util/measure.cpp
    ${HERE}/src/util/measure.h
    ${HERE}/src/util/move.h
    ${HERE}/src/util/new.cpp
    ${HERE}/src/util/new.h
    ${HERE}/src/util/pool.h
    ${HERE}/src/util/random.cpp
    ${HERE}/src/util/random.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
    ${HERE}/src/util/string2.cpp
    ${HERE}/src/util/string2.h
    ${HERE}/src/util/transform.c
    ${HERE}/src/util/transform.h
    ${HERE}/src/util/vector.h
)

set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/util/align.h
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/compiler.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/hashtable.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/list.h
    ${HERE}/src/util/move.h
    ${HERE}/src/util/new.cpp
    ${HERE}/src/util/new.h
    ${HERE}/src/util/queue.h
    ${HERE}/src/util/sort.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
    ${HERE}/src/util/vector.h
)

set(TEST_SOURCES ${TEST_SOURCES}
    ${HERE}/src/util/assert.cpp
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/compiler.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
)

set(UTIL_SOURCES ${UTIL_SOURCES}
    ${HERE}/src/util/align.h
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/compiler.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/function.h
    ${HERE}/src/util/hash.cpp
    ${HERE}/src/util/hash.h
    ${HERE}/src/util/hashtable.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/jobs.cpp
    ${HERE}/src/util/jobs.h
    ${HERE}/src/util/json.cpp
    ${HERE}/src/util/json.h
    ${HERE}/src/util/likely.h
    ${HERE}/src/util/list.h
    ${HERE}/src/util/markable.h
    ${HERE}/src/util/math2.h
    ${HERE}/src/util/move.h
    ${HERE}/src/util/new.cpp
    ${HERE}/src/util/new.h
    ${HERE}/src/util/pool.h
    ${HERE}/src/util/random.cpp
    ${HERE}/src/util/random.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
    ${HERE}/src/util/string2.cpp
    ${HERE}/src/util/string2.h
    ${HERE}/src/util/transform.c
    ${HERE}/src/util/transform.h
    ${HERE}/src/util/vector.h
)

if(MSVC OR XCODE OR NOT CMAKE_GTE_31)
    # MSVC 2017 and below, Xcode, and CMake 3.0 and below cannot vary source
    # files by configuration.
    # https://cmake.org/cmake/help/latest/release/3.1.html#properties
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/util/assert.cpp
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/util/assert.cpp
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/util/assert.cpp
    )
else()
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        $<${IS_DEBUG}:${HERE}/src/util/assert.cpp>
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        $<${IS_DEBUG}:${HERE}/src/util/assert.cpp>
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        $<${IS_DEBUG}:${HERE}/src/util/assert.cpp>
    )
endif()
set(UTIL_SOURCES ${UTIL_SOURCES}
    ${HERE}/src/util/assert.cpp
)



#
# Targets
#

if(UTIL_ONLY)
    set(TSTHREAD pthread)
    set(TSMATH m)

    add_library(tsutil ${UTIL_SOURCES})
else()
    add_library(tsunagari ${TSUNAGARI_SOURCES})
    add_executable(null-world ${NULL_WORLD_SOURCES})
    add_executable(pack-tool ${PACK_TOOL_SOURCES})
    add_executable(units ${TEST_SOURCES})

    target_link_libraries(null-world tsunagari)
endif()

include_directories(src)


#
# Source groups for IDEs
#

if(CMAKE_GTE_38)
    if(UTIL_ONLY)
        set(ALL_SOURCES ${UTIL_SOURCES})
    else()
        set(ALL_SOURCES ${TSUNAGARI_SOURCES}
                        ${NULL_WORLD_SOURCES}
                        ${PACK_TOOL_SOURCES}
                        ${TEST_SOURCES}
      )
    endif()
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_SOURCES})
endif()


#
# Preprocessor definitions
#

if(AUDIO_NULL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAUDIO_NULL")
endif()
if(AUDIO_SDL2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAUDIO_SDL2")
endif()
if(WINDOW_NULL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINDOW_NULL")
endif()
if(WINDOW_COCOA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINDOW_COCOA")
endif()
if(WINDOW_SDL2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINDOW_SDL2")
endif()
if(RENDERER_NULL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRENDERER_NULL")
endif()
if(RENDERER_SDL2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRENDERER_SDL2")
endif()
if(RENDERER_GL)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DRENDERER_GL")
endif()


#
# Compilation options
#

if(GCC_GTE_6)
    # Fix warning that happens in util/function.h because GCC cannot statically
    # analyze that the code is safe.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-placement-new")
endif()

if(CLANG OR GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

if (GCC_LT_47)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CLANG OR GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -nostdinc++")
endif()

# Disable C++ exceptions
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
elseif(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

# Disable DWARF unwind tables
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fno-unwind-tables -fno-asynchronous-unwind-tables")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-unwind-tables -fno-asynchronous-unwind-tables")
endif()

# Disable C++ run-time type information
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
elseif(MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
endif()

# Hide most symbols
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_C_FLAGS_MINSIZEREL
        "${CMAKE_C_FLAGS_MINSIZEREL} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fvisibility=hidden")
endif()

# Don't allow enum values outside enum range
# Warning: Careful! Not valid for C code.
if(CLANG OR GCC_GTE_46)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-enums")
endif()

# Don't allow thread-safe function-local static variables
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
endif()

# Enable warnings
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if(GCC AND GCC_GTE_46)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused-but-set-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-but-set-variable")
endif()
# On GCC 4.6 this warns about unused constants in int.h
if(GCC AND GCC_GTE_47)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused-variable")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-variable")
endif()

# Disable warnings when making old OS X builds since we don't use libstdc++
#if(APPLE AND CLANG)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-stdlibcxx-not-found")
#endif()

# -fno-implicit-templates

# Reduce code size
if(CLANG)
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Oz")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Oz")
endif()

# Enable floating point optimizations
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
elseif(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:fast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
endif()

# Disable stack canary
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")
elseif(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS-")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS-")
endif()

# Disable frame pointer
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE} -fomit-frame-pointer")
    set(CMAKE_C_FLAGS_MINSIZEREL
        "${CMAKE_C_FLAGS_MINSIZEREL} -fomit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fomit-frame-pointer")
endif()

# Optimize at link time
if((CLANG OR GCC) AND NOT NO_LTO)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -flto")
    #set(CMAKE_C_FLAGS_RELWITHDEBINFO
    #    "${CMAKE_C_FLAGS_RELWITHDEBINFO} -flto")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -flto")
    #set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    #    "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto")
endif()

# Enable NDEBUG anything not a debug build.
#if(${CMAKE_VERSION} VERSION_LESS 2.8.11 OR UTIL_ONLY)
    set(CMAKE_C_FLAGS_DEFAULT "${CMAKE_C_FLAGS_DEFAULT} -DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS_DEFAULT} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
#else()
#    target_compile_definitions(tsunagari PUBLIC $<$<NOT:${IS_DEBUG}>:NDEBUG>)
#    target_compile_definitions(pack-tool PRIVATE $<$<NOT:${IS_DEBUG}>:NDEBUG>)
#endif()

# Enable RELEASE in anything with optimizations.
#if(${CMAKE_VERSION} VERSION_LESS 2.8.11 OR UTIL_ONLY)
    set(CMAKE_C_FLAGS_MINSIZEREL
        "${CMAKE_C_FLAGS_MINSIZEREL} -DRELEASE")
    set(CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE} -DRELEASE")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO
        "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DRELEASE")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -DRELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DRELEASE")
#else()
#    target_compile_definitions(tsunagari PUBLIC $<${IS_RELEASE}:RELEASE>)
#    target_compile_definitions(pack-tool PRIVATE $<${IS_RELEASE}:RELEASE>)
#endif()

# Emscripten configuration
if(AV_EM)
    # TODO: Enable these in C?
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=0")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_THROWING=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ENVIRONMENT=web")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s HEADLESS=1")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s INITIAL_MEMORY=134217728")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s LLD_REPORT_UNDEFINED=1")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s PTHREADS_DEBUG=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS=[bmp]")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s STRICT=1")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SUPPORT_LONGJMP=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Enable color within compiler's output
if(NOT DEFINED ENV{CI})
    if(NINJA AND CLANG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
    endif()
    if(NINJA AND GCC_GTE_49)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    endif()
endif()

if(APPLE)
    # Metal is available on 64-bit machines from 10.11+.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")

    # Lowest that has an exported _start symbol on Xcode 12.5.
    #set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")
endif()

if(UNIVERSAL)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64 -arch arm64")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -arch x86_64 -arch arm64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -arch arm64")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -arch x86_64 -arch arm64")
endif()

# Share variables with parent
if(IS_SUBPROJECT)
    set(CMAKE_C_FLAGS
        ${CMAKE_C_FLAGS}
        PARENT_SCOPE)

    set(CMAKE_C_FLAGS_RELEASE
        ${CMAKE_C_FLAGS_RELEASE}
        PARENT_SCOPE)

    set(CMAKE_C_FLAGS_MINSIZEREL
        ${CMAKE_C_FLAGS_MINSIZEREL}
        PARENT_SCOPE)

    set(CMAKE_C_FLAGS_RELWITHDEBINFO
        ${CMAKE_C_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS_RELEASE
        ${CMAKE_CXX_FLAGS_RELEASE}
        PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS_MINSIZEREL
        ${CMAKE_CXX_FLAGS_MINSIZEREL}
        PARENT_SCOPE)

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)

    set(CMAKE_EXECUTABLE_SUFFIX
        ${CMAKE_EXECUTABLE_SUFFIX}
        PARENT_SCOPE)

    get_property(TSUNAGARI_INCLUDE_DIRECTORIES
        DIRECTORY
        PROPERTY INCLUDE_DIRECTORIES)
    set(TSUNAGARI_INCLUDE_DIRECTORIES
        ${TSUNAGARI_INCLUDE_DIRECTORIES}
        PARENT_SCOPE)
endif()


#
# Linking and externally-managed dependencies
#

if(CMAKE_OSX_SYSROOT)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_OSX_SYSROOT}/usr/lib")
endif()

if(STATIC_LINK)
    if(CLANG OR GCC)
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static")
    else()
        message(WARNING "Not sure how to statically link for this compiler")
    endif()
endif()

# Optimize at link time
if((CLANG OR GCC) AND NOT NO_LTO)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto=full")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -flto=full")
    #set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
    #   "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -flto=thin")
endif()
if(GCC_GTE_6 AND GCC_LT_7)
    set(CMAKE_AR gcc-ar)
    set(CMAKE_NM gcc-nm)
    set(CMAKE_RANLIB gcc-ranlib)
endif()

# Do not link to libstdc++ or libc++
if(APPLE OR LINUX OR FREEBSD OR NETBSD)
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

# Strip symbols
if(LINUX OR FREEBSD OR NETBSD)
    # GNU binutils
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--strip-all")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -Wl,--strip-all")
elseif(APPLE)
    # macOS linker
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-x -Wl,-dead_strip")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -Wl,-x -Wl,-dead_strip")
endif()

if(UNIVERSAL)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -arch x86_64 -arch arm64")
endif()

if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} --no-standard-libraries -lSystem")
endif()

if(APPLE AND NOT WINDOW_NULL)
    # FIXME: Change to be target-specific
    set(GUI_TYPE MACOSX_BUNDLE)
endif()

if(APPLE AND NOT WINDOW_NULL AND NOT AUDIO_NULL)
    target_link_libraries(tsunagari
        /System/Library/Frameworks/Cocoa.framework
        /System/Library/Frameworks/Foundation.framework
    )
endif()

if(LINUX AND NOT AV_EM)
    execute_process(COMMAND ldd --version
                    OUTPUT_VARIABLE LDD_OUT
                    ERROR_VARIABLE LDD_ERR)

    set(LDD_VERSION "${LDD_OUT}${LDD_ERR}")

    if("${LDD_VERSION}" MATCHES "GLIBC (2.[0-9]*)")
        set(GLIBC_VERSION "${CMAKE_MATCH_1}")
    elseif("${LDD_VERSION}" MATCHES "GNU libc. (2.[0-9]*)")
        set(GLIBC_VERSION "${CMAKE_MATCH_1}")
    elseif("${LDD_VERSION}" MATCHES "Version ([1-9][0-9.]*)")
        set(MUSL_VERSION "${CMAKE_MATCH_1}")
    else()
        message(WARNING "Could not determine libc version")
        message(WARNING "${LDD_VERSION}")
    endif()

    if("${GLIBC_VERSION}" VERSION_LESS "2.17")
        if(UTIL_ONLY)
            target_link_libraries(tsutil rt)
        else()
            target_link_libraries(tsunagari rt)
        endif()
    endif()

    if(GLIBC_VERSION)
        if(UTIL_ONLY)
            set(TSTHREAD pthread)
            set(TSMATH m)
        else()
            target_link_libraries(tsunagari m pthread)
        endif()
    endif()
endif()

if(FREEBSD OR NETBSD)
    if(UTIL_ONLY)
        set(TSTHREAD pthread)
        set(TSMATH m)
    else()
        target_link_libraries(tsunagari m pthread)
    endif()
endif()
if(AV_EM)
    target_link_libraries(tsunagari pthread)
endif()

if(STATIC_SDL)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${STATIC_SDL}")
    if(APPLE)
        if(WINDOW_SDL2)
            # FIXME: Make these specific to libtsunagari.
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lobjc -Wl,-framework,Carbon -Wl,-framework,Cocoa -Wl,-framework,CoreVideo -Wl,-framework,IOKit -Wl,-weak_framework,QuartzCore")
        endif()
        if(AUDIO_SDL2)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-framework,AudioToolbox -Wl,-framework,CoreAudio")
        endif()
        if(RENDERER_SDL2)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-weak_framework,Metal")
        endif()
    endif()
elseif(SDL2)
    if(USE_SDL2_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2 REQUIRED SDL2 sdl2)

        if("${SDL2_LIBRARIES}" STREQUAL "")
            message(SEND_ERROR "pkg-config could not find SDL2")
        endif()
    else()
        set(SDL2_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2 library")
        set(SDL2_LIBRARIES "" CACHE STRING
            "Name of SDL2 library")

        if("${SDL2_LIBRARIES}" STREQUAL "")
            message(SEND_ERROR "When not using pkg-config, please manually set"
                " SDL2 libraries")
        endif()
    endif()

    link_directories(${SDL2_LIBRARY_DIRS})
    target_link_libraries(tsunagari ${SDL2_LIBRARIES})
endif()

if(AUDIO_SDL2)
    if(USE_SDL2_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)
    else()
        set(SDL2_mixer_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2_mixer library")
        set(SDL2_mixer_LIBRARIES "" CACHE STRING
            "Name with SDL2_mixer library")
    endif()

    if("${SDL2_mixer_LIBRARIES}" STREQUAL "")
        message(SEND_ERROR "When not using pkg-config, please manually set"
            " SDL2_mixer libraries")
    endif()

    link_directories(${SDL2_mixer_LIBRARY_DIRS})
    target_link_libraries(tsunagari ${SDL2_mixer_LIBRARIES})
endif()

#if(RENDERER_SDL2)
#    if(USE_SDL2_PKGCONFIG)
#        pkg_search_module(SDL2_image REQUIRED SDL2_image)
#    else()
#        set(SDL2_image_INCLUDE_DIRS "" CACHE PATH
#            "Directory with SDL2_image headers")
#        set(SDL2_image_LIBRARY_DIRS "" CACHE PATH
#            "Directory with SDL2_image library")
#        set(SDL2_image_LIBRARIES "" CACHE STRING
#            "Name with SDL2_image library")
#    endif()
#
#    if("${SDL2_image_LIBRARIES}" STREQUAL "")
#        message(SEND_ERROR "When not using pkg-config, please manually set"
#            " SDL2_image include dirs and libraries")
#    endif()
#
#    target_include_directories(tsunagari PRIVATE ${SDL2_image_INCLUDE_DIRS})
#    link_directories(${SDL2_image_LIBRARY_DIRS})
#    target_link_libraries(tsunagari ${SDL2_image_LIBRARIES})
#endif()

# Share variables with parent
if(IS_SUBPROJECT)
    set(CMAKE_EXE_LINKER_FLAGS
        ${CMAKE_EXE_LINKER_FLAGS}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        ${CMAKE_EXE_LINKER_FLAGS_RELEASE}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        ${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}
        PARENT_SCOPE)
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
        ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)
    set(CMAKE_CXX_LINK_EXECUTABLE
        ${CMAKE_CXX_LINK_EXECUTABLE}
        PARENT_SCOPE)
endif()


#
# Policies
#

# Do not turn on -rdynamic. Also solved with CMP0065 in CMake 3.4
# FIXME: Apply it only to local targets, or possibly just try to
#        remove -rdynamic from these.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
