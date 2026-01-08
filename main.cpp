#include "API.h"
#include "floodfill.h"

int main() {
    floodfillInit();
    while (!reachedGoal()) {
        floodfillStep();
    }
    API::setText(7, 7, "GOAL");
    saveMaze();
}
