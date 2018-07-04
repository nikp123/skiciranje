cmake_minimum_required(VERSION 3.0)
project(skiciranje)

set(CMAKE_BUILD_TYPE Debug)

# Include SDL2 and other external libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDLTTF REQUIRED SDL2_ttf)
#pkg_check_modules(SDLIMG REQUIRED SDL2_image)
#pkg_check_modules(SDLMIX REQUIRED SDL2_mixer)
#pkg_check_modules(INIPARSER REQUIRED iniparser)
include_directories(${SDL2_INCLUDE_DIRS} ${SDLTTF_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS} ${SDLTTF_LIBRARY_DIRS})

# Tinyfiledialogs
add_library(tinyfiledialogs libs/tinyfiledialogs/tinyfiledialogs.c)

find_library(MATH_LIB m)
if(NOT MATH_LIB)
  message(FATAL_ERROR "math library not found")
endif()

add_executable(
	skiciranje
	src/clean.c
	src/clean.h
	src/editor.c
	src/editor.h
	src/functions.c
	src/functions.h
	src/init.c
	src/init.h
	src/main.c
	src/menu.c
	src/menu.h
	src/shared.c
	src/shared.h
	src/tutorial.c
	src/tutorial.h
)

add_definitions(${SDL2_CFLAGS_OTHER} ${SDLTTF_CFLAGS_OTHER})
target_link_libraries(skiciranje ${SDL2_LIBRARIES} ${SDLTTF_LIBRARIES} m tinyfiledialogs)
