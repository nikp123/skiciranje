// Most of the configurations and variables are to be stored here and nowhere else, got it!?
// Cool.

#define SDL_MAIN_HANDLED            // Fixes SDL2's broken crap with TCC

// add bools to C
#define false 0
#define true 1
#define FALSE 0
#define TRUE 1

// forced variables
#define SDL_USE_FLAGS       SDL_INIT_EVENTS&SDL_INIT_VIDEO&SDL_INIT_TIMER&SDL_INIT_AUDIO
#define RENDERER_FLAGS      SDL_RENDERER_SOFTWARE&SDL_RENDERER_PRESENTVSYNC
#define WINDOW_FLAGS        SDL_WINDOW_RESIZABLE
#define WINDOW_TITLE        "Test program"
#define WINDOW_WIDTH        640
#define WINDOW_HEIGHT       480
#define CLEAR_COLOR         0xff222222
#define TITLE_FONT_COLOR    0xffffffff
#define MENU_FONT_COLOR     0xffaaaaaa
#define OPTION_FONT_COLOR   0xffbbbbbb
#define DETAIL_FONT_COLOR   0xff666666
#define FRAMERATE           60

// libraries and functions that we are going to borrow for our program
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// We must deal with cross platform crap again
#ifdef __linux__
    #include <unistd.h>
#endif
#ifdef __WIN32__
    #include <io.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

extern int argumentCount;
extern char **arguments;

extern char **textLine;
extern int textLines;
extern int glow;

extern SDL_Window *win;
extern SDL_Surface *swin;
extern SDL_Renderer *render;
extern TTF_Font *titleFont, *menuFont, *optionFont, *detailFont;
extern SDL_Event event;
