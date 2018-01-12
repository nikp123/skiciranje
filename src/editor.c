#include "editor.h"

int numOfLines, numOfDraws;
int special, grid;
int lastX, lastY;
double zoom, x, y, cuts;
SDL_Surface *positionSurface = NULL;
SDL_Texture *positionTexture = NULL, *buttonTexture;
char *positionString, mouseButton = false, lineType = 0, lineStatus = 0;
char *filename;

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

typedef struct {
    Uint32 type;
    double x[2];
    double y[2];
} line1;

typedef struct {
    Uint32 type;
    Uint32 lenght;
    double *x;
    double *y;
} line2;

line1 *normalLine;
line2 *drawLine;

void allocNewLine(int type, double posX, double posY) {
    numOfLines++;
    line1 *new;

    if(normalLine == NULL) new = malloc(2*sizeof(line1));
    else new = realloc(normalLine, (numOfLines+1)*sizeof(line1));

    if(new == NULL) {
        SDL_Log("Uh oh! Something went terribly wrong! The program is going to crash, NOW!\n");
        exit(EXIT_FAILURE);
    }

    normalLine = new;
    normalLine[numOfLines-1].x[0] = posX;
    normalLine[numOfLines-1].y[0] = posY;
    normalLine[numOfLines-1].x[1] = posX;
    normalLine[numOfLines-1].y[1] = posY;
    normalLine[numOfLines-1].type = type;
    lineStatus = 1;
    return;
}

void allocNewDraw(int type, double posX, double posY) {
    line2 *new;

    numOfDraws++;

    if(drawLine != NULL) new = realloc(drawLine, (numOfDraws+1)*sizeof(line2));
    else new = malloc(sizeof(line2)*2);

    if(new == NULL) {
        SDL_Log("Uh oh! Something went terribly wrong! The program is going to crash, NOW!\n");
        exit(EXIT_FAILURE);
    }

    drawLine = new;
    drawLine[numOfDraws-1].lenght = 1;
    drawLine[numOfDraws-1].x = (double*)malloc(101*sizeof(double));
    drawLine[numOfDraws-1].y = (double*)malloc(101*sizeof(double));
    if(drawLine[numOfDraws-1].x == NULL||drawLine[numOfDraws-1].y == NULL) {
        SDL_Log("Memory error!\n");
        exit(EXIT_FAILURE);
    }
    drawLine[numOfDraws-1].x[0] = posX;
    drawLine[numOfDraws-1].y[0] = posY;
    drawLine[numOfDraws-1].type = type;

    lineStatus = 2;
    return;
}

void addPointToNewDraw(double posX, double posY) {
    // increment the pointer
    drawLine[numOfDraws-1].lenght++;

    // grow by every 100th
    if(((int)drawLine[numOfDraws-1].lenght/100) - ((int)(drawLine[numOfDraws-1].lenght-1)/100)) {
        // grow by 100 points
        double *new1, *new2,
                *old1 = drawLine[numOfDraws-1].x,
                *old2 = drawLine[numOfDraws-1].y;
        new1 = realloc(old1, (drawLine[numOfDraws-1].lenght+101)*sizeof(double));
        new2 = realloc(old2, (drawLine[numOfDraws-1].lenght+101)*sizeof(double));
        if(new1 == NULL||new2 == NULL) {
            SDL_Log("Memory error!\n");
            exit(EXIT_FAILURE);
        }
        drawLine[numOfDraws-1].x = new1;
        drawLine[numOfDraws-1].y = new2;
    }

    int potato = drawLine[numOfDraws-1].lenght-1;

    // apply the new points
    drawLine[numOfDraws-1].x[potato] = posX;
    drawLine[numOfDraws-1].y[potato] = posY;
    return;
}

