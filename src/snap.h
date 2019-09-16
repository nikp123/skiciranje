#include <math.h>
#include "shared.h"
#include "line.h"

_Bool snapIfPossible(double posX, double posY, double cuts, line1 *line, Uint32 lines, char snap, char grid);
_Bool isSnappable(double posX, double posY, double cuts, line1 *line, Uint32 lines, char snap, char grid);
