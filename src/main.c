#include "shared.h"
#include "init.h"
#include "clean.h"
#include "menu.h"
#include "editor.h"
#include "tutorial.h"

int main(int argc, char *argv[])
{
    if(gameInit(argc, argv))
    {
        fprintf(stderr, "Initilization failed!");
        return 1;
    }
    gotoMenu:
    switch(menu()) {
        case 1: break;
        case 2:
            switch(editor(0)) {
                case 0:
                    goto gotoMenu;
                default:
                    break;
            }
        case 3:
            switch(tutorial()) {
                case 0:
                    goto gotoMenu;
                default:
                    break;
            }
        //case 4: informationScreen(); break;
        default:
            SDL_Log("Error! Undefined result in menu()!\n");
            break;
    }
    clean();
    return 0;
}
