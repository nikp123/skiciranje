# You will need fontforge, some kind of shell interpreter, 
# a fully featured C compiler and SDL2, SDL2_ttf for now.

CC = gcc
DB = gdb
WINCC = wine ~/.wine/drive_c/Program\ Files/tcc/tcc.exe
TARGET = executable
TARGET_WIN = executable.exe
LINKER = -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_ttf -lm
WINLINKER = -ldirent -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_ttf 
default: build
all: build_debug

build:
	$(CC) src/*.c $(LINKER) -o $(TARGET)

build_win:
	$(WINCC) src/*.c $(WINLINKER) -o $(TARGET_WIN)

build_debug:
	$(CC) src/*.c $(LINKER) -g -o $(TARGET)
	
clean:
	rm $(TARGET) $(TARGET_WIN)
