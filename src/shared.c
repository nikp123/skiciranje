#include "shared.h"

int argumentCount;
char **arguments;

SDL_Window *win;
SDL_Surface *swin;
SDL_Renderer *render;

char **textLine;
int textLines;

TTF_Font *titleFont, *menuFont, *optionFont, *detailFont;
SDL_Event event;

// many functions use this ;P
int glow;
