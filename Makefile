# Requirements are still not final... this may change in the future

CC 		?= gcc
DB 		?= gdb
CFLAGS 	?= -Wall -Werror
TARGET 	?= executable
LINKER 	= -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_ttf -lm

default: build
all: build_debug

build:
	$(CC) src/*.c $(LINKER) $(CFLAGS) -o $(TARGET)

build_debug:
	$(CC) src/*.c $(LINKER) $(CFLAGS) -g -o $(TARGET)
	
build_release:
	$(CC) src/*.c $(LINKER) $(CFLAGS) -O3 -o $(TARGET)

clean:
	rm $(TARGET)
