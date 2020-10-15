cmake_minimum_required(VERSION 3.8)
project(TsunagariEngine LANGUAGES CXX)


#
# Options
#

option(AV_NULL "Disable audio and video output")
option(AV_SDL2 "Use SDL2 for audio and video output")
option(AV_GOSU "Use Gosu for audio and video output")
option(AV_EM "Use Emscripten for audio and video output")

if(NOT AV_SDL2 AND NOT AV_GOSU AND NOT AV_EM)
    set(AV_NULL ON)
endif()

option(USE_SDL2_PKGCONFIG "Use pkg-config to find SDL2" ON)
option(USE_GOSU_CONFIG_MODULE "Use the GosuConfig.cmake module to find SDL2" ON)

option(BUILD_SHARED_LIBS "Build Tsunagari as a shared library")


#
# Variables
#

# Whether this is a subproject in a larger project.
get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)
if("${SUBPROJECT}" STREQUAL "")
    set(SUBPROJECT 1)
else()
    set(SUBPROJECT 1)
endif()

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
set(IS_RELEASE
    "$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>")


#
# Targets
#

set(HERE ${CMAKE_CURRENT_LIST_DIR})

add_library(tsunagari ${HERE}/src/main.cpp)
add_executable(null-world ${HERE}/src/null-world.cpp)
add_executable(pack-tool ${HERE}/src/pack/main.cpp)

target_link_libraries(null-world tsunagari)


#
# Sources in deps
#

target_include_directories(tsunagari
    PUBLIC deps/rapidjson/include
    PUBLIC deps/pjson
)

#
# Sources in src
#

target_include_directories(tsunagari PUBLIC src)
target_include_directories(null-world PUBLIC src)
target_include_directories(pack-tool PRIVATE src)

target_sources(tsunagari
    PUBLIC  ${HERE}/src/config.h
)

if(AV_NULL)
    target_sources(tsunagari
        PRIVATE ${HERE}/src/av/null/images.cpp
                ${HERE}/src/av/null/music.cpp
                ${HERE}/src/av/null/sounds.cpp
                ${HERE}/src/av/null/window.cpp
    )
endif()

if(AV_SDL2 OR AV_EM)
    target_sources(tsunagari
        PRIVATE ${HERE}/src/av/sdl2/error.cpp
                ${HERE}/src/av/sdl2/error.h
                ${HERE}/src/av/sdl2/images.cpp
                ${HERE}/src/av/sdl2/music.cpp
                ${HERE}/src/av/sdl2/sounds.cpp
                ${HERE}/src/av/sdl2/window.cpp
                ${HERE}/src/av/sdl2/window.h
    )
endif()

if(AV_GOSU)
    target_sources(tsunagari
        PRIVATE ${HERE}/src/av/gosu/cbuffer.cpp
                ${HERE}/src/av/gosu/cbuffer.h
                ${HERE}/src/av/gosu/gosu.h
                ${HERE}/src/av/gosu/images.cpp
                ${HERE}/src/av/null/music.cpp
                ${HERE}/src/av/null/sounds.cpp
                ${HERE}/src/av/gosu/window.cpp
                ${HERE}/src/av/gosu/window.h
    )
endif()

target_sources(tsunagari
    PUBLIC ${HERE}/src/cache/rc-cache-impl.h
    PUBLIC ${HERE}/src/cache/rc-cache.h
    PUBLIC ${HERE}/src/cache/rc-reader-cache.h
)

