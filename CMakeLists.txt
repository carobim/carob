cmake_minimum_required(VERSION 2.8.9...3.19)
project(TsunagariEngine C CXX)
#https://cmake.org/cmake/help/v2.8.9/cmake.html


#
# Options
#

option(AV_NULL "Disable audio and video output")
option(AV_SDL2 "Use SDL2 for audio and video output")
option(AV_EM "Use Emscripten for audio and video output")

if(NOT AV_SDL2 AND NOT AV_EM)
    set(AV_NULL ON)
endif()

option(USE_SDL2_PKGCONFIG "Use pkg-config to find SDL2" ON)

option(BUILD_SHARED_LIBS "Build Tsunagari as a shared library")
option(STATIC_LINK "Build statically linked binaries")


#
# Variables
#

# Whether this is a subproject in a larger project
get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

# Project generator
if(CMAKE_GENERATOR MATCHES Ninja)
    set(NINJA 1)
endif()

# Compiler
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    set(CLANG 1)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(GCC 1)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    set(MSVC 1)
endif()

# OS
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(LINUX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    set(FREEBSD 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES NetBSD)
    set(NETBSD 1)
endif()

if(AV_EM)
    set(WIN32 0)
    set(APPLE 0)
    set(LINUX 1)
    set(FREEBSD 0)
    set(NETBSD 0)
endif()

# Build type
set(IS_DEBUG
    "$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>")
set(IS_RELEASE
    "$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>")


#
# Sources
#

set(HERE ${CMAKE_CURRENT_LIST_DIR})


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

if(AV_NULL)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/null/images.cpp
        ${HERE}/src/av/null/music.cpp
        ${HERE}/src/av/null/sounds.cpp
        ${HERE}/src/av/null/window.cpp
    )
endif()

if(AV_SDL2 OR AV_EM)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/av/sdl2/error.cpp
        ${HERE}/src/av/sdl2/error.h
        ${HERE}/src/av/sdl2/images.cpp
        #${HERE}/src/av/gl/images.cpp
        ${HERE}/src/av/sdl2/music.cpp
        ${HERE}/src/av/sdl2/sounds.cpp
        ${HERE}/src/av/sdl2/window.cpp
        ${HERE}/src/av/sdl2/window.h
    )
endif()

set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
    ${HERE}/src/core/algorithm.h
    ${HERE}/src/core/animation.cpp
    ${HERE}/src/core/animation.h
    ${HERE}/src/core/area.cpp
    ${HERE}/src/core/area.h
    ${HERE}/src/core/area-json.cpp
    ${HERE}/src/core/area-json.h
    ${HERE}/src/core/character.cpp
    ${HERE}/src/core/character.h
    ${HERE}/src/core/client-conf.cpp
    ${HERE}/src/core/client-conf.h
    ${HERE}/src/core/cooldown.cpp
    ${HERE}/src/core/cooldown.h
    ${HERE}/src/core/display-list.cpp
    ${HERE}/src/core/display-list.h
    ${HERE}/src/core/entity.cpp
    ${HERE}/src/core/entity.h
    ${HERE}/src/core/images.h
    ${HERE}/src/core/jsons.cpp
    ${HERE}/src/core/jsons.h
    ${HERE}/src/core/log.cpp
    ${HERE}/src/core/log.h
    ${HERE}/src/core/measure.cpp
    ${HERE}/src/core/measure.h
    ${HERE}/src/core/music.cpp
    ${HERE}/src/core/music-worker.h
    ${HERE}/src/core/music.h
    ${HERE}/src/core/npc.cpp
    ${HERE}/src/core/npc.h
    ${HERE}/src/core/overlay.cpp
    ${HERE}/src/core/overlay.h
    ${HERE}/src/core/player.cpp
    ${HERE}/src/core/player.h
    ${HERE}/src/core/resources.h
    ${HERE}/src/core/sounds.h
    ${HERE}/src/core/tile.cpp
    ${HERE}/src/core/tile.h
    ${HERE}/src/core/tile-grid.cpp
    ${HERE}/src/core/tile-grid.h
    ${HERE}/src/core/vec.cpp
    ${HERE}/src/core/vec.h
    ${HERE}/src/core/viewport.cpp
    ${HERE}/src/core/viewport.h
    ${HERE}/src/core/window.cpp
    ${HERE}/src/core/window.h
    ${HERE}/src/core/world.cpp
    ${HERE}/src/core/world.h
)

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

