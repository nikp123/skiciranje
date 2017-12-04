#include "shared.h"

void clean(void) {

    // freeing fonts
    TTF_CloseFont(optionFont);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(menuFont);
    TTF_CloseFont(detailFont);

    // freeing basic stuff
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return;
}