target_sources(tsunagari
    PUBLIC  ${HERE}/src/core/algorithm.h
    PRIVATE ${HERE}/src/core/animation.cpp
    PUBLIC  ${HERE}/src/core/animation.h
    PRIVATE ${HERE}/src/core/area.cpp
    PUBLIC  ${HERE}/src/core/area.h
    PRIVATE ${HERE}/src/core/area-json.cpp
    PUBLIC  ${HERE}/src/core/area-json.h
    PRIVATE ${HERE}/src/core/character.cpp
    PUBLIC  ${HERE}/src/core/character.h
    PRIVATE ${HERE}/src/core/client-conf.cpp
    PUBLIC  ${HERE}/src/core/client-conf.h
    PRIVATE ${HERE}/src/core/cooldown.cpp
    PUBLIC  ${HERE}/src/core/cooldown.h
    PRIVATE ${HERE}/src/core/display-list.cpp
    PRIVATE ${HERE}/src/core/display-list.h
    PRIVATE ${HERE}/src/core/entity.cpp
    PUBLIC  ${HERE}/src/core/entity.h
    PUBLIC  ${HERE}/src/core/images.h
    PRIVATE ${HERE}/src/core/jsons-rapidjson.cpp
    PUBLIC  ${HERE}/src/core/jsons-rapidjson.h
    PRIVATE ${HERE}/src/core/log.cpp
    PUBLIC  ${HERE}/src/core/log.h
    PRIVATE ${HERE}/src/core/measure.cpp
    PUBLIC  ${HERE}/src/core/measure.h
    PRIVATE ${HERE}/src/core/music.cpp
    PUBLIC  ${HERE}/src/core/music-worker.h
    PUBLIC  ${HERE}/src/core/music.h
    PRIVATE ${HERE}/src/core/npc.cpp
    PUBLIC  ${HERE}/src/core/npc.h
    PRIVATE ${HERE}/src/core/overlay.cpp
    PUBLIC  ${HERE}/src/core/overlay.h
    PRIVATE ${HERE}/src/core/player.cpp
    PUBLIC  ${HERE}/src/core/player.h
    PUBLIC  ${HERE}/src/core/resources.h
    PUBLIC  ${HERE}/src/core/sounds.h
    PRIVATE ${HERE}/src/core/tile.cpp
    PUBLIC  ${HERE}/src/core/tile.h
    PRIVATE ${HERE}/src/core/tile-grid.cpp
    PUBLIC  ${HERE}/src/core/tile-grid.h
    PRIVATE ${HERE}/src/core/viewport.cpp
    PUBLIC  ${HERE}/src/core/viewport.h
    PRIVATE ${HERE}/src/core/window.cpp
    PUBLIC  ${HERE}/src/core/window.h
    PRIVATE ${HERE}/src/core/world.cpp
    PUBLIC  ${HERE}/src/core/world.h
)

target_sources(tsunagari
    PRIVATE ${HERE}/src/data/data-area.cpp
    PUBLIC  ${HERE}/src/data/data-area.h
    PUBLIC  ${HERE}/src/data/data-world.h
    PRIVATE ${HERE}/src/data/inprogress.cpp
    PUBLIC  ${HERE}/src/data/inprogress.h
    PUBLIC  ${HERE}/src/data/inprogress-sound.h
    PUBLIC  ${HERE}/src/data/inprogress-timer.h
)

target_sources(tsunagari
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/document.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/reader.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/allocators.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/rapidjson.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/stream.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/encodings.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/stack.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/swap.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/strtod.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/ieee754.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/biginteger.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/diyfp.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/pow10.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/meta.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/internal/strfunc.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/memorystream.h
    PRIVATE ${HERE}/deps/rapidjson/include/rapidjson/encodedstream.h
    PRIVATE ${HERE}/deps/pjson/pjson.h
)

target_sources(tsunagari
    PUBLIC ${HERE}/src/os/c.h
    PUBLIC ${HERE}/src/os/chrono.h
    PUBLIC ${HERE}/src/os/mapped-file.h
    PUBLIC ${HERE}/src/os/mutex.h
    PUBLIC ${HERE}/src/os/thread.h
    PUBLIC ${HERE}/src/os/os.h
)
target_sources(pack-tool
    PRIVATE ${HERE}/src/os/c.h
    PRIVATE ${HERE}/src/os/condition-variable.h
    PRIVATE ${HERE}/src/os/mapped-file.h
    PRIVATE ${HERE}/src/os/mutex.h
    PRIVATE ${HERE}/src/os/os.h
    PRIVATE ${HERE}/src/os/thread.h
)