if (WIN32)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/memmove.cpp

        # On MSVC2013 including this file results in a warning. In MSVC2015+
        # omitting it results in an error.
        ${HERE}/src/os/windows-c.cpp
        
        ${HERE}/src/os/windows-c.h
        ${HERE}/src/os/windows-chrono.cpp
        ${HERE}/src/os/windows-mapped-file.cpp
        ${HERE}/src/os/windows-mapped-file.h
        ${HERE}/src/os/windows.cpp
        ${HERE}/src/os/windows.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/memmove.cpp
        ${HERE}/src/os/windows-c.cpp
        ${HERE}/src/os/windows-c.h
        ${HERE}/src/os/windows-condition-variable.h
        ${HERE}/src/os/windows-mapped-file.cpp
        ${HERE}/src/os/windows-mapped-file.h
        ${HERE}/src/os/windows-mutex.h
        ${HERE}/src/os/windows-thread.h
        ${HERE}/src/os/windows.cpp
        ${HERE}/src/os/windows.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/memmove.cpp
        ${HERE}/src/os/windows-c.cpp
        ${HERE}/src/os/windows-c.h
        ${HERE}/src/os/windows.cpp
        ${HERE}/src/os/windows.h
        ${HERE}/src/os/windows/io.cpp
        ${HERE}/src/os/windows/io.h
    )
elseif(APPLE)
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/mac-c.h
        ${HERE}/src/os/mac-chrono.cpp
        ${HERE}/src/os/mac-gui.mm
        ${HERE}/src/os/mac-gui.h
        ${HERE}/src/os/mac-thread.cpp
        ${HERE}/src/os/mac-thread.h
        ${HERE}/src/os/unix-mapped-file.cpp
        ${HERE}/src/os/unix-mapped-file.h
        ${HERE}/src/os/unix-mutex.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/mac-c.h
        ${HERE}/src/os/mac-thread.cpp
        ${HERE}/src/os/mac-thread.h
        ${HERE}/src/os/unix-condition-variable.h
        ${HERE}/src/os/unix-mapped-file.cpp
        ${HERE}/src/os/unix-mapped-file.h
        ${HERE}/src/os/unix-mutex.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/mac-c.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
    )
else()
    set(TSUNAGARI_SOURCES ${TSUNAGARI_SOURCES}
        ${HERE}/src/os/linux-c.h
        ${HERE}/src/os/unix-chrono.cpp
        ${HERE}/src/os/unix-mapped-file.cpp
        ${HERE}/src/os/unix-mutex.h
        ${HERE}/src/os/unix-thread.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
    )
    set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
        ${HERE}/src/os/linux-c.h
        ${HERE}/src/os/unix-condition-variable.h
        ${HERE}/src/os/unix-mapped-file.cpp
        ${HERE}/src/os/unix-mapped-file.h
        ${HERE}/src/os/unix-mutex.h
        ${HERE}/src/os/unix-thread.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
    )
    set(TEST_SOURCES ${TEST_SOURCES}
        ${HERE}/src/os/linux-c.h
        ${HERE}/src/os/unix.cpp
        ${HERE}/src/os/unix/io.cpp
        ${HERE}/src/os/unix/io.h
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
    ${HERE}/src/util/align.h
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/constexpr.h
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
    ${HERE}/src/util/meta.h
    ${HERE}/src/util/move.h
    ${HERE}/src/util/new.cpp
    ${HERE}/src/util/new.h
    ${HERE}/src/util/noexcept.h
    ${HERE}/src/util/pool.h
    ${HERE}/src/util/random.cpp
    ${HERE}/src/util/random.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
    ${HERE}/src/util/string2.cpp
    ${HERE}/src/util/string2.h
    ${HERE}/src/util/transform.cpp
    ${HERE}/src/util/transform.h
    ${HERE}/src/util/vector.h
)

