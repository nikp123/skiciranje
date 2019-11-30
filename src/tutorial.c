#include "tutorial.h"
#include "editor.h"

SDL_Surface **tutorials;
static SDL_Surface *backButton;
static double offset = 0.0;
int numOfTutorials = 1;

int initTutorial(void) {
	glow = 0;
	tutorials = (SDL_Surface **)malloc(sizeof(SDL_Surface*)*numOfTutorials);
	tutorials[0] = TTF_RenderText_Blended(menuFont, textLine[10], translate_color(OPTION_FONT_COLOR));
	backButton = TTF_RenderText_Blended(menuFont, textLine[9], translate_color(MENU_FONT_COLOR));
	return 0;
}

void drawTutorial(void){
	// clear screen
	SDL_FillRect(swin, NULL, CLEAR_COLOR);

	// draw bar
	SDL_Rect rect;
	rect.x = 0;
	rect.y = swin->h - TTF_FontHeight(menuFont);
	rect.w = swin->w;
	rect.h = TTF_FontHeight(menuFont);
	SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);

	// draw bar buttons
	rect.w = backButton->w;
	rect.h = backButton->h;
	if(glow == 1) SDL_FillRect(swin, &rect, CLEAR_COLOR);
	SDL_BlitSurface(backButton, NULL, swin, &rect);

	// draw elements
	for(int i = floor(offset); i < ceil((swin->h-TTF_FontHeight(menuFont))/TTF_FontHeight(menuFont))+ceil(offset) && i < numOfTutorials; i++) {
		rect.x = 0;
		rect.y = (i-offset)*TTF_FontHeight(menuFont);
		rect.w = tutorials[i]->w;
		rect.h = tutorials[i]->h;
		if(glow == i+2)
			SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
		SDL_BlitSurface(tutorials[i], NULL, swin, &rect);
	}

	SDL_UpdateWindowSurface(win);
	return;
}

int inputTutorial(void) {
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				return 2;
			case SDL_KEYDOWN:
				// Get keystroke
				switch(event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						return 1;
				}
				break;
			case SDL_MOUSEMOTION:
			{
				int mouseCorX = 0, mouseCorY = 0;
				SDL_GetMouseState(&mouseCorX, &mouseCorY);

				if(mouseCorY > swin->h - TTF_FontHeight(menuFont)) {
					if(mouseCorX < backButton->w) glow = 1;
					else glow = 0;
				} else {
					glow = ceil(1.0+(float)mouseCorY/TTF_FontHeight(menuFont)+offset);
				}
				SDL_FlushEvent(SDL_MOUSEMOTION);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				int mouseCorX = 0, mouseCorY = 0;
				SDL_GetMouseState(&mouseCorX, &mouseCorY);

				if(mouseCorY > swin->h - TTF_FontHeight(menuFont)) {
					if(mouseCorX < backButton->w) return 1;
				} else {
					return 3+(int)floor((float)mouseCorY/TTF_FontHeight(menuFont)+offset);
				}
				break;
			}
			case SDL_WINDOWEVENT:
				// In case the user closes the window at any point
				if(event.window.event == SDL_WINDOWEVENT_CLOSE)
					return 2;
				else if(event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					SDL_FreeSurface(swin);
					swin = SDL_GetWindowSurface(win);
					SDL_FillRect(swin, NULL, CLEAR_COLOR);
					SDL_UpdateWindowSurface(win);
					SDL_FlushEvent(SDL_WINDOWEVENT);
					if(numOfTutorials*TTF_FontHeight(menuFont) < swin->w+TTF_FontHeight(menuFont))
						offset = 0.0;
				}
				break;
			case SDL_MOUSEWHEEL:
				if(numOfTutorials*TTF_FontHeight(menuFont) < swin->w+TTF_FontHeight(menuFont)) {
					offset = 0.0;
					break;
				}
				offset -= (float)event.wheel.y;
				if(offset < 0.0) offset = 0.0;
				if(offset+swin->h/TTF_FontHeight(menuFont) > numOfTutorials)
					offset = numOfTutorials - swin->h/TTF_FontHeight(menuFont);
				SDL_FlushEvent(SDL_MOUSEWHEEL);
				break;
			default: break;
		}
	}
	return 0;
}

void cleanTutorial(void) {
	for(int i = 0; i < numOfTutorials; i++)
		SDL_FreeSurface(tutorials[i]);
	free(tutorials);

	SDL_FreeSurface(backButton);
	return;
}

int tutorial(void) {
	if(initTutorial()) return 1;
	drawTutorial();
	int status = 0;
	while(!status) {
		int startFrame = SDL_GetTicks();
		status = inputTutorial();
		drawTutorial();
		if(SDL_GetTicks() - startFrame<1000/FRAMERATE)
			SDL_Delay(1000/FRAMERATE-(SDL_GetTicks()-startFrame));
	}
	if(status==2) exit(EXIT_SUCCESS);
	if(status >= 3) editor(status-2);
	cleanTutorial();
	return 0;
}
