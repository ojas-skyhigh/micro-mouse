#include "floodfill.h"
#include "API.h"

#include <queue>
#include <utility>
#include <fstream>

using namespace std;

static int dx[4] = {0, 1, 0, -1};
static int dy[4] = {1, 0, -1, 0};

#define WALL_N 0x01
#define WALL_E 0x02
#define WALL_S 0x04
#define WALL_W 0x08

static uint8_t walls[16][16];
static uint16_t cost[16][16];

static int x = 0, y = 0;
static Dir heading = NORTH;
static bool initialized = false;

static bool isGoal(int x, int y) {
    return (x == 7 || x == 8) && (y == 7 || y == 8);
}

bool reachedGoal() {
    return isGoal(x, y);
}

static bool hasWall(int x, int y, int dir) {
    return walls[x][y] & (1 << dir);
}

static void setWallInternal(int x, int y, int dir) {
    walls[x][y] |= (1 << dir);
    int nx = x + dx[dir];
    int ny = y + dy[dir];
    if (nx < 0 || nx >= 16 || ny < 0 || ny >= 16) return;
    walls[nx][ny] |= (1 << ((dir + 2) % 4));
}

static void initBoundaryWalls() {
    for (int i = 0; i < 16; i++) {
        walls[i][0] |= WALL_S;
        walls[i][15] |= WALL_N;
        walls[0][i] |= WALL_W;
        walls[15][i] |= WALL_E;
    }
}

void saveMaze() {
    std::ofstream fout("walls.dat", std::ios::binary);
    fout.write((char*)walls, sizeof(walls));
    fout.close();
}

void loadMaze() {
    std::ifstream fin("walls.dat", std::ios::binary);
    if (fin) {
        fin.read((char*)walls, sizeof(walls));
        fin.close();
    } else {
        initBoundaryWalls();
    }
}
static void computeFloodFill() {
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

static Dir chooseNextDir() {
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

static void senseWalls() {
    if (API::wallFront()) setWallInternal(x, y, heading);
    if (API::wallLeft())  setWallInternal(x, y, (heading + 3) % 4);
    if (API::wallRight()) setWallInternal(x, y, (heading + 1) % 4);
}

static void turnTo(Dir target) {
    int diff = (target - heading + 4) % 4;
    if (diff == 1) API::turnRight();
    else if (diff == 3) API::turnLeft();
    else if (diff == 2) {
        API::turnRight();
        API::turnRight();
    }
    heading = target;
}

void floodfillInit() {
    if (initialized) return;

    loadMaze();

    API::setColor(7,7,'R');
    API::setColor(7,8,'R');
    API::setColor(8,7,'R');
    API::setColor(8,8,'R');

    initialized = true;
}

void floodfillStep() {
    if (reachedGoal()) {
        saveMaze();
        return;
    }

    senseWalls();
    computeFloodFill();
    Dir next = chooseNextDir();
    turnTo(next);
    API::moveForward();

    x += dx[heading];
    y += dy[heading];
}

