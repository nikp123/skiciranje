﻿#include "editor.h"
#include "snap.h"

static _Bool mouseButton = false, tryToSnap = true, snap, grid;
static char *positionString, *filename;
static int special, lastX, lastY;
static double zoom, x, y, cuts;
static Uint32 lines, drawableLines, lineType = 0, lineStatus = 0;
static line1 *line;
static SDL_Surface *positionSurface = NULL;
static SDL_Texture *positionTexture = NULL, *buttonTexture = NULL;

void redo(void) {
	if(drawableLines != lines) drawableLines++;
	return;
}

void undo(void) {
	if(drawableLines != 0) drawableLines--;
	return;
}

void allocNewLine(Uint32 type, double posX, double posY, int drawType) {
	line1 *new;

	// free memory from redos
	while(lines > drawableLines)
		discardNewLine();

	lines++;
	drawableLines++;


	new = realloc(line, (lines+1)*sizeof(line1));
	if(new == NULL) {
		tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
		SDL_Log("realloc/malloc failed at: %d\n", __LINE__-3);
		exit(EXIT_FAILURE);
	}

	line = new;
	line[lines-1].x = (double*)malloc((drawType ? 101 : 3)*sizeof(double));
	line[lines-1].y = (double*)malloc((drawType ? 101 : 3)*sizeof(double));
	if(line[lines-1].x==NULL||line[lines-1].y==NULL) {
		tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
		SDL_Log("realloc/malloc failed at: %d or %d\n", __LINE__-4, __LINE__-3);
		exit(EXIT_FAILURE);
	}
	if(!drawType) {
		line[lines-1].x[1] = posX;
		line[lines-1].y[1] = posY;
	}

	line[lines-1].x[0] = posX;
	line[lines-1].y[0] = posY;
	line[lines-1].lenght = 1;
	line[lines-1].type = type;
	if(!drawType) {
		if(tryToSnap) snapIfPossible(posX, posY, cuts, line, lines, snap, grid, 1);
		line[lines-1].lenght++;
	}

	// 0 == nothing
	// 1 == line mode
	// 2 == draw mode
	lineStatus = drawType+1;
	return;
}

void addPointToNewDraw(double posX, double posY) {
	// increment the pointer
	line[lines-1].lenght++;

	// grow by every 100th
	if(((int)line[lines-1].lenght/100) - ((int)(line[lines-1].lenght-1)/100)) {
		// grow by 100 points
		double *new1, *new2;
		new1 = realloc(line[lines-1].x, (line[lines-1].lenght+101)*sizeof(double));
		new2 = realloc(line[lines-1].y, (line[lines-1].lenght+101)*sizeof(double));
		if(new1 == NULL||new2 == NULL) {
			tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
			SDL_Log("realloc/malloc failed at: %d or %d\n", __LINE__-4, __LINE__-3);
			exit(EXIT_FAILURE);
		}
		line[lines-1].x = new1;
		line[lines-1].y = new2;
	}

	// apply the new points
	line[lines-1].x[line[lines-1].lenght-1] = posX;
	line[lines-1].y[line[lines-1].lenght-1] = posY;
	return;
}

void finishNewDraw(void) {
	// reallocate to save on memory
	double *new1, *new2;
	new1 = realloc(line[lines-1].x, (line[lines-1].lenght+1)*sizeof(double));
	new2 = realloc(line[lines-1].y, (line[lines-1].lenght+1)*sizeof(double));
	if(new1 == NULL||new2 == NULL) {
		tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
		SDL_Log("realloc/malloc failed at: %d or %d\n", __LINE__-4, __LINE__-3);
		exit(EXIT_FAILURE);
	}
	line[lines-1].x = new1;
	line[lines-1].y = new2;

	// reset line status
	lineStatus = 0;
	return;
}

void discardNewLine(void) {
	if(!lines) return;

	free(line[lines-1].x);
	free(line[lines-1].y);

	lines--;

	// correct redos
	if(lineStatus == 1) drawableLines = lines;

	line1 *new = realloc(line, (lines+1)*sizeof(line1));
	if(new == NULL) {
		tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
		SDL_Log("realloc/malloc failed at: %d\n", __LINE__-3);
		exit(EXIT_FAILURE);
	}

	line = new;
	return;
}

