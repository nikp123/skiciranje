#include "snap.h"

void snapIfPossible(double posX, double posY, double cuts, line1 *line, int lines, char snap, char grid) {
    double closestX = 0.0, closestY = 0.0, found = 0;
	const double minimalDistance = cuts/4;
	if(snap) {
        // snap to grid
        if(grid&&(distance2d(fmod(posX, cuts), fmod(posY, cuts)) < minimalDistance)) {
			closestX = round(posX/cuts)*cuts;
			closestY = round(posY/cuts)*cuts;
			found=1;
		} else {
			// avoid user trying to break my stuff
			if(!lines) return;
			
			closestX = line[0].x[0];
			closestY = line[0].y[0];
			if((distance2d(closestX-posX, closestY-posY) < minimalDistance)&&(lines!=1)) found=1;
		}

        // snap to an previous line
        for(int i = 0; i < lines-1; i++) {
        for(int j = 0; j < line[i].lenght; j++) {
			// avoid snapping to self ffs
			if(i==lines-2&&j==line[i].lenght-1) continue;
            if(distance2d(line[i].x[j]-posX, line[i].y[j]) < distance2d(closestX-posX, closestY-posY)) {
                if(distance2d(line[i].x[j]-posX, line[i].y[i]-posY) < minimalDistance) found=1;
				closestX = line[i].x[j];
                closestY = line[i].y[j];
            }
        }}
    }
    // replace the original value
	line[lines-1].x[line[lines-1].lenght-1] = found ? closestX : posX;
	line[lines-1].y[line[lines-1].lenght-1] = found ? closestY : posY;
}

