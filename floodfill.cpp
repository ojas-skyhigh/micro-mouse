/*
    Still need to add storing each wall in EEPROM aftter reaching goal
*/

#include "floodfill.h"

#define MAZE_SIZE 16
#define INF 0xFFFF

#define WALL_N 0x01
#define WALL_E 0x02
#define WALL_S 0x04
#define WALL_W 0x08

static uint8_t walls[MAZE_SIZE][MAZE_SIZE];
static uint16_t cost[MAZE_SIZE][MAZE_SIZE];

static int x = 0, y = 0;
static Dir heading = NORTH;

static const int dx[4] = {0, 1, 0, -1};
static const int dy[4] = {1, 0, -1, 0};

static bool isGoal(int cx, int cy) {
    return (cx == 7 || cx == 8) && (cy == 7 || cy == 8);
}

static void setWall(int cx, int cy, Dir d) {
    walls[cx][cy] |= (1 << d);

    int nx = cx + dx[d];
    int ny = cy + dy[d];

    if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) return;

    walls[nx][ny] |= (1 << ((d + 2) % 4));
}

static bool hasWall(int cx, int cy, Dir d) {
    return walls[cx][cy] & (1 << d);
}

static void initBoundaryWalls() {
    for (int i = 0; i < MAZE_SIZE; i++) {
        walls[i][0]  |= WALL_S;
        walls[i][15] |= WALL_N;
        walls[0][i]  |= WALL_W;
        walls[15][i] |= WALL_E;
    }
}

static void computeFloodFill() {
    for (int i = 0; i < MAZE_SIZE; i++)
        for (int j = 0; j < MAZE_SIZE; j++)
            cost[i][j] = INF;

    int qx[256], qy[256];
    int head = 0, tail = 0;

    for (int i = 7; i <= 8; i++) {
        for (int j = 7; j <= 8; j++) {
            cost[i][j] = 0;
            qx[tail] = i;
            qy[tail] = j;
            tail++;
        }
    }

    while (head != tail) {
        int cx = qx[head];
        int cy = qy[head];
        head++;

        for (int d = 0; d < 4; d++) {
            if (hasWall(cx, cy, (Dir)d)) continue;

            int nx = cx + dx[d];
            int ny = cy + dy[d];

            if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

            if (cost[nx][ny] > cost[cx][cy] + 1) {
                cost[nx][ny] = cost[cx][cy] + 1;
                qx[tail] = nx;
                qy[tail] = ny;
                tail++;
            }
        }
    }
}

/* ================= PUBLIC API ================= */

void floodfillInit() {
    for (int i = 0; i < MAZE_SIZE; i++)
        for (int j = 0; j < MAZE_SIZE; j++)
            walls[i][j] = 0;

    initBoundaryWalls();
    computeFloodFill();
}

void floodfillUpdateWalls(bool front, bool left, bool right) {
    if (front) setWall(x, y, heading);
    if (left)  setWall(x, y, (Dir)((heading + 3) % 4));
    if (right) setWall(x, y, (Dir)((heading + 1) % 4));

    computeFloodFill();
}

Dir floodfillNextMove() {
    Dir best = heading;
    uint16_t bestCost = cost[x][y];

    for (int d = 0; d < 4; d++) {
        if (hasWall(x, y, (Dir)d)) continue;

        int nx = x + dx[d];
        int ny = y + dy[d];

        if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

        if (cost[nx][ny] < bestCost) {
            bestCost = cost[nx][ny];
            best = (Dir)d;
        }
    }
    return best;
}

void floodfillAdvance() {
    x += dx[heading];
    y += dy[heading];
}

Dir floodfillGetHeading() {
    return heading;
}

void floodfillSetHeading(Dir d) {
    heading = d;
}

bool floodfillAtGoal() {
    return isGoal(x, y);
}