void deleteLine(double posX, double posY) {
	// snap to an previous line
	for(Uint32 i = 0; i < drawableLines; i++) {
		//if(line[i].lenght < 2) continue; // avoids bugged out lines
		for(Uint32 j = 0; j < line[i].lenght-1; j++) {
			// don't try to understand this, this is above my expertise
			// just know that this works and don't touch it
			const double r = 10*zoom;

			// this is a special case where for vertical lines the function below doesnt work so this is a workaround,
			// call it shitty, I don't care
			if(line[i].x[j+1]-line[i].x[j] == 0.0) {
				// check if line is in range of circle's perimiter
				if(((posX-r)<line[i].x[j])&&(line[i].x[j]<(posX+r))) {
					if((max(line[i].y[j+1], line[i].y[j]) > (posY-r))&&((posY+r)>min(line[i].y[j+1], line[i].y[j])))	{
						// free memory from redos
						while(lines > drawableLines)
							discardNewLine();
						free(line[i].x);
						free(line[i].y);
						lines--;
						drawableLines--;

						// dumb but safe way to approach this
						for(Uint32 l = i; l < drawableLines; l++) {
							line[l].x = line[l+1].x;
							line[l].y = line[l+1].y;
							line[l].lenght = line[l+1].lenght;
							line[l].type = line[l+1].type;
						}
					}
				}
			}

			double m = (line[i].y[j+1]-line[i].y[j])/(line[i].x[j+1]-line[i].x[j]);
			double c = line[i].y[j]-m*line[i].x[j];
			double a_ = pow(m,2)+1;
			double b_ = -(2*(posX-m*c+m*posY));
			double c_ = (pow(posX,2)+pow(posY,2)-pow(r,2)+pow(c,2)-2*c*posY);
			double discriminant = pow(b_, 2) - 4*a_*c_;
			if(discriminant>0) {
				// cool maffs huh?
				if(((-b_+sqrt(discriminant))/(2*a_) >= fmin(line[i].x[j], line[i].x[j+1])
				    && (-b_+sqrt(discriminant))/(2*a_) <= fmax(line[i].x[j], line[i].x[j+1])) ||
				        ((-b_-sqrt(discriminant))/(2*a_) >= fmin(line[i].x[j], line[i].x[j+1]) &&
				         (-b_-sqrt(discriminant))/(2*a_) <= fmax(line[i].x[j], line[i].x[j+1]))) {
					// free memory from redos
					while(lines > drawableLines)
						discardNewLine();
					free(line[i].x);
					free(line[i].y);
					lines--;
					drawableLines--;

					// dumb but safe way to approach this
					for(Uint32 l = i; l < drawableLines; l++) {
						line[l].x = line[l+1].x;
						line[l].y = line[l+1].y;
						line[l].lenght = line[l+1].lenght;
						line[l].type = line[l+1].type;
					}
				}
			}
		}
	}
}

void deleteAllLines(void) {
	if(line != NULL) {
		for(Uint32 i = 0; i < lines; i++) {
			free(line[i].x);
			free(line[i].y);
		}
		free(line);
		line = NULL;
	}
	drawableLines = 0;
	lineStatus = 0;
	lines = 0;
	return;
}

int saveEditorFile(char *filename) {
	FILE *file = fopen(filename, "wb");
	if(file == NULL) return 1;

	// wipe redos
	while(lines != drawableLines) discardNewLine();

	// write header
	fprintf(file, "%d\n", lines);

	// write individual lines
	for(Uint32 i = 0; i < lines; i++){
		fprintf(file, "%d %d\n", line[i].type, line[i].lenght);
		for(Uint32 j = 0; j < line[i].lenght; j++)
			fprintf(file, "%lg %lg\n", line[i].x[j], line[i].y[j]);
	}

	// close file pointer
	fclose(file);
	return 0;
}

