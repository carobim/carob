cmake_minimum_required(VERSION 2.8.2...3.25)
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

project(carob C CXX)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Default)
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
    if(NOT CLANG VERSION_LESS 6.0)
        set(CLANG_GTE_6 1)
    endif()
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
option(USE_SDL2_PKGCONFIG "Use pkg-config to find SDL2" ON)
option(CURL "Enable curl for HTTP")

option(BUILD_SHARED_LIBS "Build Carob as a shared library")
option(STATIC_LINK "Build statically linked binaries")
if(APPLE)
    option(UNIVERSAL "Build an AMD64/ARM64 universal binary")
endif()
if(CLANG OR GCC)
    option(NO_LTO "Disable link time optimization")
endif()


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

set(CAROB_SOURCES ${CAROB_SOURCES}
    ${HERE}/src/tiles/main.cpp
)

set(NULL_WORLD_SOURCES ${NULL_WORLD_SOURCES}
    ${HERE}/src/tiles/null-world.cpp
)
set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/pack/main.cpp
)

set(TEST_SOURCES ${TEST_SOURCES}
    ${HERE}/test/util/string-view.cpp
    ${HERE}/test/main.cpp
)

if(AUDIO_NULL)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/null/music.cpp
        ${HERE}/src/av/null/sounds.cpp
    )
endif()
if(WINDOW_NULL)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/null/window.cpp
    )
endif()
if(RENDERER_NULL)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/null/images.cpp
    )
endif()

if(SDL2)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/sdl2/error.cpp
        ${HERE}/src/av/sdl2/error.h
    )
endif()
if(AUDIO_SDL2)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/sdl2/music.cpp
        ${HERE}/src/av/sdl2/sounds.cpp
    )
endif()
if(WINDOW_SDL2)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/sdl2/window.cpp
        ${HERE}/src/av/sdl2/window.h
    )
endif()
if(RENDERER_SDL2)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/sdl2/images.cpp
    )
endif()

if(RENDERER_GL)
    set(CAROB_SOURCES ${CAROB_SOURCES}
        ${HERE}/src/av/gl/images.cpp
    )
endif()

set(CAROB_SOURCES ${CAROB_SOURCES}
    ${HERE}/src/data/action.cpp
    ${HERE}/src/data/action.h
    ${HERE}/src/data/data-area.cpp
    ${HERE}/src/data/data-area.h
    ${HERE}/src/data/data-world.h
)

set(UTIL_SOURCES ${UTIL_SOURCES}
    ${HERE}/src/net/html.cpp
    ${HERE}/src/net/html.h
)
if(CURL)
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/net/http.h
        ${HERE}/src/net/http_curl.cpp
    )
endif()

set(UTIL_SOURCES ${UTIL_SOURCES}
    ${HERE}/src/os/c.h
    ${HERE}/src/os/chrono.h
    ${HERE}/src/os/condition-variable.h
    ${HERE}/src/os/mapped-file.h
    ${HERE}/src/os/mutex.h
    ${HERE}/src/os/os.h
    ${HERE}/src/os/thread.h
)

if(WIN32)
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
    if(NOT WINDOW_NULL OR NOT AUDIO_NULL)
        set(CAROB_SOURCES ${CAROB_SOURCES}
            ${HERE}/src/os/mac/gui.mm
            ${HERE}/src/os/mac/gui.h
    )
    endif()
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/os/mac/c.h
        ${HERE}/src/os/mac/chrono.cpp
        ${HERE}/src/os/mac/dir.cpp
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
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/os/linux/c.h
        ${HERE}/src/os/unix/chrono.cpp
        ${HERE}/src/os/unix/condition-variable.h
        ${HERE}/src/os/unix/dir.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
        ${HERE}/src/os/unix/mapped-file.cpp
        ${HERE}/src/os/unix/mutex.h
        ${HERE}/src/os/unix/thread.h
        ${HERE}/src/os/unix/unix.cpp
    )
endif()

