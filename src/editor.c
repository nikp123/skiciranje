#include "editor.h"

int numOfLines, numOfDraws;
int special, grid;
int lastX, lastY;
double zoom, x, y, cuts;
SDL_Surface *positionSurface = NULL;
SDL_Texture *positionTexture = NULL, *buttonTexture;
char *positionString, mouseButton = false;

// I'm lazy, deal with it.
#define thinLineButtonOffset 0
#define thickLineButtonOffset 20
#define cutLineButton 40
#define dotLineButton 60
#define drawLineButton 80
#define clearButton 100
#define saveButton 120
#define backButton 140
#define fileButton 160

struct line1 {
    int type;
    double x[2];
    double y[2];
};

struct line2 {
    int type, lenght;
    double *x;
    double *y;
};

struct line1 *normalLine;
struct line2 *drawLine;

int loadEditorFile(char *filename, int level) {
    // setting level to 0 will result getting into editing mode
    FILE *file = fopen(filename, "rb");
    if(file == NULL) return 1;

    // reading and vertifying the first line
    if(fscanf(file, "%d %d\n", &numOfLines, &numOfDraws) != 2) {
        SDL_Log("Error: unable to read file info!\n");
        return 1;
    }

    // allocating the whole thing
    normalLine = malloc(numOfLines*sizeof(struct line1));
    drawLine = malloc(numOfDraws*sizeof(struct line2));
    if(normalLine==NULL||drawLine==NULL) {
        SDL_Log("Memory error!\n");
        return 1;
    }

    // allocating our string
    char *tempString = malloc(1024);

    // reading and importing the whole thing
    int doneLines = 0, doneDraws = 0;
    while(fgets(tempString, 1024, file) != NULL) {
        if(doneLines < numOfLines){
            sscanf(tempString, "%d %lf %lf %lf %lf", &normalLine[doneLines].type, &normalLine[doneLines].x[0],
                    &normalLine[doneLines].y[0], &normalLine[doneLines].x[1], &normalLine[doneLines].y[1]);
			doneLines++;
		} else break;
        // TODO DRAWING LINES
        /**
        if(doneDraws != numOfDraws){
            doneLines++;
            sscanf(tempString, "%d %f %f %f %f", normalLine[i].type, normalLine[i].x[0], normalLine[i].y[0], normalLine[i].x[1], normalLine[i].y[1]);
        }**/
    }

    free(tempString);
    fclose(file);
    return 0;
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
    win = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS&SDL_WINDOW_FULLSCREEN_DESKTOP);
    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    if(win == NULL||render == NULL) {
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
    zoom = 0.1f;
    x = 0.0f;
    y = 0.0f;
    cuts = 10;
    grid = 1;
    mouseButton = false; 

    drawPosition();
    return 0;
}

void drawNormalLine(int type, int x1, int y1, int x2, int y2, Uint32 color) {

    // set draw color
    SDL_SetRenderDrawColor(render, color >> 24 % 8, color >> 16 % 8, color >> 8 % 8, color % 8);

    // draw line
    switch(type) {
        case 2:
            if(abs(x1-x2) > abs(y1-y2)){
            	SDL_RenderDrawLine(render, x1, y1-1, x2, y2-1);
            	SDL_RenderDrawLine(render, x1, y1, x2, y2);
            	SDL_RenderDrawLine(render, x1, y1+1, x2, y2+1);
            } else if(abs(x1-x2) < abs(y1-y2)){
            	SDL_RenderDrawLine(render, x1-1, y1, x2-1, y2);
            	SDL_RenderDrawLine(render, x1, y1, x2, y2);
            	SDL_RenderDrawLine(render, x1+1, y1, x2+1, y2);
            }
            break;
        case 1:
            SDL_RenderDrawLine(render, x1, y1, x2, y2);
            break;
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

	free(normalLine);		// coorespoding numOfLines
	//free(drawLine);		// coorespoding numOfDraws

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

void drawPosition(void) {
    if(positionTexture != NULL) SDL_DestroyTexture(positionTexture);
    sprintf(positionString, "%sx=%.2f y=%.2f, %s%.2f\0", textLine[11], x, y, textLine[12], zoom);
    positionSurface = TTF_RenderUTF8_Blended(detailFont, positionString, translate_color(DETAIL_FONT_COLOR));
    positionTexture = SDL_CreateTextureFromSurface(render, positionSurface);
    SDL_FreeSurface(positionSurface);
    return;
}

int editorInput(void) {
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
                        zoom -= zoom*0.1;
                        if(zoom < cuts/100)
                            cuts /= 10;
                        break;
					case SDLK_g:
						grid = !grid;
						break;
                }
                drawPosition();
                break;
            case SDL_MOUSEMOTION:
            {
                int mouseCorX = 0, mouseCorY = 0;
                SDL_GetMouseState(&mouseCorX, &mouseCorY);

				// make a scrollable surface
				if(mouseButton) {
					if(lastX|lastY) {
						x += (mouseCorX-lastX)*zoom;
						y += (mouseCorY-lastY)*zoom;
						drawPosition();
					} 
					lastX = mouseCorX;
					lastY = mouseCorY;
				}			

                SDL_FlushEvent(SDL_MOUSEMOTION);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                int mouseCorX = 0, mouseCorY = 0;
                SDL_GetMouseState(&mouseCorX, &mouseCorY);

				mouseButton = true;
                break;
            }
			case SDL_MOUSEBUTTONUP:
			{
				mouseButton = false;
				lastX = 0;
				lastY = 0;
				break;
			}
			case SDL_MOUSEWHEEL:
			{
				if(event.wheel.y == 1) zoom += zoom*0.1;
				else if(event.wheel.y == -1) zoom -= zoom*0.1;
				if(zoom > cuts/10) cuts *= 10;
				else if(zoom < cuts/100) cuts /= 10;
				drawPosition();
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
                }
                break;
            default: break;
        }
    }
    return 0;
}