void finishNewDraw(void) {
    // reallocate to save on memory
    double *new = realloc(drawLine[numOfDraws-1].x, (drawLine[numOfDraws-1].lenght+1)*sizeof(double));
    drawLine[numOfDraws-1].x = new;
    new = realloc(drawLine[numOfDraws-1].y, (drawLine[numOfDraws-1].lenght+1)*sizeof(double));
    drawLine[numOfDraws-1].y = new;

    // check if something went wrong
    if(drawLine[numOfDraws-1].x==NULL||drawLine[numOfDraws-1].y==NULL) {
        SDL_Log("Memory error!\n");
        exit(EXIT_FAILURE);
    }

    // reset line status
    lineStatus = 0;
    return;
}

void discardNewLine(void) {
    if(lineStatus!=1) return;
    numOfLines--;
    lineStatus = 0;
    line1 *new = realloc(normalLine, (numOfLines+1)*sizeof(line1));
    if(new == NULL) {
        SDL_Log("Uh oh! Something went terribly wrong! The program is going to crash, NOW!\n");
        exit(EXIT_FAILURE);
    }

    normalLine = new;
    return;
}

void deleteAllLines(void) {
    free(normalLine);
    for(int i = 0; i < numOfDraws; i++) {
        free(drawLine[i].x);
        free(drawLine[i].y);
    }
    free(drawLine);

    drawLine = NULL;
    normalLine = NULL;

    lineStatus = 0;
    numOfLines ^= numOfLines;
    numOfDraws ^= numOfDraws;
    return;
}

