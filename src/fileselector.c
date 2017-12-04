#include "shared.h"
#include "fileselector.h"
#include "libs/tinydir.h"
#include "functions.h"

int fileCount;
SDL_Surface **fileList;
SDL_Surface *newFileButton;
SDL_Surface *newFolderButton;
SDL_Surface *backButton;
char **files;
double offset;

char **getDirectoryListing(char *directory, int *fileCount) {
    tinydir_dir dir;
    int wasnull = 0;
    if(directory == NULL) {
        wasnull = 1;
        directory = malloc(sizeof(char)*1024);

        // gets the current working directory > cwd
        if(getcwd(directory, 1024) != NULL)
            SDL_Log("Entering directory: %s\n", directory);
        else {
            SDL_Log("Unable to open current directory!\n");
            return NULL;
        }
    }
    // use external libraries to open a directory
    tinydir_open_sorted(&dir, directory);

    // allocate our file structure
    char **files = (char**) malloc(dir.n_files*sizeof(char*));
    if(files == NULL) {
        SDL_Log("Memory error!\n");
        return NULL;
    }

    // load file struture into a usable array
    for(int i = 0; i < dir.n_files; i++) {
        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);

        files[i] = (char*)malloc(3+sizeof(directory)+sizeof(file.name));
        if(files[i] == NULL) {
            SDL_Log("Memory error!\n");
        }
        if(file.is_dir)sprintf(files[i], "%s/%s/", directory, file.name);
        else sprintf(files[i], "%s/%s", directory, file.name);
    }

    // return the number of files
    *fileCount = dir.n_files;

    // free unused memory
    if(wasnull) free(directory);
    tinydir_close(&dir);

    return files;
}

int fileSelectorInit() {
    // clear screen
    SDL_FillRect(swin, NULL, CLEAR_COLOR);
    SDL_UpdateWindowSurface(win);

    // get the directory list
    files = getDirectoryListing(NULL, &fileCount);
    if(files == NULL) return 1;

    fileList = (SDL_Surface**)malloc(sizeof(SDL_Surface*)*fileCount);
    if(fileList == NULL) {
        SDL_Log("Memory error!\n");
        return 1;
    }

    for(int i = 0; i < fileCount; i++) {
        fileList[i] = TTF_RenderUTF8_Blended(optionFont, files[i], translate_color(OPTION_FONT_COLOR));
        if(fileList[i] == NULL) {
            SDL_Log("Memory error!\n");
            return 1;
        }
    }

    newFileButton = TTF_RenderUTF8_Blended(menuFont, textLine[7], translate_color(MENU_FONT_COLOR));
    newFolderButton = TTF_RenderUTF8_Blended(menuFont, textLine[8], translate_color(MENU_FONT_COLOR));
    backButton = TTF_RenderUTF8_Blended(menuFont, textLine[9], translate_color(MENU_FONT_COLOR));
    if(newFileButton==NULL||newFolderButton==NULL||backButton==NULL) {
        SDL_Log("Memory error!\n");
        return 1;
    }
    return 0;
}

void fileSelectorDraw(void) {
    // Clear the background
    SDL_FillRect(swin, NULL, CLEAR_COLOR);

    // Draw background of file select
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = swin->w;
    rect.h = swin->h - TTF_FontHeight(menuFont);
    SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);

    // Draw file list
    for(int i = 0; i < 1+(swin->h - TTF_FontHeight(menuFont))/TTF_FontHeight(optionFont) && i < fileCount; i++) {
        if(fileList[i]->w > swin->w) rect.x = swin->w - fileList[i]->w;
        else rect.x = 0;
        rect.y = i*TTF_FontHeight(optionFont);
        rect.w = swin->w;
        rect.h = swin->h - TTF_FontHeight(menuFont) - rect.y;

        SDL_BlitSurface(fileList[i], NULL, swin, &rect);
    }

    // Draw the buttons
    rect.x = 0;
    rect.y = swin->h - TTF_FontHeight(menuFont);
    rect.w = newFileButton->w;
    rect.h = newFileButton->h;
    if(glow == 1) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
    SDL_BlitSurface(newFileButton, NULL, swin, &rect);
    rect.x += newFileButton->w;
    rect.w = newFolderButton->w;
    if(glow == 2) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
    SDL_BlitSurface(newFolderButton, NULL, swin, &rect);
    rect.x += newFolderButton->w;
    rect.w = backButton->w;
    if(glow == 3) SDL_FillRect(swin, &rect, DETAIL_FONT_COLOR);
    SDL_BlitSurface(backButton, NULL, swin, &rect);

    // Update window
    SDL_UpdateWindowSurface(win);
    return;
}

int fileSelectorInput(void) {
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
                    if(mouseCorX < newFileButton->w) glow = 1;
                    else if(mouseCorX < newFileButton->w + newFolderButton->w) glow = 2;
                    else if(mouseCorX < newFileButton->w + newFolderButton->w + backButton->w) glow = 3;
                    else glow = 0;
                } else glow = 0;
                SDL_FlushEvent(SDL_MOUSEMOTION);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                int mouseCorX = 0, mouseCorY = 0;
                SDL_GetMouseState(&mouseCorX, &mouseCorY);

                if(mouseCorY > swin->h - TTF_FontHeight(menuFont)) {
                    if(mouseCorX < newFileButton->w) break;
                    else if(mouseCorX < newFileButton->w + newFolderButton->w) break;
                    else if(mouseCorX < newFileButton->w + newFolderButton->w + backButton->w) return 1;
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
                }
                break;
            default: break;
        }
    }
    return 0;
}

void fileSelectorClean(void) {
    for(int i = 0; i < fileCount; i++) {
        free(files[i]);
        SDL_FreeSurface(fileList[i]);
    }
    free(files);
    free(fileList);

    SDL_FreeSurface(newFileButton);
    SDL_FreeSurface(newFolderButton);
    SDL_FreeSurface(backButton);
    return;
}

char *fileSelector(void) {
    char *filename = NULL;

    if(fileSelectorInit()) return NULL;
    fileSelectorDraw();
    int status = 0;
    while(!status) {
        int frameStart = SDL_GetTicks();
        status = fileSelectorInput();
        fileSelectorDraw();
        if(SDL_GetTicks()-frameStart<1000/FRAMERATE)
            SDL_Delay((1000/FRAMERATE)-SDL_GetTicks()+frameStart);
    }
    switch(status) {
        case 1:
            fileSelectorClean();
            break;
        case 2:
            exit(EXIT_SUCCESS);
    }
    return filename;
}
