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

// handle file types
const char *FILE_TYPES[] = {"*.sketch"};
const int NUM_OF_FILE_TYPES = 1;

// some abstracted functions so I don't repeat the same code 100 times
double distance2d(double deltaX, double deltaY) {
	return sqrt(deltaX*deltaX+deltaY*deltaY);
}

float infinity(void) {
	int inf = 0x7F800000;
	return *(float*)&inf;
}

double cmod(double a, double b) {
	// performs a modulo to the closest
	double c=fmod(a, b);
	if(c<(b-c))
		return c;
	else
		return b-c;
}