int saveEditorFile(char *filename) {
    FILE *file = fopen(filename, "wb");
    if(file == NULL) return 1;

    // write header
    fprintf(file, "%d %d\n", numOfLines, numOfDraws);

    // write individual lines
    for(int i = 0; i < numOfLines; i++)
        fprintf(file, "%d %lg %lg %lg %lg\n", normalLine[i].type, normalLine[i].x[0], normalLine[i].y[0], normalLine[i].x[1], normalLine[i].y[1]);

    // write individual draws
    for(int i = 0; i < numOfDraws; i++) {
        fprintf(file, "%d %d\n", drawLine[i].type, drawLine[i].lenght);
        for(int j = 0; j < drawLine[i].lenght; j++)
            fprintf(file, "%lg %lg\n", drawLine[i].x[j], drawLine[i].y[j]);
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
    if(fscanf(file, "%d %d\n", &numOfLines, &numOfDraws) != 2) {
        SDL_Log("Error: unable to read file info!\n");
        return 1;
    }

    // allocating the whole thing
    normalLine = malloc((numOfLines+1)*sizeof(line1));
    drawLine = malloc((numOfDraws+1)*sizeof(line2));
    if(normalLine==NULL||drawLine==NULL) {
        SDL_Log("Memory error!\n");
        return 1;
    }

    // allocating our string
    char *tempString = malloc(1024);
    if(tempString == NULL) {
        SDL_Log("Memory error!\n");
        return 1;
    }

    // reading and importing the whole thing
    int doneLines = 0, doneDraws = 0;
    while(fgets(tempString, 1024, file) != NULL) {
        if(doneLines < numOfLines){
            sscanf(tempString, "%d %lf %lf %lf %lf", &normalLine[doneLines].type, &normalLine[doneLines].x[0],
                    &normalLine[doneLines].y[0], &normalLine[doneLines].x[1], &normalLine[doneLines].y[1]);
			doneLines++;
        } else if(doneDraws < numOfDraws){
            sscanf(tempString, "%d %d\n", &drawLine[doneDraws].type, &drawLine[doneDraws].lenght);
            drawLine[doneDraws].x = malloc(drawLine[doneDraws].lenght*sizeof(double));
            drawLine[doneDraws].y = malloc(drawLine[doneDraws].lenght*sizeof(double));
            if(drawLine[doneDraws].x==NULL||drawLine[doneDraws].y==NULL) {
                SDL_Log("Memory allocation error!\n");
                exit(EXIT_FAILURE);
            }
            for(int i = 0; i < drawLine[doneDraws].lenght; i++)
                fscanf(file, "%lf %lf\n", &drawLine[doneDraws].x[i], &drawLine[doneDraws].y[i]);
            doneDraws++;
        }
    }

    free(tempString);
    fclose(file);
    return 0;
}

void drawPosition(void) {
    if(positionTexture != NULL) SDL_DestroyTexture(positionTexture);
    sprintf(positionString, "%sx=%.2f y=%.2f, %s%.2f\\0", textLine[11], x, y, textLine[12], zoom);
    positionSurface = TTF_RenderUTF8_Blended(detailFont, positionString, translate_color(DETAIL_FONT_COLOR));
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
        case 1:
            SDL_RenderDrawLine(render, x1, y1, x2, y2);
            break;
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
                int mouseCorX = 0, mouseCorY = 0, tempVarX, tempVarY;
                SDL_GetMouseState(&mouseCorX, &mouseCorY);
                SDL_GetWindowSize(win, &tempVarX, &tempVarY);

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
                    normalLine[numOfLines-1].x[1] = (mouseCorX - tempVarX/2 - x/zoom)*zoom;
                    normalLine[numOfLines-1].y[1] = (mouseCorY - tempVarY/2 - y/zoom)*zoom;
                } else if(mouseButton&&lineStatus==2)
                    addPointToNewDraw((mouseCorX - tempVarX/2 - x/zoom)*zoom, (mouseCorY - tempVarY/2 - y/zoom)*zoom);

                SDL_FlushEvent(SDL_MOUSEMOTION);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                int mouseCorX = 0, mouseCorY = 0, tempVarX, tempVarY;
                SDL_GetMouseState(&mouseCorX, &mouseCorY);
                SDL_GetWindowSize(win, &tempVarX, &tempVarY);


                // Line type selection
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(lineStatus == 1) discardNewLine();
                    lineStatus = 0;
                    lineType = 0;
                } else if(mouseCorY <= 40 && mouseCorX <= 200)
                    if(lineType == (mouseCorX-1)/40+1)
                        lineType = 0;
                    else
                        lineType = (mouseCorX-1)/40+1;
                else if(mouseCorY <= 40 && mouseCorX >= tempVarX - 40)
                    deleteAllLines();
                else if(mouseCorY >= tempVarY - 40){
                    switch((mouseCorX-1)/40)
                    {
                        case 0:    // load file
                        {
                            char *new;
                            if(filename != NULL)
                                    new = (char*)tinyfd_openFileDialog(textLine[14], filename, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15], 0);
                            else    new = (char*)tinyfd_openFileDialog(textLine[14], NULL, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15], 0);

                            // handle cancel
                            if(new == NULL) break;

                            // switch-a-roo
                            filename = new;

                            // wipe old memory
                            cleanEditor();

                            // reinitilize editor
                            initEditor(0);

                            if(loadEditorFile(filename, 0))
                                // display error message
                                tinyfd_messageBox(textLine[16], textLine[17], "ok", "error", 1);

                            break;
                        }
                        case 1:    // save file
                        {
                            char *new;
                            if(filename != NULL)
                                    new = (char*)tinyfd_saveFileDialog(textLine[13], filename, NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);
                            else    new = (char*)tinyfd_saveFileDialog(textLine[13], "Untitled.sketch", NUM_OF_FILE_TYPES, FILE_TYPES, textLine[15]);

                            // handle cancel
                            if(new == NULL) break;

                            // clear old string;
                            filename = new;

                            // display error message
                            if(saveEditorFile(filename))
                                // display error message
                                tinyfd_messageBox(textLine[16], textLine[17], "ok", "error", 1);
                            break;
                        }
                        default:
                            break;
                    }
                }else if(lineType&&lineType!=5){
                    allocNewLine(lineType,
                                 (mouseCorX - tempVarX/2 - x/zoom)*zoom,
                                 (mouseCorY - tempVarY/2 - y/zoom)*zoom);
                    mouseButton = true;
                }else if(lineType==5){
                    allocNewDraw(lineType,
                                 (mouseCorX - tempVarX/2 - x/zoom)*zoom,
                                 (mouseCorY - tempVarY/2 - y/zoom)*zoom);
                    mouseButton = true;
                }else if(lineStatus==1){
                    lineStatus = 0;
                    normalLine[numOfLines-1].x[1] = (mouseCorX - tempVarX/2 - x/zoom)*zoom;
                    normalLine[numOfLines-1].y[1] = (mouseCorY - tempVarY/2 - y/zoom)*zoom;
                } mouseButton = true;
                break;
            }
			case SDL_MOUSEBUTTONUP:
			{
				mouseButton = false;
				lastX = 0;
				lastY = 0;
                if(lineStatus==2) finishNewDraw();
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
    SDL_SetRenderDrawColor(render, (Uint8)CLEAR_COLOR >> 24 % 8, (Uint8)CLEAR_COLOR >> 16 % 8, (Uint8)CLEAR_COLOR >> 8 % 8, (Uint8)CLEAR_COLOR % 8);
    SDL_RenderClear(render);

    // get window size
    int tempVarX, tempVarY, tempVarW, tempVarH;
    SDL_GetWindowSize(win, &tempVarX, &tempVarY);

	// additional variables
	SDL_Rect rect;


    // draw axis
    if(grid){
    	// X axis
        int tempVar = tempVarX/2 + (int)((double)x/(double)zoom);
    	while(tempVar < tempVarX) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
            tempVar += (int)((double)cuts/(double)zoom);
    	}
        tempVar = tempVarX/2 + (int)((double)x/(double)zoom);
    	while(tempVar > 0) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
            tempVar -= (int)((double)cuts/(double)zoom);
    	}
    	// Y axis
        tempVar = tempVarY/2 + (int)((double)y/(double)zoom);
    	while(tempVar < tempVarY) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
            tempVar += (int)((double)cuts/(double)zoom);
    	}
        tempVar = tempVarY/2 + (int)((double)y/(double)zoom);
    	while(tempVar > 0) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
            tempVar -= (int)((double)cuts/(double)zoom);
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


    // draw special lines
    for(int i = 0; i < numOfDraws; i++) {
        int line_x[2], line_y[2];

        for(int j = 0; j < drawLine[i].lenght-1; j++){
            // calculate the lines
            line_x[0] = tempVarX/2 + (int)((double)x/zoom + drawLine[i].x[j]/zoom);
            line_x[1] = tempVarX/2 + (int)((double)x/zoom + drawLine[i].x[j+1]/zoom);
            line_y[0] = tempVarY/2 + (int)((double)y/zoom + drawLine[i].y[j]/zoom);
            line_y[1] = tempVarY/2 + (int)((double)y/zoom + drawLine[i].y[j+1]/zoom);

            // draw the line
            drawNormalLine(1, line_x[0], line_y[0], line_x[1], line_y[1], TITLE_FONT_COLOR);
        }
    }

	// Draw UI
    SDL_RenderSetScale(render, 2.0, 2.0);
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){0, lineType == 1 ? 20 : 0,20,20}, &(SDL_Rect){0,0,20,20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){20, lineType == 2 ? 20 : 0,20,20}, &(SDL_Rect){20,0,20,20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){40, lineType == 3 ? 20 : 0,20,20}, &(SDL_Rect){40,0,20,20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){60, lineType == 4 ? 20 : 0,20,20}, &(SDL_Rect){60,0,20,20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){80, lineType == 5 ? 20 : 0,20,20}, &(SDL_Rect){80,0,20,20});

    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){160, 0, 20, 20}, &(SDL_Rect){0, tempVarY/2-20, 20, 20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){120, 0, 20, 20}, &(SDL_Rect){20, tempVarY/2-20, 20, 20});
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){100, 0, 20, 20}, &(SDL_Rect){tempVarX/2-20, 0, 20, 20});

    // In the bottom right draw the position
    SDL_RenderSetScale(render, 1.0, 1.0);
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

    // free memory
    if(level&&filename!=NULL) free(filename);

    initEditor(level);
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
    free(filename);
    return 0;
}
