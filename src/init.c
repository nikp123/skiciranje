#include "shared.h"
#include "functions.h"

int gameInit(int argc, char *argv[])
{
	// Stage 1, get window up and running
	// share command line options
	argumentCount = argc;
	arguments = argv;

	// Do library initilization first
	if(SDL_Init(SDL_USE_FLAGS)) {
		SDL_Log("Unable to initilize SDL: %s\n", SDL_GetError());
		return 1;
	}
	if(TTF_Init()) {
		SDL_Log("Unable to initilize SDL_ttf: %s\n", TTF_GetError());
		return 2;
	}

	// Create our first and only window
	win = SDL_CreateWindow(WINDOW_TITLE,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			WINDOW_FLAGS);
	if(win==NULL) {
		SDL_Log("Yo dawg, you need a desktop for this program to run!\nOr this happened: %s\n", SDL_GetError());
		return 3;
	}

	// give it graphical capabilities
	swin = SDL_GetWindowSurface(win);
	if(swin==NULL) {
		SDL_Log("Unable to get window surface from window: %s\n", SDL_GetError());
		return 4;
	}

	SDL_FillRect(swin, NULL, CLEAR_COLOR);
	SDL_UpdateWindowSurface(win);

	// Stage 2, get configuration to work
	if(loadLanguage("sr_RS.UTF-8")) return 1;

	// loading fonts
	titleFont = TTF_OpenFont("assets/fonts/Matamata-Bold.ttf", 32);
	if(titleFont == NULL) {
		SDL_Log("Cannot open font %s, reason: %s\n", "assets/fonts/Matamata-Bold.ttf", TTF_GetError());
		return 1;
	}

	// draw rect
	SDL_Surface *loadingText = TTF_RenderUTF8_Blended(titleFont, textLine[0], translate_color(TITLE_FONT_COLOR));
	SDL_Rect rect;
	rect.x = (swin->w-loadingText->w)/2;
	rect.y = (swin->h-loadingText->h)/2;
	rect.w = loadingText->w;
	rect.h = loadingText->h;
	SDL_BlitSurface(loadingText, NULL, swin, &rect);
	SDL_UpdateWindowSurface(win);
	SDL_FreeSurface(loadingText);


	// loading fonts
	menuFont = TTF_OpenFont("assets/fonts/Matamata-Bold.ttf", 28);
	if(titleFont == NULL) {
		SDL_Log("Cannot open font %s, reason: %s\n", "assets/fonts/Matamata-Bold.ttf", TTF_GetError());
		return 1;
	}
	optionFont = TTF_OpenFont("assets/fonts/Matamata-Regular.ttf", 16);
	if(titleFont == NULL) {
		SDL_Log("Cannot open font %s, reason: %s\n", "assets/fonts/Matamata-Regular.ttf", TTF_GetError());
		return 1;
	}
	detailFont = TTF_OpenFont("assets/fonts/Matamata-Regular.ttf", 12);
	if(titleFont == NULL) {
		SDL_Log("Cannot open font %s, reason: %s\n", "assets/fonts/Matamata-Regular.ttf", TTF_GetError());
		return 1;
	}


	return 0;
}
