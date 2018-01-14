# Requirements are still not final... this may change in the future

CC = gcc
WINCC = winegcc
DB = gdb
TARGET = executable
WINTARGET = executable.exe
LINKER = -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_ttf -lm -Wall -Werror
default: build
all: build_debug

build:
	$(CC) src/*.c $(LINKER) -o $(TARGET)

build_debug:
	$(CC) src/*.c $(LINKER) -g -o $(TARGET)
	
build_win:
	$(WINCC) src/*.c $(LINKER) -g -o $(WINTARGET)

build_release:
	$(CC) src/*.c $(LINKER) -O3 -o $(TARGET)

clean:
	rm $(TARGET)
