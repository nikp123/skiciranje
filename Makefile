# Requirements are still not final... this may change in the future

CC = gcc
DB = gdb
TARGET = executable
LINKER = -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_ttf -lm -Wall -Werror
default: build
all: build_debug

build:
	$(CC) src/*.c $(LINKER) -o $(TARGET)

build_debug:
	$(CC) src/*.c $(LINKER) -g -o $(TARGET)
	
clean:
	rm $(TARGET)
