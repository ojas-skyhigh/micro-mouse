#pragma once
#include <cstdint>

enum Dir { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

// Call once on boot
void floodfillInit();

// Call repeatedly (Arduino loop / MMS while loop)
void floodfillStep();

// Optional helpers
bool reachedGoal();

// For MMS
void saveMaze();
void loadMaze();