int loadEditorFile(char *filename, int level) {
	// setting level to 0 will result getting into editing mode
	FILE *file = fopen(filename, "rb");
	if(file == NULL) return 1;

	// reading and vertifying the first line
	if(fscanf(file, "%d\n", &lines) != 1) {
		tinyfd_messageBox(textLine[20], textLine[21], "ok", "error", 1);
		SDL_Log("fscanf failed at: %d\n", __LINE__-2);
		return 1;
	}
	drawableLines = lines;

	// allocating the whole thing
	line = malloc((lines+1)*sizeof(line1));
	if(line==NULL) {
		tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
		SDL_Log("realloc/malloc failed at: %d\n", __LINE__-3);
		return 1;
	}

	// reading and importing the whole thing
	for(Uint32 doneLines = 0; doneLines < lines; doneLines++) {
		if(fscanf(file, "%d %d", &line[doneLines].type, &line[doneLines].lenght) != 2) {
			tinyfd_messageBox(textLine[20], textLine[22], "ok", "error", 1);
			SDL_Log("fscanf failed at: %d\n", __LINE__-2);
			return 1;
		}

		if(line[doneLines].type < 5 && line[doneLines].lenght != 2)
			SDL_Log("WARNING: Potential corruption while loading line %d\nType %d lenght is %d, but it should be 2.\n",
			        doneLines+1, line[doneLines].type, line[doneLines].lenght);

		line[doneLines].x = malloc((line[doneLines].lenght+1)*sizeof(double));
		line[doneLines].y = malloc((line[doneLines].lenght+1)*sizeof(double));
		if(line[doneLines].x == NULL||line[doneLines].y == NULL) {
			tinyfd_messageBox(textLine[20], textLine[19], "ok", "error", 1);
			SDL_Log("realloc/malloc failed at: %d or %d\n", __LINE__-4, __LINE__-3);
			return 1;
		}

		for(Uint32 i = 0; i < line[doneLines].lenght; i++) {
			if(fscanf(file, "%lf %lf", &line[doneLines].x[i], &line[doneLines].y[i]) != 2) {
				tinyfd_messageBox(textLine[20], textLine[22], "ok", "error", 1);
				SDL_Log("fscanf failed at: %d\n", __LINE__-2);
				return 1;
			}
		}
	}

	fclose(file);
	return 0;
}

void drawPosition(void) {
	if(positionTexture != NULL) SDL_DestroyTexture(positionTexture);
	double base = floor(log10(zoom));
	sprintf(positionString, "%sx=%.2f y=%.2f, %s%.2f*10^%.0fm", textLine[11], x, y, textLine[12], zoom/pow(10.0, base), base);
	positionSurface = TTF_RenderUTF8_Blended(detailFont, positionString, translate_color(TITLE_FONT_COLOR));
	positionTexture = SDL_CreateTextureFromSurface(render, positionSurface);
	SDL_FreeSurface(positionSurface);
	return;
}

int initEditor(int level) {
	// special in the sense it needs more bloody code
	special = level;

	// switch window
	SDL_FreeSurface(swin);
	SDL_DestroyWindow(win);
	SDL_Delay(1000);
	win = NULL;
	render = NULL;

	// prepare the new window
	if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS, &win, &render))
	{
		SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
		return 1;
	}

	// allocate memory for position string
	positionString = (char*)malloc(100);
	if(positionString == NULL) {
		SDL_Log("Memory error!\n");
		return 1;
	}

	// allocate and prepare buttons
	SDL_Surface *image = SDL_LoadBMP("assets/image/editor.bmp");
	if(image == NULL) {
		SDL_Log("Cannot load image! ERROR: %s\n", SDL_GetError());
		return 1;
	}
	buttonTexture = SDL_CreateTextureFromSurface(render, image);
	if(buttonTexture == NULL) {
		SDL_Log("Failed to create texture! ERROR: %s\n", SDL_GetError());
		return 1;
	}

	// set the requred parameters
	zoom = 0.1;
	x = 0.0;
	y = 0.0;
	cuts = 10;
	grid = 1;
	snap = 1;
	mouseButton = false;

	drawPosition();
	return 0;
}