set(PACK_TOOL_SOURCES ${PACK_TOOL_SOURCES}
    ${HERE}/src/util/align.h
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/constexpr.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/hashtable.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/list.h
    ${HERE}/src/util/meta.h
    ${HERE}/src/util/move.h
    ${HERE}/src/util/new.cpp
    ${HERE}/src/util/new.h
    ${HERE}/src/util/noexcept.h
    ${HERE}/src/util/queue.h
    ${HERE}/src/util/sort.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
    ${HERE}/src/util/vector.h
)

set(TEST_SOURCES ${TEST_SOURCES}
    ${HERE}/src/util/assert.h
    ${HERE}/src/util/fnv.cpp
    ${HERE}/src/util/fnv.h
    ${HERE}/src/util/int.h
    ${HERE}/src/util/io.cpp
    ${HERE}/src/util/io.h
    ${HERE}/src/util/noexcept.h
    ${HERE}/src/util/string-view.cpp
    ${HERE}/src/util/string-view.h
    ${HERE}/src/util/string.cpp
    ${HERE}/src/util/string.h
)

if(MSVC OR ${CMAKE_VERSION} VERSION_LESS 3.1)
    # MSVC 2017 and below, and CMake 3.0 and below cannot vary source files by
    # configuration.
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


#
# Targets
#

add_library(tsunagari ${TSUNAGARI_SOURCES})
add_executable(null-world ${NULL_WORLD_SOURCES})
add_executable(pack-tool ${PACK_TOOL_SOURCES})
add_executable(test ${TEST_SOURCES})

target_link_libraries(null-world tsunagari)

include_directories(src)


#
# Source groups for IDEs
#

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.8.0)
    get_target_property(TSUNAGARI_SOURCES tsunagari SOURCES)
    get_target_property(NULL_WORLD_SOURCES null-world SOURCES)
    get_target_property(PACK_TOOL_SOURCES pack-tool SOURCES)
    get_target_property(TEST_SOURCES test SOURCES)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES ${TSUNAGARI_SOURCES}
              ${NULL_WORLD_SOURCES}
              ${PACK_TOOL_SOURCES}
              ${TEST_SOURCES}
    )
endif()


#
# Compilation options
#

if(GCC)
    # Fix warning that happens in util/function.h because GCC cannot statically
    # analyze that the code is safe.
    if(NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 6.0)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-placement-new")
    endif()
endif()

if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++")
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
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fvisibility=hidden")
endif()

# Don't allow enum values outside enum range
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstrict-enums")
endif()

# Don't allow thread-safe function-local static variables
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
endif()

# Enable warnings
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wunused-variable")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if(GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-but-set-variable")
endif()

# -fno-implicit-templates

# Reduce code size
if(CLANG)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Oz")
endif()

# Enable floating point optimizations
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")
endif()

# Disable stack canary
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS-")
endif()

# Disable frame pointer
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -fomit-frame-pointer")
endif()

# Optimize at link time
if(CLANG OR GCC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -flto")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto")
endif()

# Enable NDEBUG anything not a debug build.
if(${CMAKE_VERSION} VERSION_LESS 2.8.10)
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
else()
    target_compile_definitions(tsunagari PUBLIC $<$<NOT:${IS_DEBUG}>:NDEBUG>)
    target_compile_definitions(pack-tool PRIVATE $<$<NOT:${IS_DEBUG}>:NDEBUG>)
endif()

