cmake_minimum_required(VERSION 3.0)
project(skiciranje)

set(CMAKE_BUILD_TYPE Debug)
set(skiciranje_VERSION_MAJOR 0)
set(skiciranje_VERSION_MINOR 1)
set(skiciranje_VERSION_PATCH 0)
set(skiciranje_VERSION_TWEAK 0)

set(skiciranje_VERSION "${skiciranje_VERSION_MAJOR}.${skiciranje_VERSION_MINOR}.${skiciranje_VERSION_PATCH}.${skiciranje_VERSION_TWEAK}")

add_definitions(-DPACKAGE="skiciranje" -DVERSION="${skiciranje_VERSION}")

# Include SDL2 and other external libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDLTTF REQUIRED SDL2_ttf)
pkg_check_modules(SDLGFX REQUIRED SDL2_gfx)
#pkg_check_modules(SDLIMG REQUIRED SDL2_image)
#pkg_check_modules(SDLMIX REQUIRED SDL2_mixer)
#pkg_check_modules(INIPARSER REQUIRED iniparser)
include_directories(${SDL2_INCLUDE_DIRS} ${SDLTTF_INCLUDE_DIRS} ${SDLGFX_INCLUDE_DIRS} )
link_directories(${SDL2_LIBRARY_DIRS} ${SDLTTF_LIBRARY_DIRS} ${SDLGFX_LIBRARY_DIRS} )

# Tinyfiledialogs
add_library(tinyfiledialogs libs/tinyfiledialogs/tinyfiledialogs.c)

find_library(MATH_LIB m)
if(NOT MATH_LIB)
  message(FATAL_ERROR "math library not found")
endif()

add_executable(
	skiciranje WIN32
	src/clean.c
	src/editor.c
	src/functions.c
	src/init.c
	src/main.c
	src/menu.c
	src/shared.c
	src/snap.c
	src/tutorial.c
)

add_definitions(${SDL2_CFLAGS_OTHER} ${SDLTTF_CFLAGS_OTHER} ${SDLGFX_CFLAGS_OTHER})
target_link_libraries(skiciranje ${SDL2_LIBRARIES} ${SDLTTF_LIBRARIES} ${SDLGFX_LIBRARIES} m tinyfiledialogs)