if (WIN32)
    target_sources(tsunagari
        PRIVATE ${HERE}/src/os/memmove.cpp
        PRIVATE ${HERE}/src/os/windows-c.cpp
        PUBLIC  ${HERE}/src/os/windows-c.h
        PRIVATE ${HERE}/src/os/windows-chrono.cpp
        PRIVATE ${HERE}/src/os/windows-mapped-file.cpp
        PUBLIC  ${HERE}/src/os/windows-mapped-file.h
        PRIVATE ${HERE}/src/os/windows.cpp
        PUBLIC  ${HERE}/src/os/windows.h
    )
    target_sources(pack-tool
        PRIVATE ${HERE}/src/os/memmove.cpp
        PRIVATE ${HERE}/src/os/windows-c.cpp
        PRIVATE ${HERE}/src/os/windows-c.h
        PRIVATE ${HERE}/src/os/windows-condition-variable.h
        PRIVATE ${HERE}/src/os/windows-mapped-file.cpp
        PRIVATE ${HERE}/src/os/windows-mapped-file.h
        PRIVATE ${HERE}/src/os/windows-mutex.h
        PRIVATE ${HERE}/src/os/windows-thread.h
        PRIVATE ${HERE}/src/os/windows.cpp
        PRIVATE ${HERE}/src/os/windows.h
    )
elseif(APPLE)
    target_sources(tsunagari
        PUBLIC  ${HERE}/src/os/mac-c.h
        PRIVATE ${HERE}/src/os/mac-chrono.cpp
        PRIVATE ${HERE}/src/os/mac-gui.mm
        PUBLIC  ${HERE}/src/os/mac-gui.h
        PRIVATE ${HERE}/src/os/mac-thread.cpp
        PUBLIC  ${HERE}/src/os/mac-thread.h
        PRIVATE ${HERE}/src/os/unix-mapped-file.cpp
        PUBLIC  ${HERE}/src/os/unix-mapped-file.h
        PUBLIC  ${HERE}/src/os/unix-mutex.h
        PRIVATE ${HERE}/src/os/unix.cpp
    )
    target_sources(pack-tool
        PRIVATE ${HERE}/src/os/mac-c.h
        PRIVATE ${HERE}/src/os/mac-thread.cpp
        PRIVATE ${HERE}/src/os/mac-thread.h
        PRIVATE ${HERE}/src/os/unix-condition-variable.h
        PRIVATE ${HERE}/src/os/unix-mapped-file.cpp
        PRIVATE ${HERE}/src/os/unix-mapped-file.h
        PRIVATE ${HERE}/src/os/unix-mutex.h
        PRIVATE ${HERE}/src/os/unix.cpp
    )
else()
    target_sources(tsunagari
        PUBLIC  ${HERE}/src/os/linux-c.h
        PRIVATE ${HERE}/src/os/unix-chrono.cpp
        PRIVATE ${HERE}/src/os/unix-mapped-file.cpp
        PUBLIC  ${HERE}/src/os/unix-mutex.h
        PUBLIC  ${HERE}/src/os/unix-thread.h
        PRIVATE ${HERE}/src/os/unix.cpp
    )
    target_sources(pack-tool
        PRIVATE ${HERE}/src/os/linux-c.h
        PRIVATE ${HERE}/src/os/unix-condition-variable.h
        PRIVATE ${HERE}/src/os/unix-mapped-file.cpp
        PRIVATE ${HERE}/src/os/unix-mapped-file.h
        PRIVATE ${HERE}/src/os/unix-mutex.h
        PRIVATE ${HERE}/src/os/unix-thread.h
        PRIVATE ${HERE}/src/os/unix.cpp
    )
endif()

target_sources(tsunagari
    PRIVATE ${HERE}/src/pack/file-type.cpp
    PUBLIC  ${HERE}/src/pack/file-type.h
    PRIVATE ${HERE}/src/pack/pack-reader.cpp
    PUBLIC  ${HERE}/src/pack/pack-reader.h
)

