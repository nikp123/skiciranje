#include "snap.h"

_Bool snapIfPossible(double posX, double posY, double cuts, line1 *line, Uint32 lines, char snap, char grid, _Bool write) {
    double closestX = (double)cuts/4+posX, closestY = (double)cuts/4+posY;
	_Bool found = 0;
	const double minimalDistance = cuts/4;
	if(snap) {
        // snap to an previous line
		if(lines) {
		for(Uint32 i = 0; i < lines-1; i++) {
		for(Uint32 j = 0; j < line[i].lenght; j++) {
			if(distance2d(line[i].x[j]-posX, line[i].y[j]-posY) < distance2d(closestX-posX, closestY-posY)) {
				found=1;
				closestX = line[i].x[j];
				closestY = line[i].y[j];
			}
		}}}

		if(!found) {
		// snap to grid
        if(grid&&(distance2d(cmod(posX, cuts), cmod(posY, cuts)) < minimalDistance)) {
			closestX = round(posX/cuts)*cuts;
			closestY = round(posY/cuts)*cuts;
			found=1;
		}
		}
    }
    // replace the original value
	if(write) {
		line[lines-1].x[line[lines-1].lenght-1] = found ? closestX : posX;
		line[lines-1].y[line[lines-1].lenght-1] = found ? closestY : posY;
	}
	return found;
}
