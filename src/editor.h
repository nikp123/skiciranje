#include "shared.h"
#include "functions.h"
#include "line.h"

Uint32 lines, drawableLines;
int special, grid, snap;
int lastX, lastY;
double zoom, x, y, cuts;
SDL_Surface *positionSurface;
SDL_Texture *positionTexture, *buttonTexture;
char *positionString, mouseButton, lineType, lineStatus;
char *filename;

line1 *line;
//static void drawPosition(void);
extern int editor(int level);
void discardNewLine(void);