target_sources(pack-tool
    PRIVATE ${HERE}/src/pack/file-type.cpp
    PRIVATE ${HERE}/src/pack/file-type.h
    PRIVATE ${HERE}/src/pack/pack-reader.cpp
    PRIVATE ${HERE}/src/pack/pack-reader.h
    PRIVATE ${HERE}/src/pack/pack-writer.cpp
    PRIVATE ${HERE}/src/pack/pack-writer.h
    PRIVATE ${HERE}/src/pack/ui-log.cpp
    PRIVATE ${HERE}/src/pack/ui.h
    PRIVATE ${HERE}/src/pack/walker.cpp
    PRIVATE ${HERE}/src/pack/walker.h
)

target_sources(tsunagari
    PRIVATE ${HERE}/src/resources/pack.cpp
)

target_sources(tsunagari
    PUBLIC  ${HERE}/src/util/algorithm.h
    PUBLIC  ${HERE}/src/util/align.h
    PUBLIC  ${HERE}/src/util/arc.h
    PRIVATE ${HERE}/src/util/assert.cpp
    PUBLIC  ${HERE}/src/util/assert.h
    PRIVATE ${HERE}/src/util/bitrecord.cpp
    PUBLIC  ${HERE}/src/util/bitrecord.h
    PUBLIC  ${HERE}/src/util/constexpr.h
    PRIVATE ${HERE}/src/util/fnv.cpp
    PUBLIC  ${HERE}/src/util/fnv.h
    PUBLIC  ${HERE}/src/util/function.h
    PRIVATE ${HERE}/src/util/hash.cpp
    PUBLIC  ${HERE}/src/util/hash.h
    PUBLIC  ${HERE}/src/util/hashtable.h
    PUBLIC  ${HERE}/src/util/int.h
    PRIVATE ${HERE}/src/util/jobs.cpp
    PUBLIC  ${HERE}/src/util/jobs.h
    PUBLIC  ${HERE}/src/util/likely.h
    PUBLIC  ${HERE}/src/util/list.h
    PUBLIC  ${HERE}/src/util/markable.h
    PUBLIC  ${HERE}/src/util/math2.h
    PUBLIC  ${HERE}/src/util/meta.h
    PUBLIC  ${HERE}/src/util/move.h
    PUBLIC  ${HERE}/src/util/new.h
    PUBLIC  ${HERE}/src/util/noexcept.h
    PUBLIC  ${HERE}/src/util/optional.h
    PUBLIC  ${HERE}/src/util/pool.h
    PRIVATE ${HERE}/src/util/random.cpp
    PUBLIC  ${HERE}/src/util/random.h
    PUBLIC  ${HERE}/src/util/rc.h
    PRIVATE ${HERE}/src/util/string-view.cpp
    PUBLIC  ${HERE}/src/util/string-view.h
    PRIVATE ${HERE}/src/util/string.cpp
    PUBLIC  ${HERE}/src/util/string.h
    PRIVATE ${HERE}/src/util/string2.cpp
    PUBLIC  ${HERE}/src/util/string2.h
    PRIVATE ${HERE}/src/util/transform.cpp
    PUBLIC  ${HERE}/src/util/transform.h
    PUBLIC  ${HERE}/src/util/unique.h
    PUBLIC  ${HERE}/src/util/vector.h
)