set(CAROB_SOURCES ${CAROB_SOURCES}
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

set(CAROB_SOURCES ${CAROB_SOURCES}
    ${HERE}/src/resources/pack.cpp
)

set(CAROB_SOURCES ${CAROB_SOURCES}
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

set(UTIL_SOURCES ${UTIL_SOURCES}
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
    ${HERE}/src/util/hashvector.h
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
    ${HERE}/src/util/queue.h
    ${HERE}/src/util/random.cpp
    ${HERE}/src/util/random.h
    ${HERE}/src/util/sort.h
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

if(MSVC OR XCODE)
    # MSVC 2017 and below and Xcode cannot vary source files by configuration.
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/util/assert.cpp
    )
    # FIXME: Use a generator exrpression on MSVC 2019+ and CMake 3.1+.
elseif(CMAKE_GTE_31)
    # https://cmake.org/cmake/help/latest/release/3.1.html#properties
    set(UTIL_SOURCES ${UTIL_SOURCES}
        $<${IS_DEBUG}:${HERE}/src/util/assert.cpp>
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug" OR
       CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(UTIL_SOURCES ${UTIL_SOURCES}
        ${HERE}/src/util/assert.cpp
    )
endif()


#
# Targets
#

if(UTIL_ONLY)
    set(CTHREAD pthread)
    set(CMATH m)

    add_library(cutil ${UTIL_SOURCES})
else()
    add_library(cutil ${UTIL_SOURCES})
    add_library(carob ${CAROB_SOURCES})
    add_executable(null-world ${NULL_WORLD_SOURCES})
    add_executable(pack-tool ${PACK_TOOL_SOURCES})
    add_executable(units ${TEST_SOURCES})

    target_link_libraries(carob)
    target_link_libraries(null-world carob cutil)
    target_link_libraries(pack-tool cutil)
    target_link_libraries(units cutil)
endif()

include_directories(src)


#
# Source groups for IDEs
#

if(CMAKE_GTE_38)
    if(UTIL_ONLY)
        set(ALL_SOURCES ${UTIL_SOURCES})
    else()
        set(ALL_SOURCES ${CAROB_SOURCES}
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
macro(add_c_flag text)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${text}")
endmacro()
macro(add_cxx_flag text)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${text}")
endmacro()
macro(add_c_and_cxx_flag text)
    add_c_flag("${text}")
    add_cxx_flag("${text}")
endmacro()
macro(add_link_flag text)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${text}")
endmacro()

if(AUDIO_NULL)
    add_cxx_flag("-DAUDIO_NULL")
endif()
if(AUDIO_SDL2)
    add_cxx_flag("-DAUDIO_SDL2")
endif()
if(WINDOW_NULL)
    add_cxx_flag("-DWINDOW_NULL")
endif()
if(WINDOW_COCOA)
    add_cxx_flag("-DWINDOW_COCOA")
endif()
if(WINDOW_SDL2)
    add_cxx_flag("-DWINDOW_SDL2")
endif()
if(RENDERER_NULL)
    add_cxx_flag("-DRENDERER_NULL")
endif()
if(RENDERER_SDL2)
    add_cxx_flag("-DRENDERER_SDL2")
endif()
if(RENDERER_GL)
    add_cxx_flag("-DRENDERER_GL")
endif()


#
# Compilation options
#

if(GCC_GTE_6)
    # Fix warning that happens in util/function.h because GCC cannot statically
    # analyze that the code is safe.
    add_cxx_flag("-Wno-placement-new")
endif()

if(CLANG OR GCC)
    add_c_flag("-std=c99")
endif()

if(GCC_LT_47)
    add_cxx_flag("-std=c++0x")
elseif(CLANG OR GCC)
    add_cxx_flag("-std=c++11")
endif()

if(CLANG OR GCC)
    add_c_flag("-nostdinc")
    add_cxx_flag("-nostdinc -nostdinc++")
endif()

# Disable C++ exceptions
if(CLANG OR GCC)
    add_cxx_flag("-fno-exceptions")
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
    add_cxx_flag("-fno-rtti")
elseif(MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    add_cxx_flag("/GR-")
endif()

# Hide most symbols
if(CLANG OR GCC)
    set(CMAKE_C_FLAGS_RELEASE
        "${CMAKE_C_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_C_FLAGS_MINSIZEREL
        "${CMAKE_C_FLAGS_MINSIZEREL} -fvisibility=hidden")
    add_cxx_flag("-fvisibility-inlines-hidden")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fvisibility=hidden")
endif()

# Don't allow enum values outside enum range
# Warning: Careful! Not valid for C code.
if(CLANG OR GCC_GTE_46)
    add_cxx_flag("-fstrict-enums")
endif()

# Don't allow thread-safe function-local static variables
if(CLANG OR GCC)
    add_cxx_flag("-fno-threadsafe-statics")
endif()

# Enable warnings
if(CLANG OR GCC)
    add_c_flag("-pedantic")
    add_cxx_flag("-pedantic")
    #add_cxx_flag("-Wall")
endif()
if(GCC AND GCC_GTE_46)
    add_c_flag("-Wunused-but-set-variable")
    add_cxx_flag("-Wunused-but-set-variable")
endif()
# On GCC 4.6 this warns about unused constants in int.h
if(GCC AND GCC_GTE_47)
    add_c_flag("-Wunused-variable")
    add_cxx_flag("-Wunused-variable")
endif()
# Disable warnings
if(CLANG_GTE_6)
    add_cxx_flag(-Wno-c++17-compat-mangling)
endif()
# Disable warnings when making old OS X builds since we don't use libstdc++
#if(APPLE AND CLANG)
#    add_cxx_flag("-Wno-stdlibcxx-not-found")
#endif()

# -fno-implicit-templates

# Reduce code size
if(CLANG)
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Oz")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Oz")
endif()

# Enable floating point optimizations
if(CLANG OR GCC)
    add_c_flag("-ffast-math")
    add_cxx_flag("-ffast-math")
elseif(MSVC)
    add_c_flag("/fp:fast")
    add_cxx_flag("/fp:fast")
endif()

# Disable stack canary
if(CLANG OR GCC)
    add_c_flag("-fno-stack-protector")
    add_cxx_flag("-fno-stack-protector")
elseif(MSVC)
    add_c_flag("/GS-")
    add_cxx_flag("/GS-")
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
set(CMAKE_C_FLAGS_DEFAULT "${CMAKE_C_FLAGS_DEFAULT} -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS_DEFAULT} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# Enable RELEASE in anything with optimizations.
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

# Emscripten configuration
if(AV_EM)
    # TODO: Enable these in C?
    add_cxx_flag("-s ALLOW_MEMORY_GROWTH=1")
    #add_cxx_flag("-s ASSERTIONS=0")
    #add_cxx_flag("-s DISABLE_EXCEPTION_THROWING=1")
    add_cxx_flag("-s ENVIRONMENT=web")
    #add_cxx_flag("-s HEADLESS=1")
    #add_cxx_flag("-s INITIAL_MEMORY=134217728")
    #add_cxx_flag("-s LLD_REPORT_UNDEFINED=1")
    #add_cxx_flag("-s PTHREADS_DEBUG=1")
    add_cxx_flag("-s SDL2_IMAGE_FORMATS=[bmp]")
    #add_cxx_flag("-s STRICT=1")
    #add_cxx_flag("-s SUPPORT_LONGJMP=1")
    add_cxx_flag("-s USE_SDL2=1")
    add_cxx_flag("-s USE_SDL_IMAGE=2")
    add_cxx_flag("-s USE_SDL_MIXER=2")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Enable color within compiler's output
if(NOT DEFINED ENV{CI})
    if(NINJA AND CLANG)
        add_c_and_cxx_flag("-fcolor-diagnostics")
    endif()
    if(NINJA AND GCC_GTE_49)
        add_c_and_cxx_flag("-fdiagnostics-color=always")
    endif()
endif()

if(APPLE)
    # Metal is available on 64-bit machines from 10.11+.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")

    # Lowest that has an exported _start symbol on Xcode 12.5.
    #set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")
endif()

if(UNIVERSAL)
    add_c_and_cxx_flag("-arch x86_64 -arch arm64")
endif()

# Share variables with parent
if(IS_SUBPROJECT)
    set(CMAKE_C_FLAGS
        ${CMAKE_C_FLAGS}
        PARENT_SCOPE)

    set(CMAKE_C_FLAGS_DEFAULT
        ${CMAKE_C_FLAGS_DEFAULT}
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

    set(CMAKE_CXX_FLAGS_DEFAULT
        ${CMAKE_CXX_FLAGS_DEFAULT}
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

    set(CMAKE_OSX_DEPLOYMENT_TARGET
        ${CMAKE_OSX_DEPLOYMENT_TARGET}
        PARENT_SCOPE)

    get_property(CAROB_INCLUDE_DIRECTORIES
        DIRECTORY
        PROPERTY INCLUDE_DIRECTORIES)
    set(CAROB_INCLUDE_DIRECTORIES
        ${CAROB_INCLUDE_DIRECTORIES}
        PARENT_SCOPE)
endif()


#
# Linking and externally-managed dependencies
#

if(CMAKE_OSX_SYSROOT)
    add_link_flag("-L${CMAKE_OSX_SYSROOT}/usr/lib")
endif()

if(STATIC_LINK)
    if(CLANG OR GCC)
        add_link_flag("-static")
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
    add_link_flag("-arch x86_64 -arch arm64")
endif()

if(APPLE)
    add_link_flag("--no-standard-libraries -lSystem")
endif()

if(APPLE AND NOT WINDOW_NULL)
    # FIXME: Change to be target-specific
    set(GUI_TYPE MACOSX_BUNDLE)
endif()

if(APPLE AND NOT WINDOW_NULL AND NOT AUDIO_NULL)
    target_link_libraries(carob
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
            target_link_libraries(cutil rt)
        else()
            target_link_libraries(carob rt)
        endif()
    endif()

    if(GLIBC_VERSION)
        if(UTIL_ONLY)
            set(CTHREAD pthread)
            set(CMATH m)
        else()
            target_link_libraries(carob m pthread)
        endif()
    endif()
endif()

if(FREEBSD OR NETBSD)
    if(UTIL_ONLY)
        set(CTHREAD pthread)
        set(CMATH m)
    else()
        target_link_libraries(carob m pthread)
    endif()
endif()
if(AV_EM)
    target_link_libraries(carob pthread)
endif()

if(STATIC_SDL)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${STATIC_SDL}")
    if(APPLE)
        if(WINDOW_SDL2)
            # FIXME: Make these specific to libcarob.
            add_link_flag("-lobjc -Wl,-framework,Carbon -Wl,-framework,Cocoa -Wl,-framework,CoreVideo -Wl,-framework,IOKit -Wl,-weak_framework,QuartzCore")
        endif()
        if(AUDIO_SDL2)
            add_link_flag("-Wl,-framework,AudioToolbox -Wl,-framework,CoreAudio")
        endif()
        if(RENDERER_SDL2)
            add_link_flag("-Wl,-weak_framework,Metal")
        endif()
    endif()
elseif(SDL2)
    if(USE_SDL2_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2 REQUIRED SDL2 sdl2)

        if(SDL2_LIBRARIES STREQUAL "")
            message(SEND_ERROR "pkg-config could not find SDL2")
        endif()
    else()
        set(SDL2_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2 library")
        set(SDL2_LIBRARIES "" CACHE STRING
            "Name of SDL2 library")

        if(SDL2_LIBRARIES STREQUAL "")
            message(SEND_ERROR "When not using pkg-config, please manually set"
                " SDL2 libraries")
        endif()
    endif()

    link_directories(${SDL2_LIBRARY_DIRS})
    target_link_libraries(carob ${SDL2_LIBRARIES})
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

    if(SDL2_mixer_LIBRARIES STREQUAL "")
        message(SEND_ERROR "When not using pkg-config, please manually set"
            " SDL2_mixer libraries")
    endif()

    link_directories(${SDL2_mixer_LIBRARY_DIRS})
    target_link_libraries(carob ${SDL2_mixer_LIBRARIES})
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
#    if(SDL2_image_LIBRARIES STREQUAL "")
#        message(SEND_ERROR "When not using pkg-config, please manually set"
#            " SDL2_image include dirs and libraries")
#    endif()
#
#    target_include_directories(carob PRIVATE ${SDL2_image_INCLUDE_DIRS})
#    link_directories(${SDL2_image_LIBRARY_DIRS})
#    target_link_libraries(carob ${SDL2_image_LIBRARIES})
#endif()

if(CURL)
    target_link_libraries(cutil curl)
endif()

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
    set(CMAKE_EXECUTABLE_SUFFIX
        ${CMAKE_EXECUTABLE_SUFFIX}
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
