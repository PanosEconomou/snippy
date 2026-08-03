cmake_minimum_required(VERSION 3.23)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(snippy C)
add_executable(snippy)

# Link external libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV REQUIRED IMPORTED_TARGET libevdev      )
pkg_check_modules(UDEV  REQUIRED IMPORTED_TARGET libudev       )
pkg_check_modules(XKB   REQUIRED IMPORTED_TARGET xkbcommon     )
pkg_check_modules(WLC   REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(LUA   REQUIRED IMPORTED_TARGET luajit        )
target_link_libraries(snippy 
    PRIVATE 
        PkgConfig::EVDEV
        PkgConfig::UDEV
        PkgConfig::XKB
        PkgConfig::WLC
        PkgConfig::LUA
)

# My own files
target_sources(snippy
    PRIVATE
        src/main.c
        src/keyboard.c
        src/decoder.c
        src/ring.c
        src/trie.c
        src/config.c
        src/snippet.c
    PRIVATE
        FILE_SET HEADERS
        BASE_DIRS 
            include
        FILES 
            include/keyboard.h
            include/decoder.h
            include/ring.h
            include/trie.h
            include/config.h
            include/snippet.h
)