target_sources(pack-tool
    PRIVATE ${HERE}/src/util/algorithm.h
    PRIVATE ${HERE}/src/util/align.h
    PRIVATE ${HERE}/src/util/assert.cpp
    PRIVATE ${HERE}/src/util/assert.h
    PRIVATE ${HERE}/src/util/constexpr.h
    PRIVATE ${HERE}/src/util/fnv.cpp
    PRIVATE ${HERE}/src/util/fnv.h
    PRIVATE ${HERE}/src/util/function.h
    PRIVATE ${HERE}/src/util/hashtable.h
    PRIVATE ${HERE}/src/util/int.h
    PRIVATE ${HERE}/src/util/jobs.cpp
    PRIVATE ${HERE}/src/util/jobs.h
    PRIVATE ${HERE}/src/util/list.h
    PRIVATE ${HERE}/src/util/meta.h
    PRIVATE ${HERE}/src/util/move.h
    PRIVATE ${HERE}/src/util/new.h
    PRIVATE ${HERE}/src/util/noexcept.h
    PRIVATE ${HERE}/src/util/optional.h
    PRIVATE ${HERE}/src/util/sort.h
    PRIVATE ${HERE}/src/util/string-view.cpp
    PRIVATE ${HERE}/src/util/string-view.h
    PRIVATE ${HERE}/src/util/string.cpp
    PRIVATE ${HERE}/src/util/string.h
    PRIVATE ${HERE}/src/util/vector.h
)


#
# Source groups for IDEs
#

get_target_property(TSUNAGARI_SOURCES tsunagari SOURCES)
get_target_property(NULL_WORLD_SOURCES null-world SOURCES)
get_target_property(PACK_TOOL_SOURCES pack-tool SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${TSUNAGARI_SOURCES}
          ${NULL_WORLD_SOURCES}
          ${PACK_TOOL_SOURCES}
)


#
# Compilation options
#

