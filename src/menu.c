#include "menu.h"

SDL_Surface *editorButton, *aboutButton, *tutorialButton, *exitButton, *loadButton;
SDL_Surface *title, *information;

int initMenu() {
	char *line = realloc(textLine[6], (strlen(textLine[6])+strlen(VERSION)+1)*sizeof(char));
	textLine[6] = line;
	strcat(textLine[6], VERSION);

	title = TTF_RenderUTF8_Blended(titleFont, textLine[1], translate_color(TITLE_FONT_COLOR));
	information = TTF_RenderUTF8_Blended(detailFont, textLine[6], translate_color(DETAIL_FONT_COLOR));
	editorButton = TTF_RenderUTF8_Blended(menuFont, textLine[2], translate_color(MENU_FONT_COLOR));
	loadButton = TTF_RenderUTF8_Blended(menuFont, textLine[18], translate_color(MENU_FONT_COLOR));
	tutorialButton = TTF_RenderUTF8_Blended(menuFont, textLine[3], translate_color(MENU_FONT_COLOR));
	aboutButton = TTF_RenderUTF8_Blended(menuFont, textLine[4], translate_color(MENU_FONT_COLOR));
	exitButton = TTF_RenderUTF8_Blended(menuFont, textLine[5], translate_color(MENU_FONT_COLOR));

	if(title==NULL||information==NULL||editorButton==NULL||loadButton==NULL||tutorialButton==NULL||aboutButton==NULL||exitButton==NULL) {
		SDL_Log("Some of the texts failed to generate: %s\n", TTF_GetError());
		return 1;
	}
	return 0;
}

void drawMenu() {
	// first clear screen
	SDL_FillRect(swin, NULL, CLEAR_COLOR);

	SDL_Rect rect;

	// draw title
	rect.x = (swin->w-title->w)/2;
	rect.y = swin->h/4;
	rect.w = title->w;
	rect.h = title->h;
	SDL_BlitSurface(title, NULL, swin, &rect);

	// draw info
	rect.x = swin->w-information->w;
	rect.y = swin->h-information->h;
	rect.w = information->w;
	rect.h = information->h;
	SDL_BlitSurface(information, NULL, swin, &rect);

	// draw edit button
	rect.x = (swin->w-editorButton->w)/2;
	rect.y = swin->h/4+title->h*2;
	rect.w = editorButton->w;
	rect.h = editorButton->h;
	if(glow==1) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
	SDL_BlitSurface(editorButton, NULL, swin, &rect);

	// draw load button
	rect.x = (swin->w-loadButton->w)/2;
	rect.y += editorButton->h;
	rect.w = loadButton->w;
	rect.h = loadButton->h;
	if(glow==2) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
	SDL_BlitSurface(loadButton, NULL, swin, &rect);

	// draw tutorial button
	rect.x = (swin->w-tutorialButton->w)/2;
	rect.y += loadButton->h;
	rect.w = tutorialButton->w;
	rect.h = tutorialButton->h;
	if(glow==3) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
	SDL_BlitSurface(tutorialButton, NULL, swin, &rect);

	// draw info button
	rect.x = (swin->w-aboutButton->w)/2;
	rect.y += tutorialButton->h;
	rect.w = aboutButton->w;
	rect.h = aboutButton->h;
	if(glow==4) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
	SDL_BlitSurface(aboutButton, NULL, swin, &rect);

	// draw exit button
	rect.x = (swin->w-exitButton->w)/2;
	rect.y += aboutButton->h;
	rect.w = exitButton->w;
	rect.h = exitButton->h;
	if(glow==5) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
	SDL_BlitSurface(exitButton, NULL, swin, &rect);

	SDL_UpdateWindowSurface(win);
	return;
}

