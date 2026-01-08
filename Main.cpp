#include <iostream>
#include <queue>
#include <utility>
#include "API.h"
#include <cstdint>
#include <fstream>

using namespace std;

enum Dir { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

int dx[4] = {0, 1, 0, -1};
int dy[4] = {1, 0, -1, 0};

#define WALL_N 0x01
#define WALL_E 0x02
#define WALL_S 0x04
#define WALL_W 0x08

uint8_t walls[16][16];
uint16_t cost[16][16];

int x = 0, y = 0;
Dir heading = NORTH;

// Only called once at the start of the ESP32 power-up
void initBoundaryWalls() {
    for (int i = 0; i < 16; i++) {
        walls[i][0] |= WALL_S;
        walls[i][15] |= WALL_N;
        walls[0][i] |= WALL_W;
        walls[15][i] |= WALL_E;
    }
}

bool hasWall(int x, int y, int dir) {
    return walls[x][y] & (1 << dir);
}

void setWall(int x, int y, int dir) {
    walls[x][y] |= (1 << dir);
    int nx = x + dx[dir];
    int ny = y + dy[dir];
    if (nx < 0 || nx >= 16 || ny < 0 || ny >= 16) return;
    walls[nx][ny] |= (1 << ((dir + 2) % 4));
}

bool isGoal(int x, int y) {
    return (x == 7 || x == 8) && (y == 7 || y == 8);
}


void saveWalls() {
    std::ofstream fout("walls.dat", std::ios::binary);
    fout.write((char*)walls, sizeof(walls));
    fout.close();
}

void loadWalls() {
    std::ifstream fin("walls.dat", std::ios::binary);
    if (fin) {
        fin.read((char*)walls, sizeof(walls));
        fin.close();
    } else {
        // first run, just initialize boundaries
        initBoundaryWalls();
    }
}


void computeFloodFill() {
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            cost[i][j] = 0xFFFF;

    queue<pair<int,int>> q;
    for (int i = 7; i <= 8; i++)
        for (int j = 7; j <= 8; j++) {
            cost[i][j] = 0;
            q.push({i, j});
        }

    while (!q.empty()) {
        auto [cx, cy] = q.front(); q.pop();
        for (int d = 0; d < 4; d++) {
            if (hasWall(cx, cy, d)) continue;
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nx < 0 || nx >= 16 || ny < 0 || ny >= 16) continue;
            if (cost[nx][ny] > cost[cx][cy] + 1) {
                cost[nx][ny] = cost[cx][cy] + 1;
                q.push({nx, ny});
            }
        }
    }
}

Dir chooseNextDir() {
    Dir best = heading;
    uint16_t bestCost = cost[x][y];

    for (int d = 0; d < 4; d++) {
        if (hasWall(x, y, d)) continue;
        int nx = x + dx[d];
        int ny = y + dy[d];
        if (nx < 0 || nx >= 16 || ny < 0 || ny >= 16) continue;
        if (cost[nx][ny] < bestCost) {
            bestCost = cost[nx][ny];
            best = (Dir)d;
        }
    }
    return best;
}

void senseWalls() {
    if (API::wallFront()) setWall(x, y, heading);
    if (API::wallLeft())  setWall(x, y, (heading + 3) % 4);
    if (API::wallRight()) setWall(x, y, (heading + 1) % 4);
}

void turnTo(Dir target) {
    int diff = (target - heading + 4) % 4;
    if (diff == 1) API::turnRight();
    else if (diff == 3) API::turnLeft();
    else if (diff == 2) {
        API::turnRight();
        API::turnRight();
    }
    heading = target;
}

int main() {
    // Only set the boundaries once per session
    static bool initialized = false;
    if (!initialized) {
        initBoundaryWalls();
        initialized = true;
    }
    loadWalls();

    // Optional: mark goal cells in the simulator
    API::setColor(7,7,'R');
    API::setColor(7,8,'R');
    API::setColor(8,7,'R');
    API::setColor(8,8,'R');

    // Start floodfill exploration
    while (!isGoal(x, y)) {
        senseWalls();
        computeFloodFill();
        Dir next = chooseNextDir();
        turnTo(next);
        API::moveForward();
        x += dx[heading];
        y += dy[heading];
    }

    API::setText(x, y, "GOAL");
    saveWalls();
    return 0;
}