target_compile_features(tsunagari PUBLIC cxx_std_14)
target_compile_features(null-world PUBLIC cxx_std_14)
target_compile_features(pack-tool PUBLIC cxx_std_14)
set_target_properties(tsunagari PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(null-world PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(pack-tool PROPERTIES CXX_EXTENSIONS OFF)

# Disable C++ exceptions
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-exceptions")
elseif(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

# Disable C++ run-time type information
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-rtti")
elseif(MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(APPEND CMAKE_CXX_FLAGS " /GR-")
endif()

# Hide most symbols
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -fvisibility=hidden")
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -fvisibility=hidden")
endif()

# Reduce code size
if(CLANG)
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -Oz")
endif()

# Enable floating point optimizations
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -ffast-math")
elseif(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /fp:fast")
endif()

# Disable stack canary
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fno-stack-protector")
elseif(MSVC)
    string(APPEND CMAKE_CXX_FLAGS " /GS-")
endif()

# Optimize at link time
if(CLANG OR GCC)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto")
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -flto")
    string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -flto=thin")
endif()

target_compile_definitions(tsunagari
    PUBLIC $<$<BOOL:${IS_RELEASE}>:NDEBUG>
)
target_compile_definitions(pack-tool
    PRIVATE $<$<BOOL:${IS_RELEASE}>:NDEBUG>
)

# Emscripten configuration
if(AV_EM)
    string(APPEND CMAKE_CXX_FLAGS " -s ALLOW_MEMORY_GROWTH=1")
    #string(APPEND CMAKE_CXX_FLAGS " -s ASSERTIONS=0")
    #string(APPEND CMAKE_CXX_FLAGS " -s DISABLE_EXCEPTION_THROWING=1")
    string(APPEND CMAKE_CXX_FLAGS " -s ENVIRONMENT=web")
    #string(APPEND CMAKE_CXX_FLAGS " -s HEADLESS=1")
    #string(APPEND CMAKE_CXX_FLAGS " -s INITIAL_MEMORY=134217728")
    #string(APPEND CMAKE_CXX_FLAGS " -s LLD_REPORT_UNDEFINED=1")
    #string(APPEND CMAKE_CXX_FLAGS " -s PTHREADS_DEBUG=1")
    string(APPEND CMAKE_CXX_FLAGS " -s SDL2_IMAGE_FORMATS=[bmp]")
    #string(APPEND CMAKE_CXX_FLAGS " -s STRICT=1")
    #string(APPEND CMAKE_CXX_FLAGS " -s SUPPORT_LONGJMP=0")
    string(APPEND CMAKE_CXX_FLAGS " -s USE_SDL=2")
    string(APPEND CMAKE_CXX_FLAGS " -s USE_SDL_IMAGE=2")
    string(APPEND CMAKE_CXX_FLAGS " -s USE_SDL_MIXER=2")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Enable color within compiler's output
if(NINJA AND CLANG)
    string(APPEND CMAKE_CXX_FLAGS " -fcolor-diagnostics")
endif()
if(NINJA AND GCC)
    string(APPEND CMAKE_CXX_FLAGS " -fdiagnostics-color=always")
endif()

# Share variables with parent.
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
endif()


#
# Linking and externally-managed dependencies
#

# Optimize at link time
if(CLANG OR GCC)
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto=full")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_MINSIZEREL " -flto=full")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto=thin")
endif()

# Do not link to libc++
#if(APPLE)
#   string(APPEND CMAKE_EXE_LINKER_FLAGS " -nostdlib -lSystem")
#endif()

# Strip symbols
if(APPLE AND CLANG)
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -Wl,-x -Wl,-dead_strip")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_MINSIZEREL " -Wl,-x -Wl,-dead_strip")
endif()

# Share variables with parent.
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
    # FIXME: Change to be target-specific.
    set(GUI_TYPE MACOSX_BUNDLE)

    target_link_libraries(tsunagari
        PUBLIC /System/Library/Frameworks/Cocoa.framework
               /System/Library/Frameworks/Foundation.framework
    )
endif()

if(LINUX AND NOT AV_EM)
    # Check if we are compiling against glibc, which needs libpthread, or
    # against musl, which doesn't.
    find_package(Threads)
    if(Threads_FOUND)
        target_link_libraries(tsunagari PUBLIC  Threads::Threads)
        target_link_libraries(pack-tool PRIVATE Threads::Threads)
    endif()
endif()
if(FREEBSD OR NETBSD)
    target_link_libraries(tsunagari PUBLIC  pthread)
    target_link_libraries(pack-tool PRIVATE pthread)
endif()
if(AV_EM)
    target_link_libraries(tsunagari PUBLIC  pthread)
    target_link_libraries(pack-tool PRIVATE pthread)
endif()

if(AV_SDL2)
    if(USE_SDL2_PKGCONFIG)
        find_package(PkgConfig REQUIRED)
        pkg_search_module(SDL2 REQUIRED SDL2 sdl2)
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

    link_directories(tsunagari
        PUBLIC ${SDL2_LIBRARY_DIRS}
               ${SDL2_image_LIBRARY_DIRS}
               ${SDL2_mixer_LIBRARY_DIRS}
    )

    target_link_libraries(tsunagari
        PUBLIC ${SDL2_LIBRARIES}
               ${SDL2_image_LIBRARIES}
               ${SDL2_mixer_LIBRARIES}
    )
endif()

if(AV_GOSU)
    if(USE_GOSU_CONFIG_MODULE)
        find_package(Gosu REQUIRED)
    else()
        set(GOSU_INCLUDE_DIRS "" CACHE PATH
            "Directory with Gosu headers")

        set(GOSU_LIBRARY_DIRS "" CACHE PATH
            "Directory with Gosu library")

        set(GOSU_LIBRARIES "" CACHE STRING
            "Name of Gosu library")

        set(GOSU_DEPENDENCIES "" CACHE STRING
            "Names of Gosu dependency libraries")
    endif()

    if("${GOSU_LIBRARIES}" STREQUAL "")
        message(SEND_ERROR "When not using GosuConfig.cmake, please"
            " manually set Gosu include dirs and libraries")
    endif()

    target_include_directories(tsunagari PRIVATE ${GOSU_INCLUDE_DIRS})
    link_directories(tsunagari PUBLIC ${GOSU_LIBRARY_DIRS})
    target_link_libraries(tsunagari
        PUBLIC ${GOSU_LIBRARIES}
               ${GOSU_DEPENDENCIES}
    )
endif()