int inputMenu() {
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_QUIT:
				return 1;
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

				if(mouseCorX > (swin->w-editorButton->w)/2 && mouseCorX < (swin->w-editorButton->w)/2+editorButton->w&&
				        mouseCorY > swin->h/4+title->h*2 && mouseCorY < swin->h/4+title->h*2+editorButton->h) glow = 1;
				else if(mouseCorX > (swin->w-loadButton->w)/2 && mouseCorX < (swin->w-loadButton->w)/2+loadButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h && mouseCorY < swin->h/4+title->h*2+editorButton->h*2) glow = 2;
				else if(mouseCorX > (swin->w-tutorialButton->w)/2 && mouseCorX < (swin->w-tutorialButton->w)/2+tutorialButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*2 && mouseCorY < swin->h/4+title->h*2+editorButton->h*3) glow = 3;
				else if(mouseCorX > (swin->w-aboutButton->w)/2 && mouseCorX < (swin->w-aboutButton->w)/2+aboutButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*3 && mouseCorY < swin->h/4+title->h*2+editorButton->h*4) glow = 4;
				else if(mouseCorX > (swin->w-exitButton->w)/2 && mouseCorX < (swin->w-exitButton->w)/2+exitButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*4 && mouseCorY < swin->h/4+title->h*2+editorButton->h*5) glow = 5;
				else glow = 0;
				SDL_FlushEvent(SDL_MOUSEMOTION);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				int mouseCorX = 0, mouseCorY = 0;
				SDL_GetMouseState(&mouseCorX, &mouseCorY);

				if(mouseCorX > (swin->w-editorButton->w)/2 && mouseCorX < (swin->w-editorButton->w)/2+editorButton->w&&
				        mouseCorY > swin->h/4+title->h*2 && mouseCorY < swin->h/4+title->h*2+editorButton->h) return 2;
				else if(mouseCorX > (swin->w-loadButton->w)/2 && mouseCorX < (swin->w-loadButton->w)/2+loadButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h && mouseCorY < swin->h/4+title->h*2+editorButton->h*2) return 3;
				else if(mouseCorX > (swin->w-tutorialButton->w)/2 && mouseCorX < (swin->w-tutorialButton->w)/2+tutorialButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*2 && mouseCorY < swin->h/4+title->h*2+editorButton->h*3) return 4;
				else if(mouseCorX > (swin->w-aboutButton->w)/2 && mouseCorX < (swin->w-aboutButton->w)/2+editorButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*3 && mouseCorY < swin->h/4+title->h*2+editorButton->h*4) return 5;
				else if(mouseCorX > (swin->w-exitButton->w)/2 && mouseCorX < (swin->w-exitButton->w)/2+exitButton->w&&
				        mouseCorY > swin->h/4+title->h*2+editorButton->h*4 && mouseCorY < swin->h/4+title->h*2+editorButton->h*5) return 1;
				SDL_FlushEvent(SDL_MOUSEBUTTONDOWN);
				break;
			}
			case SDL_WINDOWEVENT:
				// In case the user closes the window at any point
				if(event.window.event == SDL_WINDOWEVENT_CLOSE)
					return 1;
				else if(event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					SDL_FreeSurface(swin);
					swin = SDL_GetWindowSurface(win);
					SDL_FillRect(swin, NULL, CLEAR_COLOR);
					SDL_UpdateWindowSurface(win);
					SDL_FlushEvent(SDL_WINDOWEVENT);
				}
				break;
			default: break;
		}
	}
	return 0;
}

void cleanMenu() {
	SDL_FreeSurface(title);
	SDL_FreeSurface(information);
	SDL_FreeSurface(editorButton);
	SDL_FreeSurface(loadButton);
	SDL_FreeSurface(tutorialButton);
	SDL_FreeSurface(aboutButton);
	SDL_FreeSurface(exitButton);
	return;
}

int menu() {
	if(initMenu()) return 1;
	drawMenu();
	int status = 0;
	int lastTick = SDL_GetTicks();
	while(!status) {
		status = inputMenu();
		drawMenu();

		// do performance thingies
		int waitTime = 1000/FRAMERATE;
		waitTime -= SDL_GetTicks()-lastTick;
		if(waitTime > 0) SDL_Delay(waitTime);
		lastTick = SDL_GetTicks();
	}

	cleanMenu();
	return status;
}