# Enable RELEASE in anything with optimizations.
if(${CMAKE_VERSION} VERSION_LESS 2.8.10)
    set(CMAKE_CXX_FLAGS_MINSIZEREL
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -DRELEASE")
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        "${CMAKE_CXX_FLAGS_MINSIZEREL} -DRELEASE")
else()
    target_compile_definitions(tsunagari PUBLIC $<${IS_RELEASE}:RELEASE>)
    target_compile_definitions(pack-tool PRIVATE $<${IS_RELEASE}:RELEASE>)
endif()

# Emscripten configuration
if(AV_EM)
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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
    endif()
    if(NINJA AND GCC AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    endif()
endif()

if(AV_NULL)
    if(${CMAKE_VERSION} VERSION_LESS 2.8.10)
        add_definitions(-DAV_NULL)
    else()
        target_compile_definitions(tsunagari PUBLIC AV_NULL)
    endif()
endif()

# Share variables with parent
if(IS_SUBPROJECT)
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

if(STATIC_LINK)
    if(CLANG OR GCC)
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static")
    else()
        message(WARNING "Not sure how to statically link for this compiler")
    endif()
endif()

# Optimize at link time
if(CLANG OR GCC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto=full")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
        "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -flto=full")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
        "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -flto=thin")
endif()

# Do not link to libstdc++ or libc++
if(APPLE OR LINUX OR FREEBSD OR NETBSD)
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
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
endif()

if(APPLE)
    # FIXME: Change to be target-specific
    set(GUI_TYPE MACOSX_BUNDLE)

    target_link_libraries(tsunagari
        /System/Library/Frameworks/Cocoa.framework
        /System/Library/Frameworks/Foundation.framework
    )

    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} --no-standard-libraries -lSystem")
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
        target_link_libraries(tsunagari rt)
    endif()

    if(GLIBC_VERSION)
        target_link_libraries(tsunagari m pthread)
    endif()
endif()

if(FREEBSD OR NETBSD)
    target_link_libraries(tsunagari PUBLIC  m pthread)
endif()
if(AV_EM)
    target_link_libraries(tsunagari PUBLIC  pthread)
endif()

if(AV_SDL2)
    if(USE_SDL2_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2       REQUIRED SDL2 sdl2)
        pkg_search_module(SDL2_image REQUIRED SDL2_image)
        pkg_search_module(SDL2_mixer REQUIRED SDL2_mixer)
    else()
        set(SDL2_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2 headers")
        set(SDL2_image_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2_image headers")
        set(SDL2_mixer_INCLUDE_DIRS "" CACHE PATH
            "Directory with SDL2_mixer headers")

        set(SDL2_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2 library")
        set(SDL2_image_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2_image library")
        set(SDL2_mixer_LIBRARY_DIRS "" CACHE PATH
            "Directory with SDL2_mixer library")

        set(SDL2_LIBRARIES "" CACHE STRING
            "Name of SDL2 library")
        set(SDL2_image_LIBRARIES "" CACHE STRING
            "Name with SDL2_image library")
        set(SDL2_mixer_LIBRARIES "" CACHE STRING
            "Name with SDL2_mixer library")
    endif()

    if("${SDL2_LIBRARIES}" STREQUAL "")
        message(SEND_ERROR "When not using pkg-config, please manually set"
            " SDL2, SDL2_image, and SDL2_mixer include dirs and libraries")
    endif()

    target_include_directories(tsunagari
        PRIVATE ${SDL2_INCLUDE_DIRS}
                ${SDL2_image_INCLUDE_DIRS}
                ${SDL2_mixer_INCLUDE_DIRS}
    )

    link_directories(
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_image_LIBRARY_DIRS}
        ${SDL2_mixer_LIBRARY_DIRS}
    )

    target_link_libraries(tsunagari
        ${SDL2_LIBRARIES}
        ${SDL2_image_LIBRARIES}
        ${SDL2_mixer_LIBRARIES}
    )
endif()


#
# Policies
#

# Do not turn on -rdynamic. Also solved with CMP0065 in CMake 3.4
# FIXME: Apply it only to local targets, or possibly just try to
#        remove -rdynamic from these.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