void drawNormalLine(Uint32 type, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color) {
	// get window size
	int mousePosX, mousePosY;
	SDL_GetWindowSize(win, &mousePosX, &mousePosY);

	// set draw color
	SDL_SetRenderDrawColor(render, r32(color), g32(color), b32(color), a32(color));

	// draw line
	switch(type) {
		case 1:
			SDL_RenderDrawLine(render, x1, y1, x2, y2);
			break;
		case 2:
			thickLineColor(render, x1, y1, x2, y2, 3, color);
			break;
		case 3:
		{
			double xRatio = (x2-x1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
			double yRatio = (y2-y1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
			double lineLen = sqrt(pow((x2-x1), 2)+pow((y2-y1), 2));
			for(int i = 0; i < floor(lineLen)-6; i += 12){
				if(x1+i*xRatio < 0) continue;
				if(y1+i*yRatio < 0) continue;
				if(x1+i*xRatio > mousePosX) continue;
				if(y1+i*yRatio > mousePosY) continue;

				SDL_RenderDrawLine(render, (Sint16)(x1+i*xRatio),
								   (Sint16)(y1+i*yRatio),
								   (Sint16)(x1+(i+5)*xRatio),
								   (Sint16)(y1+(i+5)*yRatio));
			}
			break;
		}
		case 4:
		{
			double xRatio = (x2-x1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
			double yRatio = (y2-y1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
			double lineLen = sqrt(pow((x2-x1), 2)+pow((y2-y1), 2));
			for(int i = 0; i < floor(lineLen)-6; i += 13){
				if(x1+i*xRatio < 0) continue;
				if(y1+i*yRatio < 0) continue;
				if(x1+i*xRatio > mousePosX) continue;
				if(y1+i*yRatio > mousePosY) continue;

				SDL_RenderDrawLine(render, (Sint16)(x1+i*xRatio),
								   (Sint16)(y1+i*yRatio),
								   (Sint16)(x1+(i+5)*xRatio),
								   (Sint16)(y1+(i+5)*yRatio));
				SDL_RenderDrawPoint(render, (Sint16)(x1+(i+9)*xRatio),
									(Sint16)(y1+(i+9)*yRatio));
			}
			break;
		}
		default:
			SDL_RenderDrawLine(render, x1, y1, x2, y2);
			break;
	}
	return;
}

void cleanEditor() {
	// delete SDL2 stuff
	SDL_DestroyTexture(buttonTexture);
	SDL_DestroyTexture(positionTexture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(win);

	// delete our stuff
	free(positionString);

	// wipe everything
	deleteAllLines();

	// reinitilize the old stuff
	win = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);
	if(win == NULL) {
		SDL_Log("Something went really wrong :|\n");
		exit(EXIT_FAILURE);
	}
	swin = SDL_GetWindowSurface(win);
	if(swin == NULL) {
		SDL_Log("This should've not happened!\n");
		exit(EXIT_FAILURE);
	}

	return;
}

void promptBeforeExit(void) {
	// ask user for their opinion
	if(special) return;

	int dialogResult = tinyfd_messageBox(textLine[23], textLine[24], "yesno", "question", 1);

	if(!dialogResult) return;

	char *new;
	if(filename != NULL)
		new = (char*)tinyfd_saveFileDialog(textLine[13], filename, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);
	else    new = (char*)tinyfd_saveFileDialog(textLine[13], "Untitled.sketch", NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);

	// handle cancel
	if(new == NULL) return;

	// clear old string;
	filename = new;

tryAgain:
	// display error message
	if(saveEditorFile(filename)) {
		// display error message
		tinyfd_messageBox(textLine[16], textLine[17], "ok", "error", 1);
		goto tryAgain;
	}
	return;
}

int editorInput(void) {
	char somethingHasChanged=0;
	while(SDL_PollEvent(&event))
	{
		somethingHasChanged=1;
		switch(event.type)
		{
			case SDL_QUIT:
				promptBeforeExit();
				return 2;
			case SDL_KEYDOWN:
				// Get keystroke
				switch(event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						//promptBeforeExit();
						//SDL_FlushEvent(SDL_KEYDOWN);
						//return 1;
						if(lineStatus == 1) discardNewLine();
						lineStatus = 0;
						lineType = 0;
						break;
					case SDLK_LEFT:
						x += zoom/0.1;
						break;
					case SDLK_RIGHT:
						x -= zoom/0.1;
						break;
					case SDLK_UP:
						y += zoom/0.1;
						break;
					case SDLK_DOWN:
						y -= zoom/0.1;
						break;
					case SDLK_a:
						zoom += zoom*0.1;
						if(zoom > cuts/10)
							cuts *= 10;
						break;
					case SDLK_z:
						if(zoom < 0.00001) break;
						zoom -= zoom*0.1;
						if(zoom < cuts/100)
							cuts /= 10;
						break;
					case SDLK_g:
						grid = !grid;
						break;
					case SDLK_h:
						snap = !snap;
						break;
					case SDLK_x:
						undo();
						break;
					case SDLK_c:
						redo();
						break;
					case SDLK_1:
						lineType = 1;
						break;
					case SDLK_2:
						lineType = 2;
						break;
					case SDLK_3:
						lineType = 3;
						break;
					case SDLK_4:
						lineType = 4;
						break;
					case SDLK_5:
						lineType = 5;
						break;
				}
				drawPosition();
				break;
			case SDL_MOUSEMOTION:
			{
				int mouseCorX = 0, mouseCorY = 0, screenSize[2];
				SDL_GetMouseState(&mouseCorX, &mouseCorY);
				SDL_GetWindowSize(win, &screenSize[0], &screenSize[1]);

				// make a scrollable surface
				if(mouseButton&&!lineType) {
					if(lastX|lastY) {
						x += (mouseCorX-lastX)*zoom;
						y += (mouseCorY-lastY)*zoom;
						drawPosition();
					}
					lastX = mouseCorX;
					lastY = mouseCorY;
				} else if(!mouseButton&&lineStatus==1) {
					if(tryToSnap)
						snapIfPossible((mouseCorX - screenSize[0]/2 - x/zoom)*zoom, (mouseCorY - screenSize[1]/2 - y/zoom)*zoom, cuts, line, lines, snap, grid, 1);
					else {
						line[lines-1].x[line[lines-1].lenght-1] = (mouseCorX - screenSize[0]/2 - x/zoom)*zoom;
						line[lines-1].y[line[lines-1].lenght-1] = (mouseCorY - screenSize[1]/2 - y/zoom)*zoom;
					}
				} else if(mouseButton&&lineStatus==2)
					addPointToNewDraw((mouseCorX - screenSize[0]/2 - x/zoom)*zoom, (mouseCorY - screenSize[1]/2 - y/zoom)*zoom);
				else if(mouseButton&&lineType==127)
					deleteLine((mouseCorX - screenSize[0]/2 - x/zoom)*zoom, (mouseCorY - screenSize[1]/2 - y/zoom)*zoom);
				SDL_FlushEvent(SDL_MOUSEMOTION);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				int mouseCorX = 0, mouseCorY = 0, screenSize[2];
				SDL_GetMouseState(&mouseCorX, &mouseCorY);
				SDL_GetWindowSize(win, &screenSize[0], &screenSize[1]);

				if(event.button.button == SDL_BUTTON_MIDDLE) {
					tryToSnap=false;
					break;
				}

				// Line type selection
				if(event.button.button == SDL_BUTTON_RIGHT){
					if(lineStatus == 1) discardNewLine();
					lineStatus = 0;
					lineType = 0;
				} else if(mouseCorY <= 40 && mouseCorX <= 200)
					if((int)lineType == (mouseCorX-1)/40+1)
						lineType = 0;
					else
						lineType = (Uint32)(mouseCorX-1)/40+1;
				else if(mouseCorY <= 40 && mouseCorX    >= screenSize[0] - 40)
					deleteAllLines();
				else if(mouseCorY <= 80 && mouseCorX >= screenSize[0] - 40) {
					promptBeforeExit();
					return 1;
				} else if(mouseCorY <= 40 && mouseCorX >= screenSize[0] - 120 && mouseCorX < screenSize[0] - 80)
					redo();
				else if(mouseCorY <= 40 && mouseCorX >= screenSize[0] - 160 && mouseCorX < screenSize[0] - 120)
					undo();
				else if(mouseCorY <= 40 && mouseCorX >= screenSize[0] - 240 && mouseCorX < screenSize[0] - 200)
					lineType = 127;
				else if(mouseCorY >= 40 && mouseCorY < 80 && mouseCorX < 40)
					snap = !snap;
				else if(mouseCorY >= 40 && mouseCorY < 80 && mouseCorX >= 40 && mouseCorX < 80)
					grid = !grid;
				else if((mouseCorY >= screenSize[1]-40)&&(mouseCorX<40)){
					// load file
					char *new;
					if(filename != NULL)
						new = (char*)tinyfd_openFileDialog(textLine[14], filename, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15], 0);
					else    new = (char*)tinyfd_openFileDialog(textLine[14], NULL, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15], 0);

					// handle cancel
					if(new == NULL) break;
					filename = new;
					cleanEditor();
					initEditor(0);

					if(loadEditorFile(filename, 0)) tinyfd_messageBox(textLine[16], textLine[17], "ok", "error", 1);
				} else if((mouseCorY >= screenSize[1]-40)&&(mouseCorX<80)){
					char *new;
					if(filename != NULL)
						new = (char*)tinyfd_saveFileDialog(textLine[13], filename, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);
					else    new = (char*)tinyfd_saveFileDialog(textLine[13], "Untitled.sketch", NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);

					// handle cancel
					if(new == NULL) break;
					filename = new;
					if(saveEditorFile(filename)) tinyfd_messageBox(textLine[16], textLine[17], "ok", "error", 1);
				}else if(lineType){
					if(lineType==127){
						deleteLine((mouseCorX - screenSize[0]/2 - x/zoom)*zoom, (mouseCorY - screenSize[1]/2 - y/zoom)*zoom);
					} else allocNewLine(lineType,
					                    (mouseCorX - screenSize[0]/2 - x/zoom)*zoom,
					        (mouseCorY - screenSize[1]/2 - y/zoom)*zoom, lineType==5 ? 1 : 0);
					mouseButton = true;
				}else if(lineStatus==1){
					lineStatus = 0;
					line[lines-1].x[1] = (mouseCorX - screenSize[0]/2 - x/zoom)*zoom;
					line[lines-1].y[1] = (mouseCorY - screenSize[1]/2 - y/zoom)*zoom;
				} mouseButton = true;
				break;
			}
			case SDL_MOUSEBUTTONUP:
			{
				if(event.button.button == SDL_BUTTON_MIDDLE) tryToSnap=true;
				else {
					mouseButton = false;
					lastX = 0;
					lastY = 0;
					if(lineStatus==2) finishNewDraw();
				}
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				if(event.wheel.y == 1) zoom += zoom*0.1;
				else if(event.wheel.y == -1) {
					if(zoom < 0.00001) break;
					zoom -= zoom*0.1;
				}
				if(zoom > cuts/10) cuts *= 10;
				else if(zoom < cuts/100) cuts /= 10;

				drawPosition();
				break;
			}
			case SDL_WINDOWEVENT:
				// In case the user closes the window at any point
				if(event.window.event == SDL_WINDOWEVENT_CLOSE)
					return 2;
				else if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_RenderClear(render);
					SDL_FlushEvent(SDL_WINDOWEVENT);
				}
				break;
			default: break;
		}
	}
	return somethingHasChanged?0:3;
}

void drawEditor() {
	// clear screen
	SDL_SetRenderDrawColor(render, (Uint8)CLEAR_COLOR >> 24 % 8, (Uint8)CLEAR_COLOR >> 16 % 8, (Uint8)CLEAR_COLOR >> 8 % 8, (Uint8)CLEAR_COLOR % 8);
	SDL_RenderClear(render);

	// get window size
	int screenSize[2], textureSize[2];
	SDL_GetWindowSize(win, &screenSize[0], &screenSize[1]);

	// additional variables
	SDL_Rect rect;


	// draw axis
	if(grid){
		// X axis
		Sint16 counter = 0;
		Sint16 tempVar = (Sint16)screenSize[0]/2 + (Sint16)((double)x/zoom);
		double cutSize = cuts/zoom;
		while(tempVar < screenSize[0]) {
			drawNormalLine(1, tempVar, 0, tempVar, (Sint16)screenSize[1], DETAIL_FONT_COLOR);
			counter++;
			tempVar = (Sint16)(screenSize[0]/2 + x/zoom + cutSize*counter);
		}
		counter = 0;
		tempVar = (Sint16)screenSize[0]/2 + (Sint16)((double)x/(double)zoom);
		while(tempVar > 0) {
			drawNormalLine(1, tempVar, 0, tempVar, (Sint16)screenSize[1], DETAIL_FONT_COLOR);
			counter++;
			tempVar = (Sint16)(screenSize[0]/2 + x/zoom - cutSize*counter);
		}

		// Y axis
		counter = 0;
		tempVar = (Sint16)screenSize[1]/2 + (Sint16)((double)y/(double)zoom);
		while(tempVar < screenSize[1]) {
			drawNormalLine(1, 0, tempVar, (Sint16)screenSize[0], tempVar, DETAIL_FONT_COLOR);
			counter++;
			tempVar = (Sint16)(screenSize[1]/2 + y/zoom + cutSize*counter);
		}
		counter = 0;
		tempVar = (Sint16)screenSize[1]/2 + (Sint16)((double)y/(double)zoom);
		while(tempVar > 0) {
			drawNormalLine(1, 0, tempVar, (Sint16)screenSize[0], tempVar, DETAIL_FONT_COLOR);
			counter++;
			tempVar = (Sint16)(screenSize[1]/2 + y/zoom - cutSize*counter);
		}
	}

	// draw lines
	double screenDiag=sqrt(pow(screenSize[0],2)+pow(screenSize[1],2));
	for(Uint32 i = 0; i < drawableLines; i++) {
		double line_x[2], line_y[2];

		for(Uint32 j = 0; j < line[i].lenght-1; j++){
			// convert line coordinates to screen space ones
			// using doubles because even longs run out
			line_x[0] = screenSize[0]/2 + ((double)x/zoom + (double)line[i].x[j]/zoom);
			line_x[1] = screenSize[0]/2 + ((double)x/zoom + (double)line[i].x[j+1]/zoom);
			line_y[0] = screenSize[1]/2 + ((double)y/zoom + (double)line[i].y[j]/zoom);
			line_y[1] = screenSize[1]/2 + ((double)y/zoom + (double)line[i].y[j+1]/zoom);

			// We'll need this for the 3 laws of not rendering(C)
			double ratio = (double)(line_y[1]-line_y[0])/(double)(line_x[1]-line_x[0]);
			double length=sqrt(pow(line_x[1]-line_x[0], 2) + pow(line_y[1]-line_y[0], 2));

			// The following code contains 3 laws of not rendering
			// they should be charished by future generations to come

			// 1st law: Don't render a line which has both of its dots outside of the screen
			// in a single direction
			if((line_x[0]<0&&line_x[1]<0) ||
			     (line_y[0]<0&&line_y[1]<0) ||
			     (line_x[0]>screenSize[0]&&line_x[1]>screenSize[0]) ||
				 (line_y[0]>screenSize[1]&&line_y[1]>screenSize[1])) continue;

			// exception to 2nd law
			// If the line is shorter than the screen diagonal, do not apply the 2nd because
			// it's pointless to do this if the line is short to begin with
			if(length*zoom < screenDiag) {
				// 2nd law: If a line has a coordinate that goes out of bounds in any direction,
				// correct it so that the coordinate MUST be at the edge of the screen space
				// fix X to the left
				if(line_x[0] < 0) {
					line_y[0]+=(ratio*(0-line_x[0]));
					line_x[0]=0;
				} else if(line_x[1] < 0) {
					line_y[1]+=(ratio*(0-line_x[1]));
					line_x[1]=0;
				}
				// fix X to the right
				if(line_x[0] > screenSize[0]) {
					line_y[0]+=(ratio*(screenSize[0]-line_x[0]));
					line_x[0]=screenSize[0];
				} else if(line_x[1] > screenSize[0]) {
					line_y[1]+=(ratio*(screenSize[0]-line_x[1]));
					line_x[1]=screenSize[0];
				}
				// fix Y above
				if(line_y[0] < 0) {
					line_x[0]+=((0-line_y[0])/ratio);
					line_y[0] = 0;
				} else if(line_y[1] < 0) {
					line_x[1]+=((0-line_y[1])/ratio);
					line_y[1] = 0;
				}
				// fix Y downwards
				if(line_y[0] > screenSize[1]) {
					line_x[0]+=((screenSize[1]-line_y[0])/ratio);
					line_y[0] = screenSize[1];
				} else if(line_y[1] > screenSize[1]) {
					line_x[1]+=((screenSize[1]-line_y[1])/ratio);
					line_y[1] = screenSize[1];
				}
			}

			// draw the line
			drawNormalLine(line[i].type, (Sint16)line_x[0], (Sint16)line_y[0],
						(Sint16)line_x[1], (Sint16)line_y[1], TITLE_FONT_COLOR);
		}
	}

	// Draw UI
	SDL_RenderSetScale(render, 2.0, 2.0);
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){0, lineType == 1 ? 20 : 0,20,20}, &(SDL_Rect){0,0,20,20});    // line button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){20, lineType == 2 ? 20 : 0,20,20}, &(SDL_Rect){20,0,20,20});  // thick line button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){40, lineType == 3 ? 20 : 0,20,20}, &(SDL_Rect){40,0,20,20});  // line-stop-line button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){60, lineType == 4 ? 20 : 0,20,20}, &(SDL_Rect){60,0,20,20});  // line-dot-line button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){80, lineType == 5 ? 20 : 0,20,20}, &(SDL_Rect){80,0,20,20});  // draw button

	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){160, 0, 20, 20}, &(SDL_Rect){0, screenSize[1]/2-20, 20, 20});                                  // load button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){120, 0, 20, 20}, &(SDL_Rect){20, screenSize[1]/2-20, 20, 20});                                 // save button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){100, 0, 20, 20}, &(SDL_Rect){screenSize[0]/2-20, 0, 20, 20});                                  // delete all button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){180, 0, 20, 20}, &(SDL_Rect){screenSize[0]/2-20, 20, 20, 20});                                 // menu
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){140, drawableLines > 0 ? 0 : 20, 20, 20}, &(SDL_Rect){screenSize[0]/2-80, 0, 20, 20});         // back button
	SDL_RenderCopyEx(render, buttonTexture, &(SDL_Rect){140, lines == drawableLines ? 20 : 0, 20, 20}, &(SDL_Rect){screenSize[0]/2-60, 0, 20, 20},   // redo button
	                 0, NULL, SDL_FLIP_HORIZONTAL);
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){200, lineType == 127 ? 20 : 0, 20, 20}, &(SDL_Rect){screenSize[0]/2-120, 0, 20, 20});          // delete button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){220, snap ? 20 : 0, 20, 20}, &(SDL_Rect){0, 20, 20, 20});                                 // snap button
	SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){240, grid ? 20 : 0, 20, 20}, &(SDL_Rect){20, 20, 20, 20});                                // grid button

	// In the bottom right draw the position
	SDL_RenderSetScale(render, 1.0, 1.0);
	SDL_QueryTexture(positionTexture, NULL, NULL, &textureSize[0], &textureSize[1]);
	rect.x = screenSize[0] - textureSize[0] - 10;
	rect.y = screenSize[1] - textureSize[1] - 10;
	rect.w = textureSize[0] + 10;
	rect.h = textureSize[1] + 10;
	SDL_SetRenderDrawColor(render, r32(CLEAR_COLOR), g32(CLEAR_COLOR), b32(CLEAR_COLOR), a32(CLEAR_COLOR));
	SDL_RenderFillRect(render, &rect);
	SDL_SetRenderDrawColor(render, r32(TITLE_FONT_COLOR), g32(TITLE_FONT_COLOR), b32(TITLE_FONT_COLOR), a32(TITLE_FONT_COLOR));
	SDL_RenderDrawRect(render, &rect);
	rect.x+=5;
	rect.y+=5;
	rect.w-=10;
	rect.h-=10;
	SDL_RenderCopy(render, positionTexture, NULL, &rect);

	int mousePos[2];
	SDL_GetMouseState(&mousePos[0], &mousePos[1]);

	if(tryToSnap&&lineType) {
		if(snapIfPossible((mousePos[0]-screenSize[0]/2-x/zoom)*zoom, (mousePos[1]-screenSize[1]/2-y/zoom)*zoom, \
		                  cuts, line, lines, snap, grid, 0)) {

			rect.x = mousePos[0] - 5;
			rect.y = mousePos[1] - 5;
			rect.w = 10;
			rect.h = 10;
			SDL_SetRenderDrawColor(render, r32(TITLE_FONT_COLOR), g32(TITLE_FONT_COLOR), b32(TITLE_FONT_COLOR), a32(TITLE_FONT_COLOR));
			SDL_RenderFillRect(render, &rect);
		}}

	// update frame
	SDL_RenderPresent(render);
	return;
}

