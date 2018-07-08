#include "editor.h"
#include "snap.h"

int lines, drawableLines;
int special, grid, snap;
int lastX, lastY;
double zoom, x, y, cuts;
SDL_Surface *positionSurface = NULL;
SDL_Texture *positionTexture = NULL, *buttonTexture;
char *positionString, mouseButton = false, lineType = 0, lineStatus = 0;
char *filename;

line1 *line;

void redo(void) {
    if(drawableLines != lines) drawableLines++;
    return;
}

void undo(void) {
    if(drawableLines != 0) drawableLines--;
    return;
}

void allocNewLine(int type, double posX, double posY, int drawType) {
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
		snapIfPossible(posX, posY, cuts, line, lines, snap, grid);
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
    for(int i = 0; i < drawableLines; i++) {
        //if(line[i].lenght < 2) continue; // avoids bugged out lines
        for(int j = 0; j < line[i].lenght-1; j++) {
            // don't try to understand this, this is above my expertise
            // just know that this works and don't touch it
            const double r = 10*zoom;
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
					for(int l = i; l < drawableLines; l++) {
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
        for(int i = 0; i < lines; i++) {
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
    for(int i = 0; i < lines; i++){
        fprintf(file, "%d %d\n", line[i].type, line[i].lenght);
        for(int j = 0; j < line[i].lenght; j++)
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
    for(int doneLines = 0; doneLines < lines; doneLines++) {
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

        for(int i = 0; i < line[doneLines].lenght; i++) {
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
    sprintf(positionString, "%sx=%.2f y=%.2f, %s%.2f*10^%d", textLine[11], x, y, textLine[12], zoom/powf(10.0, floor(log10(zoom))), (int)floor(log10(zoom)));
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
    zoom = 0.1f;
    x = 0.0f;
    y = 0.0f;
    cuts = 10;
    grid = 1;
    snap = 1;
    mouseButton = false;

    drawPosition();
    return 0;
}

void drawNormalLine(int type, int x1, int y1, int x2, int y2, Uint32 color) {
    // get window size
    int tempVarX, tempVarY;
    SDL_GetWindowSize(win, &tempVarX, &tempVarY);

    // set draw color
    SDL_SetRenderDrawColor(render, color >> 24 % 8, color >> 16 % 8, color >> 8 % 8, color % 8);

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
            float xRatio = (x2-x1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
            float yRatio = (y2-y1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
            float lineLen = sqrt(pow((x2-x1), 2)+pow((y2-y1), 2));
            for(int i = 0; i < floor(lineLen)-6; i += 12){
                if(x1+i*xRatio < 0) continue;
                if(y1+i*yRatio < 0) continue;
                if(x1+i*xRatio > tempVarX) continue;
                if(y1+i*yRatio > tempVarY) continue;

                SDL_RenderDrawLine(render, x1+i*xRatio, y1+i*yRatio, x1+(i+5)*xRatio, y1+(i+5)*yRatio);
            }
            break;
        }
        case 4:
        {
            float xRatio = (x2-x1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
            float yRatio = (y2-y1)/floor(sqrt(pow(x2-x1, 2)+pow(y2-y1, 2)));
            float lineLen = sqrt(pow((x2-x1), 2)+pow((y2-y1), 2));
            for(int i = 0; i < floor(lineLen)-6; i += 13){
                if(x1+i*xRatio < 0) continue;
                if(y1+i*yRatio < 0) continue;
                if(x1+i*xRatio > tempVarX) continue;
                if(y1+i*yRatio > tempVarY) continue;

                SDL_RenderDrawLine(render, x1+i*xRatio, y1+i*yRatio, x1+(i+5)*xRatio, y1+(i+5)*yRatio);
                SDL_RenderDrawPoint(render, x1+(i+9)*xRatio, y1+(i+9)*yRatio);
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
    while(SDL_PollEvent(&event))
    {
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
                        promptBeforeExit();
                        SDL_FlushEvent(SDL_KEYDOWN);
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
                    case SDLK_h:
                        snap = !snap;
                        break;
                    case SDLK_x:
                        undo();
                        break;
                    case SDLK_c:
                        redo();
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
                } else if(!mouseButton&&lineStatus==1)
					snapIfPossible((mouseCorX - tempVarX/2 - x/zoom)*zoom, (mouseCorY - tempVarY/2 - y/zoom)*zoom, cuts, line, lines, snap, grid);
                else if(mouseButton&&lineStatus==2)
                    addPointToNewDraw((mouseCorX - tempVarX/2 - x/zoom)*zoom, (mouseCorY - tempVarY/2 - y/zoom)*zoom);
                else if(mouseButton&&lineType==127)
                    deleteLine((mouseCorX - tempVarX/2 - x/zoom)*zoom, (mouseCorY - tempVarY/2 - y/zoom)*zoom);
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
                else if(mouseCorY <= 40 && mouseCorX    >= tempVarX - 40)
                    deleteAllLines();
                else if(mouseCorY <= 80 && mouseCorX >= tempVarX - 40) {
                    promptBeforeExit();
                    return 1;
                } else if(mouseCorY <= 40 && mouseCorX >= tempVarX - 120 && mouseCorX < tempVarX - 80)
                    redo();
                else if(mouseCorY <= 40 && mouseCorX >= tempVarX - 160 && mouseCorX < tempVarX - 120)
                    undo();
                else if(mouseCorY <= 40 && mouseCorX >= tempVarX - 240 && mouseCorX < tempVarX - 200)
                    lineType = 127;
                else if(mouseCorY >= 40 && mouseCorY < 80 && mouseCorX < 40)
                    snap = !snap;
                else if(mouseCorY >= 40 && mouseCorY < 80 && mouseCorX >= 40 && mouseCorX < 80)
                    grid = !grid;
                else if((mouseCorY >= tempVarY-40)&&(mouseCorX<40)){
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
                } else if((mouseCorY >= tempVarY-40)&&(mouseCorX<80)){
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
                        deleteLine((mouseCorX - tempVarX/2 - x/zoom)*zoom, (mouseCorY - tempVarY/2 - y/zoom)*zoom);
                    } else allocNewLine(lineType,
                                 (mouseCorX - tempVarX/2 - x/zoom)*zoom,
                                 (mouseCorY - tempVarY/2 - y/zoom)*zoom, lineType==5 ? 1 : 0);
                    mouseButton = true;
                }else if(lineStatus==1){
                    lineStatus = 0;
                    line[lines-1].x[1] = (mouseCorX - tempVarX/2 - x/zoom)*zoom;
                    line[lines-1].y[1] = (mouseCorY - tempVarY/2 - y/zoom)*zoom;
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
                else if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_RenderClear(render);
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
    	int counter = 0;
        int tempVar = tempVarX/2 + (int)((double)x/zoom);
        double cutSize = cuts/zoom;
    	while(tempVar < tempVarX) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
            counter++;
            tempVar = tempVarX/2 + x/zoom + cutSize*counter;
    	}
    	counter = 0;
        tempVar = tempVarX/2 + (int)((double)x/(double)zoom);
    	while(tempVar > 0) {
    	    drawNormalLine(1, tempVar, 0, tempVar, tempVarY, DETAIL_FONT_COLOR);
            counter++;
            tempVar = tempVarX/2 + x/zoom - cutSize*counter;
    	}

    	// Y axis
    	counter = 0;
        tempVar = tempVarY/2 + (int)((double)y/(double)zoom);
    	while(tempVar < tempVarY) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
    	    counter++;
            tempVar = tempVarY/2 + y/zoom + cutSize*counter;
    	}
    	counter = 0;
        tempVar = tempVarY/2 + (int)((double)y/(double)zoom);
    	while(tempVar > 0) {
    	    drawNormalLine(1, 0, tempVar, tempVarX, tempVar, DETAIL_FONT_COLOR);
    	    counter++;
            tempVar = tempVarY/2 + y/zoom - cutSize*counter;
        }
    }

    // draw lines
    for(int i = 0; i < drawableLines; i++) {
        int line_x[2], line_y[2];

        for(int j = 0; j < line[i].lenght-1; j++){
            // calculate the lines
            line_x[0] = tempVarX/2 + (int)((double)x/zoom + (double)line[i].x[j]/zoom);
            line_x[1] = tempVarX/2 + (int)((double)x/zoom + (double)line[i].x[j+1]/zoom);
            line_y[0] = tempVarY/2 + (int)((double)y/zoom + (double)line[i].y[j]/zoom);
            line_y[1] = tempVarY/2 + (int)((double)y/zoom + (double)line[i].y[j+1]/zoom);

            // draw the line
            drawNormalLine(line[i].type, line_x[0], line_y[0], line_x[1], line_y[1], TITLE_FONT_COLOR);
        }
    }

	// Draw UI
    SDL_RenderSetScale(render, 2.0, 2.0);
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){0, lineType == 1 ? 20 : 0,20,20}, &(SDL_Rect){0,0,20,20});    // line button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){20, lineType == 2 ? 20 : 0,20,20}, &(SDL_Rect){20,0,20,20});  // thick line button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){40, lineType == 3 ? 20 : 0,20,20}, &(SDL_Rect){40,0,20,20});  // line-stop-line button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){60, lineType == 4 ? 20 : 0,20,20}, &(SDL_Rect){60,0,20,20});  // line-dot-line button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){80, lineType == 5 ? 20 : 0,20,20}, &(SDL_Rect){80,0,20,20});  // draw button

    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){160, 0, 20, 20}, &(SDL_Rect){0, tempVarY/2-20, 20, 20});                                  // load button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){120, 0, 20, 20}, &(SDL_Rect){20, tempVarY/2-20, 20, 20});                                 // save button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){100, 0, 20, 20}, &(SDL_Rect){tempVarX/2-20, 0, 20, 20});                                  // delete all button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){180, 0, 20, 20}, &(SDL_Rect){tempVarX/2-20, 20, 20, 20});                                 // menu
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){140, drawableLines > 0 ? 0 : 20, 20, 20}, &(SDL_Rect){tempVarX/2-80, 0, 20, 20});         // back button
    SDL_RenderCopyEx(render, buttonTexture, &(SDL_Rect){140, lines == drawableLines ? 20 : 0, 20, 20}, &(SDL_Rect){tempVarX/2-60, 0, 20, 20},   // redo button
                     0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){200, lineType == 127 ? 20 : 0, 20, 20}, &(SDL_Rect){tempVarX/2-120, 0, 20, 20});          // delete button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){220, snap ? 20 : 0, 20, 20}, &(SDL_Rect){0, 20, 20, 20});                                 // snap button
    SDL_RenderCopy(render, buttonTexture, &(SDL_Rect){240, grid ? 20 : 0, 20, 20}, &(SDL_Rect){20, 20, 20, 20});                                // grid button

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

    // small patch
    if(level == INT_MAX) level = 0;

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
    return 0;
}