void drawEditor() {
    // clear screen
    SDL_SetRenderDrawColor(render, CLEAR_COLOR >> 16 % 8, CLEAR_COLOR >> 8 % 8, CLEAR_COLOR % 8, CLEAR_COLOR >> 24 % 8);
    SDL_RenderClear(render);

    // get window size
    int tempVarX, tempVarY, tempVarW, tempVarH;
    SDL_GetWindowSize(win, &tempVarX, &tempVarY);

	// additional variables
	SDL_Rect rect;

    // draw axis
    if(grid){
    	// X axis
    	int tempVar = tempVarX/2 + x/zoom;
    	while(tempVar < tempVarX) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
    	    tempVar += cuts/zoom;
    	}
    	tempVar = tempVarX/2 + x/zoom;
    	while(tempVar > 0) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
    	    tempVar -= cuts/zoom;
    	}
    	// Y axis
    	tempVar = tempVarY/2 + y/zoom;
    	while(tempVar < tempVarY) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
    	    tempVar += cuts/zoom;
    	}
    	tempVar = tempVarY/2 + y/zoom;
    	while(tempVar > 0) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
    	    tempVar -= cuts/zoom;
    	}
    }

    // draw normal lines
    for(int i = 0; i < numOfLines; i++) {
        int line_x[2], line_y[2];

        // calculate the lines
        line_x[0] = tempVarX/2 + (int)((double)x/zoom + (double)normalLine[i].x[0]/zoom);
        line_x[1] = tempVarX/2 + (int)((double)x/zoom + (double)normalLine[i].x[1]/zoom);
        line_y[0] = tempVarY/2 + (int)((double)y/zoom + (double)normalLine[i].y[0]/zoom);
        line_y[1] = tempVarY/2 + (int)((double)y/zoom + (double)normalLine[i].y[1]/zoom);

        // draw the line
        drawNormalLine(normalLine[i].type, line_x[0], line_y[0], line_x[1], line_y[1], TITLE_FONT_COLOR);
        // TODO: Implement different drawing methods

    }

    // TODO: draw special lines

	// Draw UI
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){0,0,100,20}, &(SDL_Rect){0,0,100,20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){160, 0, 20, 20}, &(SDL_Rect){0, tempVarY-20, 20, 20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){120, 0, 20, 20}, &(SDL_Rect){20, tempVarY-20, 20, 20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){100, 0, 20, 20}, &(SDL_Rect){tempVarX-20, 0, 20, 20});


    // In the bottom right draw the position
    SDL_QueryTexture(positionTexture, NULL, NULL, &tempVarW, &tempVarH);
    rect.x = tempVarX - tempVarW;
    rect.y = tempVarY - tempVarH;
    rect.w = tempVarW;
    rect.h = tempVarH;
    SDL_RenderCopy(render, positionTexture, NULL, &rect);

    // update frame
    SDL_RenderPresent(render);
    return;
}

int editor(int level)
{
    char *filename;
    // get the name of the file you're about to open
    if(!level) {
        filename = fileSelector();
        if(filename == NULL) return 0;
    } else {
        filename = (char*) malloc(sizeof(char)*128);
        sprintf(filename, "assets/levels/%d", level);
    }

    // open it up
    if(loadEditorFile(filename, level)) {
        SDL_Log("Error loading file: %s\n", filename);
        return 0;
    }

    initEditor(level);
    drawEditor();
    int status = 0;

    while(!status) {
        int frameStart = SDL_GetTicks();

        status = editorInput();
        drawEditor();
        //drawTutorial();
        if((SDL_GetTicks() - frameStart) < 1000/FRAMERATE)
            SDL_Delay(1000/FRAMERATE - (SDL_GetTicks() - frameStart));
    }
    if(status == 2) exit(EXIT_SUCCESS);
    cleanEditor();
    return 0;
}