int editor(int level)
{
	// get the name of the file you're about to open
	if(!level) {
		filename = (char*)tinyfd_openFileDialog(textLine[14], NULL, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15], 0);
		if(filename == NULL) return 0;
	} else if(level != INT_MAX){
		filename = (char*) malloc(sizeof(char)*128);
		if(filename == NULL) {
			SDL_Log("Memory error!\n");
			exit(EXIT_FAILURE);
		}
		sprintf(filename, "assets/levels/%d", level);
	}

	// open it up, also level == INT_MAX means that it's a new file
	if(level != INT_MAX) {
		if(loadEditorFile(filename, level)) {
			SDL_Log("Error loading file: %s\n", filename);
			return 0;
		}
	}

	// INT_MAX = no level - just start the editor
	if(level == INT_MAX) level = 0;

	// free memory
	if(level&&filename!=NULL) free(filename);

	initEditor(level);
	int status = 0;

	while(!status||status==3) {
		Uint32 frameStart = SDL_GetTicks();

		if(!status)
			drawEditor();
		//drawTutorial();

		status = editorInput();

		if((SDL_GetTicks() - frameStart) < 1000/FRAMERATE)
			SDL_Delay(1000/FRAMERATE - (SDL_GetTicks() - frameStart));
	}
	if(status == 2) exit(EXIT_SUCCESS);
	cleanEditor();
	return 0;
}
