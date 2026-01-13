#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdint.h>

enum Dir { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

// Call once in setup()
void floodfillInit();

// Call every cell after reading sensors
void floodfillUpdateWalls(bool front, bool left, bool right);

// Call to get next direction
Dir floodfillNextMove();

// Call after you move one cell
void floodfillAdvance();

// Optional helpers
Dir floodfillGetHeading();
void floodfillSetHeading(Dir d);
bool floodfillAtGoal();

#endif
